#include <fstream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <cmath>
#include <iomanip>
#include <sys/mman.h>
#include <boost/unordered_map.hpp>
#include <map>
#include <dlfcn.h>
#include <Config.h>
//#include "Logger.h"
#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/lexical_cast.hpp>
#include <pthread.h>

#include "rank_interface.pb.h"

using namespace std;

//#define RANKLIB "./libmtai.so"
#define RANKLIB "./libproxy_local.so"
//#define RANKLIB "./libmtai.so"

int LL = 10;
typedef int (*MTAI_INIT)(char* config_file, int worker_num);
typedef char* (*MTAI_PREDICT)(const char* request, int bytes_request, int worker_idx, int* bytes_write);
typedef void (*MTAI_FREE)(char* ptr);

#define __F(v) keys.push_back(v)

struct thread_arg {
  void* handle;
  int idx;
};

void* mtai_func(void *arg) {
  void* handle = (void*)(((thread_arg*)arg)->handle);
  MTAI_INIT func1 = NULL;
  MTAI_PREDICT func2 = NULL;
  MTAI_FREE func3 = NULL;
  func1 = (MTAI_INIT)dlsym(handle, "mtai_init");
  func2 = (MTAI_PREDICT)dlsym(handle, "mtai_predict");
  func3 = (MTAI_FREE)dlsym(handle, "mtai_free");
  if (!func1) {
    printf("dlsym error");
    return false;
  }
  printf("%x %x %x\n", func1, func2, func3);

  pid_t pid = getpid();
  srand((unsigned)time(NULL));

  ::rank::interface::Union _union; // = new ::rank::interface::Union();
  ::rank::interface::RequestInfo* request = _union.mutable_request();
  ::rank::interface::UserInfo* user = request->mutable_user();

  ::rank::interface::kv_ss* kv = NULL;
  kv = request->add_extend_ss();
  kv->set_key("uid"); kv->set_value("863872048356495");
  kv = request->add_extend_ss();
  kv->set_key("interest"); kv->set_value("food/sport");
  kv = request->add_extend_ss();
  kv->set_key("exchange_id"); kv->set_value("2343943534");
  kv = request->add_extend_ss();
  kv->set_key("media_id"); kv->set_value("123231");
  kv = request->add_extend_ss();
  kv->set_key("client_type"); kv->set_value("iphone");
  kv = request->add_extend_ss();
  kv->set_key("client_id"); kv->set_value("123");
  kv = request->add_extend_ss();
  kv->set_key("slot_size"); kv->set_value("300");
  kv = request->add_extend_ss();
  kv->set_key("slot_width"); kv->set_value("100");
  kv = request->add_extend_ss();
  kv->set_key("slot_height"); kv->set_value("30");
  
  request->set_uid("863872048356495");
  request->set_client_id("123");

  kv = request->add_extend_ss();
  kv->set_key("unit:rtlimit:update:flag"); kv->set_value("0");

  kv = user->add_extend_ss();
  string user_feature = "-,4,N,0,0,0,-,-,3,2,-,N,1,161000000110,1,0,3,-,1,0,1,1,0,1,1,0,0,0,0,0,0,3,1,0,0,0,1,3,1,0,0,0,0,0,0,140000000020,0,0,1,1,0,0,3,3,1,0,0,0|$|126679576,1,1,0,0,0,0,0,0,0,0,3678,99.0%,0.30806,4671,4624,89.59,0#180322815,0,1,0,0,0,0,0,0,0,0,0,100.0%,1.2E-5,29,29,89.98,0#11020224054,0,1,0,0,0,0,0,0,0,0,615,99.0%,0.08455199999999999,2219,2196,95.94,0#180322815,0,1,0,0,0,0,0,0,0,0,4,99.0%,6.48E-4,483,478,96.98,0#180322815,0,1,0,0,0,0,0,0,0,0,,99.0%,9.4E-4,85,84,83.32,0#129012925,1,1,0,0,0,0,0,0,0,0,284,98.0%,0.038363999999999995,22157,21713,87.09,0#10573802325,1,1,0,0,0,0,0,0,0,0,2,100.0%,2.1600000000000002E-4,13,13,82.57,0#180322815,0,1,0,0,0,0,0,0,0,0,2867,99.0%,0.354672,35293,34940,99.65,0#180322815,0,1,0,0,0,0,0,0,0,0,2,99.0%,6.839999999999999E-4,911,901,96.98,0#10573802325,1,1,0,0,0,0,0,0,0,0,0,100.0%,0,6,6,85.57,0#144749053,0,1,0,0,0,0,0,0,0,0,2,99.0%,2.2E-4,198,196,77.09,0#10573802325,1,1,0,0,0,0,0,0,0,0,0,,0,-,-,86.57,0#129013913,0,1,0,0,0,0,0,0,0,0,0,,0,-,-,87.09,0#11356496952,0,1,0,0,0,0,0,0,0,0,-,-,-,-,-,-,0#129012925,1,1,0,0,0,0,0,0,0,0,1,100.0%,3.96E-4,19,19,81.09,0#129012925,1,1,0,0,0,0,0,0,0,0,46,,0.00399,-,-,87.09,0#129013913,0,1,0,0,0,0,0,0,0,0,182,97.0%,0.021672,6188,6002,91.09,0#180322815,0,1,0,0,0,0,0,0,0,0,4,98.0%,5.639999999999999E-4,507,496,94.32,0|$|20344,0,0,0,0,1,1,0,0,0,1|$|000480163,0,0,0,0#000130163,0,0,0,0#000481893,0,0,0,0#000131893,0,0,0,0#000071893,0,0,0,0#000130743,0,0,0,0#000040743,0,0,0,0#000190743,0,0,0,0#000060743,0,0,0,0#000070743,0,0,0,0#000020743,0,0,0,0#000150743,0,0,0,0#000010743,0,0,0,0#000480743,0,0,0,0#000030743,0,0,0,0#000460743,0,0,0,0#000050743,0,0,0,0#000290743,0,0,0,0#000450743,0,0,0,0#000135I14,0,0,0,0|$|20268200003_456003030003_315587050003_-500003_500353050003_457529300003_20358600003_20061080003_20268200007_456003030007_315587050007_-600007_500353050007_457529300007_20358600007_20061080007_20268200015_456003070015_315587050015_-600015_500353050015_457529300015_20358600115_20061080015,-500003_243504100003_20320600003_251027060003_293006100003_500365050003_456028030003_161645020003_316549050003_457551200003_457582070003_20326070003_-600007_243504100007_20320600007_251027060007_293006100007_500365050007_456028030007_161645020007_316549050007_457551200007_457582070007_20326070007_-600015_243504100015_20320600115_251027060015_293006100015_500365050015_456028070015_161645020015_316549050015_457551200015_457582070015_20326070015";
  kv->set_key("user_feature"); kv->set_value(user_feature);

  for (int i=0; i<LL; i++){
    ::rank::interface::AdInfo* ad = _union.add_ads();
    ::rank::interface::ItemInfo* item = _union.add_items();
    item->set_item_id(1000+i*2);
    ::rank::interface::ItemInfo* item2 = _union.add_items();
    item2->set_item_id(1000+i*2+1);

    kv = item->add_extend_ss();
    kv->set_key("gds_property"); kv->set_value("123#315587#500365#category_3#category_4#geli#0#10#50#-#-#-#-#-#-#4#50#50#0#0#0#0#0#0#-");

    kv = item2->add_extend_ss();
    kv->set_key("gds_property"); kv->set_value("123#category_1#category_2#category_3#category_4#geli#0#10#50#-#-#-#-#-#-#4#50#50#0#0#0#0#0#0#-");

    ad->set_ad_id(i);
    ad->set_bid_price(10000000);
    ad->set_max_bid_price(20000000);
    ad->set_bid_type(1);
    kv = ad->add_extend_ss();
    kv->set_key("unit_rt_limits"); kv->set_value("10000,20,100000000.0,12342,1000.0,5000.0,123,5000.0");  //pv,click,consume,custno,budget,pbudget,pid,balance
    kv = ad->add_extend_ss();
    kv->set_key("unit_rt_pvclk"); kv->set_value("10200,25");
    kv = ad->add_extend_ss();
    kv->set_key("solution_id"); kv->set_value("1232");
    kv = ad->add_extend_ss();
    kv->set_key("creative_id"); kv->set_value("1233");

    ::rank::interface::kv_ii* ii = ad->add_available_items();
    ii->set_key(1000+i*2); ii->set_value(i);
    ii->set_key(1000+i*2+1); ii->set_value(i);
  }

  string request_str;
  _union.SerializeToString(&request_str);
  //delete _union;

  //printf("--------------------------------------\n\n\nrequest = %s\n", request.c_str());
  //
  //
  struct timespec start, end;
  printf("begin invoke predict\n");
  
  int cnt = 0;
  long total = 0;
  while (true) { //cnt < 20) {
    long timeuse = 0;
    string res = "";
    for (int n=0; n<10; n++) {
      clock_gettime(CLOCK_MONOTONIC, &start);

      int worker_idx = (int)(rand()%10);
      int bytes_write = 0;
      //std::string request = "uid:19891111,tag:food;id:123,category:cloth;id:234,category:food";
      char* response = func2(request_str.c_str(), request_str.size(), worker_idx, &bytes_write);

      if (response == NULL || bytes_write == 0) {
        //sleep(1);
        clock_gettime(CLOCK_MONOTONIC, &end);
        timeuse += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        continue;
      }

      ::rank::interface::ResponseInfo _response;
      if (_response.ParseFromArray(response, bytes_write)) {
        if (_response.code() != 0)
          printf("res: %d, %s\n", _response.code(), _response.msg().c_str());
        else {
          //printf("[ok] %d %d %ld\n", _response.ads_size(), _response.items_size(), _response.pairs_size());
        }
      } else printf("response parse error[%s][%d]", response, strlen(response));

      func3(response);
      //sleep(1);
      clock_gettime(CLOCK_MONOTONIC, &end);
      timeuse += (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
    } 
    int ind = ((thread_arg*)arg)->idx;
    printf("[%u] request qps: %.2f ms/req, %.2f us/req\n", ind, ((float)timeuse)/100.0/1000.0, ((float)timeuse)/100.0);

    total += timeuse;
    cnt += 1;
    //printf("func2 = %s\n", res.c_str());
  }
  printf("all request over %.2f ms, %d request\n", ((float)total)/1000.0, 20*100);

  pthread_exit(NULL);
}

void exec_mtai(int THREAD_NUMBER, void* handle) {

  pthread_t thread[THREAD_NUMBER];
  thread_arg arg[THREAD_NUMBER];

  int no = 0, res;
  void * thrd_ret;
  srand(time(NULL));
  for (no = 0; no < THREAD_NUMBER; no++) {
    /* 创建多线程 */
    arg[no].idx = no; arg[no].handle = handle;
    res = pthread_create(&thread[no], NULL, mtai_func, (void*)(arg+no));
    if (res != 0) {
      printf("Create thread %d failed\n", no);
      exit(res);
    }
  }

  printf("Create treads success\n Waiting for threads to finish...\n");
  for (no = 0; no < THREAD_NUMBER; no++) {
    /* 等待线程结束 */
    res = pthread_join(thread[no], &thrd_ret);
    if (!res) {
      printf("Thread %d joined\n", no);
    } else {
      printf("Thread %d join failed\n", no);
    }
  }
}


int load_library(int argc, char* argv[], const char* libso = NULL) {
  //void* handle = dlmopen(lmid, RANKLIB, RTLD_LAZY|RTLD_LOCAL);
  void* handle = dlopen(RANKLIB, RTLD_NOW);
  if (!handle) {
    printf("dlopen error test, %s", dlerror());
    return 0;
  }
  printf("-------------------------\ndlopen ok %x\n", handle);

  MTAI_INIT func1 = NULL;
  func1 = (MTAI_INIT)dlsym(handle, "mtai_init");
  if (!func1) {
    printf("dlsym error\n");
    return 0;
  }
  printf("dlsym(mtai_init) ok\n");


  int THREAD_NUMBER = atoi(argv[1]);
  if (argc >= 3)
    LL = atoi(argv[2]);

  int v = func1("../conf/libranking.so.conf", 1);
  printf("mtai_init = %d\n", v);

/*{
  void* handle = dlopen("./libmtai.so.1", RTLD_NOW|RTLD_DEEPBIND);
  if (!handle) {
    printf("dlopen error test, %s", dlerror());
    return 0;
  }
  printf("-------------------------\ndlopen ok %x\n", handle);

  MTAI_INIT func1 = NULL;
  func1 = (MTAI_INIT)dlsym(handle, "mtai_init");
  if (!func1) {
    printf("dlsym error\n");
    return 0;
  }
  printf("dlsym(mtai_init) ok\n");

  int v = func1("../conf/libranking.so.conf", 1);
  printf("func1 = %d\n", v);
}
return true; 
*/
 
  exec_mtai(THREAD_NUMBER, handle);
//while (true) sleep(4);
  if (dlclose(handle) != 0) {
    printf("dlclose error test, %s\n", dlerror());
    return 0;
  }
  printf("dlclose ok\n-------------------------\n");

  return true;
}


int main(int argc, char* argv[]) {
/*int c=atoi(argv[1]);
int cnt=0;
while (cnt<c) {
cnt++;
int64_t ss = 1024*1024*1024;

printf("malloc %ld\n", ss);
void* ptr = (void*)malloc(ss);
char* _p = (char*)ptr;
for (size_t i=0; i<ss; i++) 
  _p[i] = 0;

  sleep(1);
}
while(true){sleep(10);}
return 0;
*/


  /*struct timeval m1, m2;
  gettimeofday(&m1, NULL);
  sleep(1);
  gettimeofday(&m2, NULL);
  printf("%ld, %ld\n", m1.tv_sec, m1.tv_usec);
  printf("%ld, %ld\n", m2.tv_sec, m2.tv_usec);
  printf("m1>m2 %d\n", timercmp(&m1, &m2, >));
  printf("m1=m2 %d\n", timercmp(&m1, &m2, =));
  printf("m1<m2 %d\n", timercmp(&m1, &m2, <));
  printf("m1!=m2 %d\n", timercmp(&m1, &m2, !=));
return 0;*/
/*
  string pvhis_c1 = "157122600003_-080003_20089100003_157122600107_-300007_420003010007_20089200107_457529020015_20268020015_157122600115_-600215_20061020015_420003050015_313118020015_503915040015_315053010015_20089600115_361003010015";

      string ad_pvhis_c1 = "_";
      int from = 0, cnt = 0;
      while (cnt < 10) {
        cnt++;
        size_t ind1 = pvhis_c1.find("123361003", from); //item.extend["category_1"].c_str(), from);
        size_t ind2 = pvhis_c1.find_first_of("_", ind1+1);
        if (ind2 == string::npos)
          ind2 = pvhis_c1.length();
        from += ind2+1;
        if (ind1 == string::npos)
          break;

        string v = "-";
        if (ind1 > 0 && ind2 > 0 && ind2 > ind1)
          v = pvhis_c1.substr(ind1, ind2-ind1);
        if (v == "__")
          v = "-";

       printf("v:%s, %d, %d, %s\n", v.c_str(), ind1, ind2, pvhis_c1.c_str());

        if (v != "-")
          ad_pvhis_c1 +=  v + "_";
      }
      ad_pvhis_c1 = ad_pvhis_c1.substr(1, ad_pvhis_c1.length()-2);
*/

/*float finf = 1.0/0.0f;
float finf2 = -finf;

float v=min(finf, 100.0f), v2=min(finf2, 100.0f);
printf("%f,%f,%f,%f\n", finf, finf2, v, v2);

float v3 = 0-finf, v4 =0*finf, v5=3+finf;
printf("%f,%f,%f, %d, %d, %d\n", v3, v4, v5, isinf(v3), isnan(v4), isinf(v5));

return 0;
*/


  load_library(argc, argv);

  //load_library(argc, argv);
  return 0;
}
