/*
 * This file is part of WEIBO AD RANKING
 *
 * Any question contact with weibo ad department support
 */

#ifndef TIMER_H
#define TIMER_H

#include "Interfaces/Thread.h"

namespace rank {

/*****************************************************************************
 *  回调函数类型声明
 *****************************************************************************/
typedef void (*CALLBACK_FUNC)(void* data);

/*****************************************************************************
 *  Timer 类
 *  实现计时功能，在时间结束时调用回调函数
 *****************************************************************************/
class Timer : public Thread {
 public:
  /**
   *  构造函数.
   */
  Timer();

  /**
   *  析构函数.
   */
  ~Timer();

  /**
   *  设置执行次数.
   *
   *  @param  count
   *          计时器有效的次数，当计时器到达超时时间后调用回调函数算作1次.
   */
  void setCount(int count);

  /**
   *  设置计时间隔.
   *
   *  @param  seconds
   *          计时器超时时限，单位：秒.
   */
  void setInterval(int seconds);

  /**
   *  设置回调函数.
   *
   *  @param  callback
   *          计时器超时的回调函数.
   *
   *  @param  data
   *          传入回调函数的数据.
   */
  void setCallback(CALLBACK_FUNC callback, void* data);

  /**
   *  多线程实现函数，继承自Thread类.
   *
   *  @return  返回执行情况，0代表成功，非0代表失败.
   */
  virtual int Exec();

 protected:
  int count_number;
  int interval;
  CALLBACK_FUNC callback_func;
  void* data;
};
}  // namespace rank
#endif
