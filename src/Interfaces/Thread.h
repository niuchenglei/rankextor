/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

namespace rank {

#define THREAD_CANCEL_POINT pthread_testcancel();

/*****************************************************************************
 *  Thread 类
 *  多线程基类，在需要实现多线程的地方，实现此类即可
 *****************************************************************************/
class Thread {
 public:
  /**
   *  构造函数.
   */
  Thread();

  /**
   *  析构函数.
   */
  ~Thread();

  /**
   *  获取当前线程ID.
   *
   *  @return  返回线程ID.
   */
  static unsigned long int getCurrentThreadId();

  /**
   *  线程休眠.
   *
   *  @param  second
   *          休眠时间，单位：秒.
   */
  static void Sleep(int second);

  /**
   *  多线程实现函数，继承自Thread类.
   *
   *  @return  返回执行情况，0代表成功，非0代表失败.
   */
  virtual int Exec();

  /**
   *  线程开启函数.
   */
  void Start();
  int Run();

  /**
   *  线程结束函数.
   */
  void Stop();

  /**
   *  等待线程结束.
   *
   *  @return  返回线程返回代码.
   */
  int Wait();

  /**
   *  获取线程运行状态.
   *
   *  @return 返回执行情况，true代表正在运行，false代表线程结束.
   */
  bool Running() const;

  /**
   *  获取线程终止信号.
   *
   *  @return
   *返回执行情况，true代表终止信号有效（调用Stop后产生），false代表无效.
   */
  bool getSignStop() const;

  int _exit_code;

 protected:
  pthread_t _pid;
  bool _sign_stop, _is_stopped;
};
}  // namespace rank
#endif
