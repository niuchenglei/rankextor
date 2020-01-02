#ifndef CLUSTER_H
#define CLUSTER_H

#include <iostream>
#include <sstream>
#include <vector>
//#include <tr1/unordered_map>
#include <boost/unordered_map.hpp>

#include <pthread.h>
#include <poll.h>

#include "Utilities/INIReader.h"
#include "Utilities/Redis.h"
//#include "Interfaces/Logger.h"

using std::string;

namespace rank {
namespace ad_algo {

//#define CLUSTER_DEBUG
#ifdef CLUSTER_DEBUG
static std::ofstream cluster_out("/data0/vad/ms/log/cluster.log",
                                 std::ios_base::app);
#endif

class Cluster {
 public:
  Cluster() : serverNum_(0), pfds_(NULL) { error_ = ""; }
  ~Cluster() {
    for (int i = 0; i < serverNum_; ++i) {
      Worker &worker = workers_[i];
      close(worker.wakeup_pipe[0]);
      close(worker.wakeup_pipe[1]);
      close(worker.sleep_pipe[0]);
      close(worker.sleep_pipe[1]);
    }
    if (pfds_ != NULL) delete[] pfds_;
    if (conf_ != NULL) delete conf_;
  }
  bool Init(const std::string &file) {
    conf_ = new INIReader(file);
    if (conf_->ParseError() < 0) {
      error_ = "config file parse failed: " + file;
      return false;
    }
    std::string cluster = conf_->Get("main", "cluster", "");
    std::string server_group = conf_->Get(cluster, "server_group", "");
    std::istringstream is(server_group);
    std::string server;
    std::vector<std::pair<std::string, int> > servers;
    while (getline(is, server, ',')) {
      size_t pos = server.find(':');
      std::string host = server.substr(0, pos);
      int port = strtoul(server.substr(pos + 1).c_str(), NULL, 10);
      servers.push_back(make_pair(host, port));
    }

    serverNum_ = servers.size();
    if ((pfds_ = new (std::nothrow) pollfd[serverNum_]) == NULL) {
      error_ = "new pollfd failed";
      return false;
    }
    workers_.resize(serverNum_);
    for (int i = 0; i < serverNum_; ++i) {
      Worker &worker = workers_[i];
      if (worker.redis.init(servers[i].first, servers[i].second, 0) == false) {
        std::ostringstream os;
        os << "init redis(" << servers[i].first << ':' << servers[i].second
           << ") failed: " << worker.redis.ErrorStr();
        error_ = os.str();
        return false;
      }
      if (pipe(worker.wakeup_pipe) != 0) {
        error_ = "init pipe failed";
        return false;
      }
      if (pipe(worker.sleep_pipe) != 0) {
        error_ = "init pipe failed";
        return false;
      }
      worker.pfd.fd = worker.wakeup_pipe[0];
      worker.pfd.events = POLLIN;
      pfds_[i].fd = worker.sleep_pipe[0];
      pfds_[i].events = POLLIN;
    }

    for (int i = 0; i < serverNum_; ++i) {
      if (pthread_create(&(workers_[i].tid), NULL, RunWorker, (void *)this) !=
          0) {
        error_ = "init thread failed";
        return false;
      }
    }
#ifdef CLUSTER_DEBUG
    cluster_out << "init thread success" << std::endl;
#endif
    return true;
  }
  void Clear() {
    for (int i = 0; i < serverNum_; ++i) workers_[i].Clear();
  }
  void Add(const std::string &key) {
    if (key.empty()) return;
    workers_[Hash(key)].kvs.insert(make_pair(key, ""));
  }
  void Add(const std::string &type, const std::string &id,
           const std::string &property) {
    std::string key = type + ':' + id + ':' + property;
    workers_[Hash(key)].kvs.insert(make_pair(key, ""));
  }
  void Fetch() {
    if (!workers_.empty()) {
      Worker &w = workers_[0];
      if (!w.redis.mget(w.kvs)) {
        string err_str = w.redis.ErrorStr();
        /*if (strlen(err_str.c_str()) != 0) {
            ERROR(LOGNAME, "redis mget error:%s[%d]", err_str.c_str(),
        strlen(err_str.c_str()));
        } else
            ERROR(LOGNAME, "redis mget unknown error!");*/
      }
    }
    // int fetchingServerNum = serverNum_;
    // for (int i = 0; i < serverNum_; ++i)
    //    write(workers_[i].wakeup_pipe[1], "x", 1);
    // char buf[128];
    // while (fetchingServerNum != 0) {
    //    if (poll(pfds_, serverNum_, 10000000) <= 0)
    //        continue;
    //    for (int i = 0; i < serverNum_; ++i) {
    //        if (pfds_[i].revents & POLLIN) {
    //            read(pfds_[i].fd, buf, 128);
    //            --fetchingServerNum;
    //        }
    //    }
    //}
  }
  std::string &Get(const std::string &key) {
    return workers_[Hash(key)].kvs[key];
  }
  void foreach() {
    for (int k = 0; k < 1; k++) {
      boost::unordered_map<std::string, std::string>::iterator iter =
          workers_[0].kvs.begin();
      for (; iter != workers_[0].kvs.end(); iter++) {
        printf("%s=%s\t", iter->first.c_str(), iter->second.c_str());
      }
    }
  }
  bool Get(const std::string &key, std::string &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    value = redis_value;
    return true;
  }
  // T must be Integer.
  template <typename T>
  bool Get(const std::string &key, T &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    value = strtol(redis_value.c_str(), NULL, 10);
    return true;
  }
  bool Get(const std::string &key, double &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    value = strtod(redis_value.c_str(), NULL);
    return true;
  }
  bool Get(const std::string &key, std::vector<std::string> &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    std::istringstream is(redis_value);
    std::string element;
    while (getline(is, element, ',')) value.push_back(element);
    return true;
  }
  // T must be Integer.
  template <typename T>
  bool Get(const std::string &key, std::vector<T> &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    std::istringstream is(redis_value);
    std::string element;
    while (getline(is, element, ','))
      value.push_back(strtol(element.c_str(), NULL, 10));
    return true;
  }
  bool Get(const std::string &key, std::vector<double> &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    std::istringstream is(redis_value);
    std::string element;
    while (getline(is, element, ','))
      value.push_back(strtod(element.c_str(), NULL));
    return true;
  }
  bool Get(const std::string &key,
           boost::unordered_map<std::string, std::string> &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    std::istringstream is(redis_value);
    std::string mkv;
    size_t pos;
    while (getline(is, mkv, ',')) {
      if ((pos = mkv.find(':')) == std::string::npos) continue;
      value[mkv.substr(0, pos)] = mkv.substr(pos + 1);
    }
    return true;
  }
  template <typename K, typename V>
  bool Get(const std::string &key, boost::unordered_map<K, V> &value) {
    std::string &redis_value = Get(key);
    if (redis_value == "") return false;
    std::istringstream is(redis_value);
    std::string mkv;
    size_t pos;
    K mkey;
    V mvalue;
    while (getline(is, mkv, ',')) {
      if ((pos = mkv.find(':')) == std::string::npos) continue;
      std::istringstream ismkey(mkv.substr(0, pos));
      ismkey >> mkey;
      std::istringstream ismvalue(mkv.substr(pos + 1));
      ismvalue >> mvalue;
      value[mkey] = mvalue;
    }
    return true;
  }
  template <typename T>
  bool Get(const std::string &type, const std::string &id,
           const std::string &property, T &value) {
    return Get(type + ':' + id + ':' + property, value);
  }
  bool Set(const std::string &key, const std::string &value) {
    return workers_[Hash(key)].redis.set(key, value);
  }
  bool Set(const std::string &type, const std::string &id,
           const std::string &property, const std::string &value) {
    return Set(type + id + property, value);
  }
  const std::string &ErrorStr() const { return error_; }

