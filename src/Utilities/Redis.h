#ifndef REDIS_H
#define REDIS_H

#include <sstream>
#include <iostream>
#include <fstream>
#include <tr1/unordered_map>

#include <errno.h>

#include "hiredis/hiredis.h"
#include "hiredis/async.h"

//#define REDIS_DEBUG

//#ifdef REDIS_DEBUG
// static std::ofstream redis_out("/data0/vad/ms/log/redis.log",
// std::ios_base::app);
//#endif

#define freeReplyObject(n) ;
#define redisConnectWithTimeout

using std::string;

namespace rank {

class Redis {
 public:
  Redis() : port_(0), db_(0), isConnected_(false), rc_(NULL) {}
  ~Redis() {
    //if (rc_) 
      //redisFree(rc_);
  }
  Redis(const Redis &orig) {
    host_ = orig.host_;
    port_ = orig.port_;
    db_ = orig.db_;
    isConnected_ = false;
    rc_ = NULL;
    error_ = "";
  }
  bool init(const std::string &host, int port, int db = 0, int timeout = 30) {
    host_ = host;
    port_ = port;
    db_ = db;
    isConnected_ = false;
    rc_ = NULL;
    return true; //connect(timeout);
  }
  bool connect(int milliseconds = 3) {
    if (isConnected_) return true;
    /*if (rc_) redisFree(rc_);*/
    rc_ = NULL;
    struct timeval tv = {milliseconds / 1000, milliseconds * 1000};
    //rc_ = redisConnectWithTimeout(host_.c_str(), port_, tv);
    if (rc_ == NULL) return false;
    if (rc_->err) {
      error_ = rc_->errstr;
      redisFree(rc_);
      rc_ = NULL;
      return false;
    }
    //#ifdef REDIS_DEBUG
    // redis_out << "connect " << host_ << ":" << port_ << " success" <<
    // std::endl;
    //#endif
    // if (!select(db_)) return false;
    isConnected_ = true;
    return true;
  }
  bool get(const std::string &key, std::string &value) {
    if (!connect()) return false;
    redisReply *reply = NULL; //(redisReply *)redisCommand(rc_, "GET %s", key.c_str());
    if (reply == NULL) {
      isConnected_ = false;
      return false;
    } else if (reply->type == REDIS_REPLY_STRING) {
      char *str = reply->str;
      if (str == NULL)
        value = "";
      else
        value = str;
      freeReplyObject(reply);
      reply = NULL;
      return true;
    } else if (reply->type == REDIS_REPLY_NIL) {
      value = "";
      freeReplyObject(reply);
      reply = NULL;
      return true;
    } else {
      isConnected_ = false;
      freeReplyObject(reply);
      reply = NULL;
      return false;
    }
  }
  bool mget(boost::unordered_map<std::string, std::string> &kvs) {
    error_ = "";
    if (kvs.empty()) return true;
    /*if (!connect()) {
      if (rc_)
        if (rc_->err) error_ = rc_->errstr;
      return false;
    }*/
    std::ostringstream os;
    os << "MGET ";
    boost::unordered_map<std::string, std::string>::iterator it = kvs.begin();
    for (; it != kvs.end(); ++it) os << it->first << " ";
    redisReply *reply = NULL; //(redisReply *)redisCommand(rc_, os.str().c_str());
    if (reply == NULL) {
      isConnected_ = false;
      return false;
    } else if (reply->type == REDIS_REPLY_ARRAY &&
               reply->elements == kvs.size()) {
      it = kvs.begin();
      for (size_t i = 0; i < kvs.size(); ++i, ++it) {
        if (reply->element[i]->type == REDIS_REPLY_ERROR) {
          error_ = string(reply->element[i]->str);
        }
        if (reply->element[i]->type != REDIS_REPLY_STRING) {
          it->second = "";
          continue;
        }
        char *str = reply->element[i]->str;
        if (str == NULL)
          it->second = "";
        else
          it->second = string(str);
      }
      if (reply != NULL) {
        freeReplyObject(reply);
        reply = NULL;
      }
      return true;
    } else {
      // if (REDIS_REPLY_ERROR == reply->type)
      //    error_ = "REDIS_REPLY_ERROR, detail:" + string(reply->str);
      error_ = "REDIS_REPLY_ERROR";
      isConnected_ = false;
      if (reply != NULL) {
        freeReplyObject(reply);
        reply = NULL;
      }
      return false;
    }
  }
  bool select(int db) {
    redisReply *reply = NULL; //(redisReply *)redisCommand(rc_, "select %d", db);
    if (reply == NULL) {
      return false;
    } else {
      freeReplyObject(reply);
      reply = NULL;
      return true;
    }
  }
  bool set(const std::string &key, const std::string &value) {
    if (!connect()) return false;
    redisReply *reply = NULL; //(redisReply *)redisCommand(rc_, "SETEX %s 172800 %s",
                                //                   key.c_str(), value.c_str());
    if (reply == NULL) {
      isConnected_ = false;
      return false;
    }
    return true;
  }
  const std::string &ErrorStr() const { return error_; }

 private:
  std::string host_;
  int port_;
  int db_;
  bool isConnected_;
  redisContext *rc_;
  std::string error_;
};

}  // namespce rank

#endif
