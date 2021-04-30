/**
* Copyright (c) 2017-present, Facebook, Inc.
* All rights reserved.

* This source code is licensed under the BSD-style license found in the
* LICENSE file in the root directory of this source tree.
*/

#pragma once

#include "blockingconcurrentqueue.h"
#include <condition_variable>
#include <mutex>

template <typename T>
using CCQueue2 = moodycamel::BlockingConcurrentQueue<T>;

#ifdef USE_TBB
  #include <tbb/concurrent_queue.h>

  template <typename T>
  using CCQueue = tbb::concurrent_queue<T>;

  template <typename T>
  void push_q(CCQueue<T>& q, const T &val) {
     q.push(val);
  }

  template <typename T>
  void pop_wait(CCQueue<T> &q, T &val) {
    while (true) {
      if (q.try_pop(val)) break;
    }
  }

  template <typename T>
  bool pop_wait_time(CCQueue<T> &q, T &val, int time_usec) {
    if (! q.try_pop(val)) {
      std::this_thread::sleep_for(std::chrono::microseconds(time_usec));
      if (! q.try_pop(val)) return false;
    }
    return true;
  }
#else
  template <typename T>
  using CCQueue = moodycamel::BlockingConcurrentQueue<T>;

  template <typename T>
  void push_q(CCQueue<T>& q, const T &val) {
     q.enqueue(val);
  }

  template <typename T>
  void pop_wait(CCQueue<T> &q, T &val) {
     q.wait_dequeue(val);
  }

  template <typename T>
  bool pop_wait_time(CCQueue<T> &q, T &val, int time_usec) {
     return q.wait_dequeue_timed(val, time_usec);
  }
#endif

class SemaCollector {
private:
    int _count;
    std::mutex _mutex;
    std::condition_variable _cv;

public:
    SemaCollector() : _count(0) { }

    inline void notify() {
        std::unique_lock<std::mutex> lock(_mutex);
        _count ++;
        _cv.notify_one();
    }

    inline int wait(int expected_count, int usec = 0) {
        if (expected_count == 0) return _count;

        std::unique_lock<std::mutex> lock(_mutex);
        if (usec == 0) {
            _cv.wait(lock, [this, expected_count]() { return _count >= expected_count; });
        } else {
            _cv.wait_for(lock, std::chrono::microseconds(usec),
                [this, expected_count]() { return _count >= expected_count; });
        }
        return _count;
    }

    inline void reset() {
        std::unique_lock<std::mutex> lock(_mutex);
        _count = 0;
        _cv.notify_all();
    }
};

class Notif {
private:
    std::atomic_bool _flag;   // flag to indicate stop
    SemaCollector _counter;

public:
    Notif() : _flag(false) { }
    const std::atomic_bool &flag() const { return _flag; }
    bool get() const { return _flag.load(); }
    void notify() { _counter.notify(); }

    void set() { _flag = true; }
    void wait(int n, std::function<void ()> f = nullptr) {
        _flag = true;
        if (f == nullptr) _counter.wait(n);
        else {
            while (true) {
              int current_cnt = _counter.wait(n, 10);
              // std::cout << "current cnt = " << current_cnt << " n = " << n << std::endl;
              if (current_cnt >= n) break;
              f();
            }
        }
    }
    void reset() {
        _counter.reset();
        _flag = false;
    }
};

template <typename T>
class Semaphore {
private:
    bool _flag;
    T _val;
    std::mutex _mutex;
    std::condition_variable _cv;

    inline void _raw_wait(std::unique_lock<std::mutex> &lock, int usec) {
        if (usec == 0) {
            _cv.wait(lock, [this]() { return _flag; });
        } else {
            _cv.wait_for(lock, std::chrono::microseconds(usec),  [this]() { return _flag; });
        }
    }

public:
    Semaphore() : _flag(false) { }

    inline void notify(T val) {
        std::unique_lock<std::mutex> lock(_mutex);
        _flag = true;
        _val = val;
        _cv.notify_one();
    }

    inline bool wait(T *val, int usec = 0) {
        std::unique_lock<std::mutex> lock(_mutex);
        _raw_wait(lock, usec);
        if (_flag) *val = _val;
        return _flag;
    }

    inline bool wait_and_reset(T *val, int usec = 0) {
        std::unique_lock<std::mutex> lock(_mutex);
        _raw_wait(lock, usec);
        bool last_flag = _flag;
        if (_flag) *val = _val;
        _flag = false;
        return last_flag;
    }

    inline void reset() {
        std::unique_lock<std::mutex> lock(_mutex);
        _flag = false;
    }
};