 private:
  Cluster(const Cluster &rhs);
  Cluster &operator=(const Cluster &rhs);
  static void *RunWorker(void *arg) {
    // Cluster &cluster = *((Cluster *)arg);
    // pthread_t tid = pthread_self();
    // int index = 0;
    // for (int i = 0; i < cluster.serverNum_; ++i) {
    //    if (tid == cluster.workers_[i].tid) {
    //        index = i;
    //        break;
    //    }
    //}
    // Worker &worker = cluster.workers_[index];
    // char buf[128];
    // for (;;) {
    //    if (poll(&(worker.pfd), 1, 10000000) <= 0)
    //        continue;
    //    if (worker.pfd.revents & POLLIN) {
    //        read(worker.pfd.fd, buf, 128);
    //        worker.redis.mget(worker.kvs);
    //        write(worker.sleep_pipe[1], "x", 1);
    //    }
    //}
    return (void *)0;
  }
  /* This is FNV hash */
  uint32_t Hash(const std::string &key) const {
    // unsigned long hash = 2166136261;
    // for (size_t i = 0; i < key.size(); ++i)
    //    hash = (hash * 16777619) ^ key[i];
    // return (uint32_t)hash % serverNum_;
    return 0;
  }

 private:
  class Worker {
   public:
    Worker() : tid(0) {}
    void Clear() { kvs.clear(); }
    pthread_t tid;
    Redis redis;
    boost::unordered_map<std::string, std::string> kvs;
    int wakeup_pipe[2];
    int sleep_pipe[2];
    pollfd pfd;
  };
  INIReader *conf_;
  int serverNum_;
  std::vector<Worker> workers_;
  pollfd *pfds_;
  std::string error_;
};
}
}  // namespace rank
#endif
