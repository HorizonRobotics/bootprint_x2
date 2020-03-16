//
// Copyright(c) 2018 Horizon Robotics.
// Created by wenhao.zou on 12/20/18.
//

#ifndef SRC_COMMON_HOBOT_UTILILTY_H_
#define SRC_COMMON_HOBOT_UTILILTY_H_
//#include <google/protobuf/stubs/scoped_ptr.h>
//#include <google/protobuf/stubs/shared_ptr.h>
#include <pthread.h>
#include <sys/inotify.h>
#include <atomic>
#include <string>
#include <map>
#include <vector>

namespace sunrise {

#ifdef __USE_ISOC99
static const int64_t kNumMillisecsPerSec = int_least64_t(1000);
static const int64_t kNumMicrosecsPerSec = int_least64_t(1000000);
static const int64_t kNumNanosecsPerSec = int_least64_t(1000000000);
#else
static const int64_t kNumMillisecsPerSec = INT64_C(1000);
static const int64_t kNumMicrosecsPerSec = INT64_C(1000000);
static const int64_t kNumNanosecsPerSec = INT64_C(1000000000);
#endif
static const int64_t kNumMicrosecsPerMillisec =
    kNumMicrosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMillisec =
    kNumNanosecsPerSec / kNumMillisecsPerSec;
static const int64_t kNumNanosecsPerMicrosec =
    kNumNanosecsPerSec / kNumMicrosecsPerSec;

//#define hobot_shared_ptr google::protobuf::internal::shared_ptr
//#define hobot_scoped_ptr google::protobuf::internal::scoped_ptr

/******************************************************************************
*[Function]: TimeMillis()
 [Description]: To Get represents monotonic time. Cannot be set
 [Return]: Monotonic time as milli second unit
 *****************************************************************************/
int64_t TimeMillis();

/******************************************************************************
*[Function]: TimeMicros()
 [Description]: To Get represents monotonic time. Cannot be set
 [Return]: Monotonic time as micro second unit
 *****************************************************************************/
int64_t TimeMicros();

/******************************************************************************
*[Function]: TimeNanos()
*[Description]: To Get represents monotonic time. Cannot be set
[Return]: Monotonic time as nano second unit
 *****************************************************************************/
int64_t TimeNanos();

/******************************************************************************
 *[Function]: TimeSecond()
 *[Description]: To Get represents monotonic time. Cannot be set
 *[Return]: Monotonic time as second unit
 *****************************************************************************/
int64_t TimeSecond();

/******************************************************************************
 *[Function]: str_trim()
 *[Description]: string trim space before and end
 *[Return]:string after trim
 *****************************************************************************/
std::string& str_trim(std::string& s);

/*********************************************************
 *[Class]:     HobotMutex
 *[Description]: The HobotMutex is used to encapsulates the mutex
 **********************************************************/
class HobotMutex {
 public:
  friend class HobotCond;
  /******************************************************************************
   *[Function]: HobotMutex()
   *[Description]:constructor
   *****************************************************************************/
  HobotMutex();

  /******************************************************************************
   *[Function]: ~HobotMutex()
   *[Description]:destructor
   *****************************************************************************/
  ~HobotMutex();

  /******************************************************************************
  *[Function]: Lock()
  *[Description]:Blocking locking mutex, only return when locked or other error
  *[Return]:return result of lock
            0: success
            other: fail
  *******************************************************************************/
  int Lock();

  /******************************************************************************
  *[Function]: TryLock()
  *[Description]:try to lock mutex, it will return immediately
  *[Return]:return result of lock
            0: success
            other: fail
  *******************************************************************************/
  int TryLock();

  /******************************************************************************
  *[Function]: imedLock()
  *[Description]:try to lock mutex in wait_for_millisecs millisec second
  *[Return]:return result of lock
            0: success
            other: fail
  *******************************************************************************/
  int TimedLock(int64_t wait_for_millisecs);

  /******************************************************************************
  *[Function]: Unlock()
  *[Description]:try to unlock mutex
  *[Return]:return result of lock
            0: success
            other: fail
  *******************************************************************************/
  int Unlock();

 protected:
  pthread_mutex_t _mutex;
};

/******************************************************************************
 *[Class]:     HobotRWMutex
 *[Description]: The HobotRWMutex is used to encapsulates the read/write mutex
 ******************************************************************************/
class HobotRWMutex {
 public:
  /*define friend class HobotReadLock*/
  friend class HobotReadLock;
  /*define friend class HobotWriteLock*/
  friend class HobotWriteLock;

  /******************************************************************************
   *[Function]: HobotRWMutex()
   *[Description]:constructor function
   *******************************************************************************/
  HobotRWMutex();

  /******************************************************************************
   *[Function]: HobotRWMutex()
   *[Description]:destructor function
   *******************************************************************************/
  ~HobotRWMutex();

 protected:
  pthread_rwlock_t _rw_mutex;
};

/******************************************************************************
 *[Class]:     HobotUniqueLock
 *[Description]: The HobotUniqueLock is used to encapsulates locking mutex
 ******************************************************************************/
class HobotUniqueLock {
 public:
  /******************************************************************************
  *[Function]: HobotUniqueLock()
  *[Description]:constructor function
                 if wait_for_millisecs == 0, the instance of HobotUniqueLock
  will be locked after be constructored if wait_for_millisecs > 0, the instance
  of HobotUniqueLock will try to lock mutex in wait_for_millisecs milli seconds.
  In fact, instead of trying wait_for_millisecs times at 1 millisecond
                 intervals.
  *******************************************************************************/
  explicit HobotUniqueLock(HobotMutex& mutex, int64_t wait_for_millisecs = 0);

  /******************************************************************************
   *[Function]: ~HobotUniqueLock()
   *[Description]:destructor function
   *******************************************************************************/
  ~HobotUniqueLock();

  /******************************************************************************
   *[Function]: Unlock()
   *[Description]:unlock the mutex
   *******************************************************************************/
  void Unlock();

  /******************************************************************************
   *[Function]: IsLocked()
   *[Description]:return if mutex is locked by this instance
   *******************************************************************************/
  bool IsLocked();

 protected:
  HobotMutex& _Mutex;
  std::atomic<bool> isLocked;
};

/******************************************************************************
 *[Class]:     HobotReadLock
 *[Description]: The HobotReadLock is used to encapsulates Reading lock
 ******************************************************************************/
class HobotReadLock {
 public:
  /******************************************************************************
  *[Function]: HobotReadLock()
  *[Description]:constructor function,
                 the lock of HobotRWMutex will be shared between HobotReadLock
  *******************************************************************************/
  explicit HobotReadLock(HobotRWMutex& mutex);

  /******************************************************************************
  *[Function]:  ~HobotReadLock()
  *[Description]:destructor function
                 it will unlock HobotRWMutex instance self-acting
  *******************************************************************************/
  ~HobotReadLock();

  /******************************************************************************
   *[Function]:  Unlock()
   *[Description]:Unlock Read lock
   *******************************************************************************/
  void Unlock();

 protected:
  HobotRWMutex& _hbt_rw_mutex;
  std::atomic<bool> isLocked;
};

/******************************************************************************
 *[Class]:     HobotCond
 *[Description]: The HobotCond is used to Cond wait
 ******************************************************************************/
class HobotCond {
 public:
  /******************************************************************************
  *[Function]: HobotCond()
  *[Description]:constructor function,

  *******************************************************************************/
  HobotCond();

  /******************************************************************************
  *[Function]: ~HobotCond()
  *[Description]:destructor function,

  *******************************************************************************/
  ~HobotCond();

  /******************************************************************************
   *[Function]: TimedWait(int64_t wait_for_millisecs)
   *[Description]:Wait sginal for mutex
   *******************************************************************************/
  int TimedWait(int64_t wait_for_millisecs);

  /******************************************************************************
   *[Function]: Signal(int64_t wait_for_millisecs)
   *[Description]:send sginal to waiting thread mutex
   *******************************************************************************/
  int Signal(int64_t wait_for_millisecs);

  /******************************************************************************
   *[Function]: Broadcast(int64_t wait_for_millisecs)
   *[Description]:Broadcast sginal to waiting thread mutex
   *******************************************************************************/
  int Broadcast(int64_t wait_for_millisecs);

 protected:
  pthread_cond_t _cond;
  pthread_condattr_t _cond_attr;
  HobotMutex _Mutex;
};

/******************************************************************************
 *[Class]:     HobotWriteLock
 *[Description]: The HobotWriteLock is used to encapsulates Writing lock
 ******************************************************************************/
class HobotWriteLock {
 public:
  /******************************************************************************
  *[Function]: HobotWriteLock()
  *[Description]:constructor function,
                 HobotWriteLock will exclusively to get lock HobotRWMutex
  *******************************************************************************/
  explicit HobotWriteLock(HobotRWMutex& mutex);

  /******************************************************************************
  *[Function]:  ~HobotWriteLock()
  *[Description]:destructor function
                 it will unlock HobotRWMutex instance self-acting
  *******************************************************************************/
  ~HobotWriteLock();
  void Unlock();

 protected:
  HobotRWMutex& _hbt_rw_mutex;
  std::atomic<bool> isLocked;
};

/******************************************************************************
 *[Class]:     TimeStatis
 *[Description]: The TimeStatis is used to statis time
 ******************************************************************************/
class TimeStatis {
 public:
  /******************************************************************************
  *[Function]:  TimeStatis()
  *[Description]:constructor function,
                 when constructor,it will start to record time,
  *******************************************************************************/
  explicit TimeStatis(std::string name);

  /******************************************************************************
  *[Function]:   Duration()
  *[Description]:Duration,
                 when call this function, to get current time, then return the
                 duration between current time and start time.
  *[Return]:    the duration between current time and start time.
  *******************************************************************************/
  int64_t Duration();

  /******************************************************************************
   *[Function]:  ~TimeStatis()
   *[Description]:destructor function
   *******************************************************************************/
  ~TimeStatis();

 private:
  /******************************************************************************
   *[Member]:  _start_time
   *[Description]: the start_time
   *******************************************************************************/
  int64_t _start_time;
  std::string _name;
};

/******************************************************************************
 *[Class]:     CommonThread
 *[Description]: Common thread for encapsulating thread
 ******************************************************************************/
class CommonThread {
 public:
  /******************************************************************************
   *[Function]:  CommonThread()
   *[Description]:constructor function
   *******************************************************************************/
  explicit CommonThread(std::string name = "");

  /******************************************************************************
   *[Function]:  ~CommonThread()
   *[Description]:destructor function
   *******************************************************************************/
  ~CommonThread();

  /******************************************************************************
  *[Function]:  StartThread()
  *[Description]: start a thread
  *[Return]: 0: success
            other: fail
  *******************************************************************************/
  int StartThread();

  /******************************************************************************
  *[Function]:  StopThreadWait(int64_t        waitmicroseconds)
  *[Description]: try to stop thread waiting for micro seconds
                  this function only would be called by other thread
  *[Return]:  0: success
              other: fail
  *******************************************************************************/
  int StopThreadWait(int64_t waitmicroseconds);

  /******************************************************************************
  *[Function]:  IsRunning()
  *[Description]: to check thread is running
  *[Return]:  true: is in running
              false: in other state
  *******************************************************************************/
  bool IsRunning();

  /******************************************************************************
  *[virtual Function]:  RunCondition()
  *[Description]: subclass would to redefined it to check condition of running
  and return whether the thread can resume execution. in this function, it can
  call blocking function. this function will be called by every Run() function
  invocation cycle.
  *[Return]:  true: is in running
              false: in other state
  *******************************************************************************/
  virtual bool RunCondition();

  /******************************************************************************
  *[Function]:  Run()
  *[Description]: subclass must redefined it to implement thread execution
  function. this function is called in a loop unitl RunCondition() return false
  *[Return]:  0: running success
              other: running fail
  *******************************************************************************/
  virtual int Run() = 0;

  /******************************************************************************
  *[Function]:  Join()
  *[Description]: subclass must redefined it to implement thread execution
  function. this function is called in a loop unitl RunCondition() return false
  *[Return]:  0: running success
                other: running fail
  *******************************************************************************/
  int Join();

 protected:
  typedef enum CommonThreadState {
    idle,
    running,
    waitstop,
  } CommonThreadState;

  /******************************************************************************
   *[Struct]:  TheadPolicy
   *[Description]: Thread scheduling policy
   *******************************************************************************/
  typedef enum ThreadPolicy {
    OTHER,
    FIFO,
    RR /*Highest Policy*/
  } ThreadPolicy;

  pthread_t _thread;
  ThreadPolicy _policy;

  /******************************************************************************
  *[Function]:  setPriority()
  *[Description]: Set Thread Policy only would be called by subclass
  *[Return]:  0: running success
              other: running fail
  *******************************************************************************/
  int setPriority(ThreadPolicy priority);

  /******************************************************************************
  *[Function]:  StopThread
  *[Description]: Subclass try to stop thread, it can't commit to stop thread.
                  it only set flag.
  *[Return]:  0: running success to set flag to stop
                other: set stop flag fail
  *******************************************************************************/
  int StopThread();

 private:
  HobotMutex _thread_state_mutex;
  CommonThreadState _state;

 protected:
  int64_t _wait_for_millisecs;
  HobotCond _state_cond;
  std::string _thread_name;
  int SetThreadName(std::string thread_name);
  static void* Routine(void* self);
  CommonThreadState ThreadState();
};

/******************************************************************************
*[Class]:     FsMonitor
*[Description]: File System Monitor
******************************************************************************/
class FsMonitor : public sunrise::CommonThread {
 public:
  typedef enum FileEvent {
    ACCESS = IN_ACCESS,               /* File was accessed */
    MODIFY = IN_MODIFY,               /* File was modified */
    ATTRIB = IN_ATTRIB,               /* Metadata changed */
    CLOSE_WRITE = IN_CLOSE_WRITE,     /* Writtable file was closed */
    CLOSE_NOWRITE = IN_CLOSE_NOWRITE, /* Unwrittable file closed */
    OPEN = IN_OPEN,                   /* File was opened */
    MOVED_FROM = IN_MOVED_FROM,       /* File was moved from X */
    MOVED_TO = IN_MOVED_TO,           /* File was moved to Y */
    CREATE = IN_CREATE,               /* Subfile was created */
    DELETE = IN_DELETE,               /* Subfile was deleted */
    DELETE_SELF = IN_DELETE_SELF,     /* Self was deleted */
    UNMOUNT = IN_UNMOUNT,             /* Backing fs was unmounted */
    Q_OVERFLOW = IN_Q_OVERFLOW,       /* Event queued overflowed */
    IGNORED = IN_IGNORED,             /* File was ignored */
    CLOSE = IN_CLOSE,     /* close (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) */
    MOVE = IN_MOVE,       /* moves (IN_MOVED_FROM | IN_MOVED_TO)*/
    ISDIR = IN_ISDIR,     /* event occurred against dir */
    ONESHOT = IN_ONESHOT, /* only send event once */
                          /*
                           * All of the events - we build the list by hand so that we can add flags in
                           * the future and not break backward compatibility.  Apps will get only the
                           * events that they originally wanted.  Be sure to add new events here!
                           */
    ALL_EVENTS = IN_ALL_EVENTS
  } FileEvent;



  typedef struct inotify_data_type {
    int fd;
    char self_type[32];
} InotifyDataType;


  /******************************************************************************
  *[Class]:     FsMonitor::CallBackHandler
  *[Description]: File System Monitor
  ******************************************************************************/
  class CallBackHandler {
   public:
    /******************************************************************************
    *[Function]:  OnFileEvent
    *[Description]: when file event occur, this funtion will be called
    *[Return]:  0: success to process file event
            other: fail to process file event
    ******************************************************************************/
    virtual int OnFileEvent(std::string path, FileEvent event) = 0;
  };

  typedef struct watcher_node {
    int               watch_fd;
    std::string       path;
    FileEvent         event;
    bool              deep;
    CallBackHandler*  call_back;
  } WatcherNode;

  /******************************************************************************
  *[Function]:  FsMonitor
  *[Description]: construce of FsMonitor
  *[In]:  std::string name: name of FsMonitor, user would assign name for
  instance uint64_t interval_millisecnd(10~60000): wait for event milli second,
          default value is 500 milliseconds;
          if interval_millisecnd < 10 millisecnd, interval_millisecnd will be 10,
          if interval_millisecnd > 60000 millisecnd, interval_millisecnd will be 60000
  ******************************************************************************/
  explicit FsMonitor(std::string name = "", uint64_t interval_millisecnd = 500);

  /******************************************************************************
  *[Function]:  FsMonitor
  *[Description]: destructor of FsMonitor
  ******************************************************************************/
  ~FsMonitor();
#if 0
  /******************************************************************************
  *[Function]:  RegisterCallBack
  *[Description]: Register File Event Call back
  *[In]:   CallBackHandler* handler, call back handler
  *[Return]:  0: success to register event call back
             other: fail to register event call back
  ******************************************************************************/
  int RegisterCallBack(CallBackHandler* handler);

  /******************************************************************************
  *[Function]:  UnregisterCallBack
  *[Description]: Unregister File Event Call back
  *[In]:   CallBackHandler* handler, call back handler
  *[Return]:  0: success to unregister event call back
             other: fail to unregister event call back
  ******************************************************************************/
  int UnregisterCallBack(CallBackHandler* handler);
#endif
  /******************************************************************************
  *[Function]:  StartMonitorDir
  *[Description]: Start to monitor directory
  *[In]:   std::vector<std::string> monitor_path, monitor directory path;
           FileEvent event, file event,it would be overlay by bit;
           bool deep, if true, it will monitor sub directory
  *[Return]:  0: success to start monitoring target directory
             other: fail to start monitoring target directory
  ******************************************************************************/
  int StartMonitorDir(std::vector<std::string> monitor_path, FileEvent event,
                      CallBackHandler* handler,
                      bool deep = false);

  /******************************************************************************
  *[Function]:  StopMonitorDir
  *[Description]: Stop monitor directory
  *[In]:   std::vector<std::string> monitor_path, monitor directory path;
  *[Return]:  0: success to stop monitoring target directory
             other: fail to stop monitoring target directory
  ******************************************************************************/
  int StopMonitorDir(std::vector<std::string> monitor_path, bool deep = false);

  /******************************************************************************
  *[Function]:  StopAllMonitorDir
  *[Description]: Stop All monitor directory
  *[Return]:  0: success to stop monitoring target directory
             other: fail to stop monitoring target directory
  ******************************************************************************/
  int StopAllMonitorDir();

  /******************************************************************************
  *[Function]:  RunCondition
  *[Description]: override virtual function RunCondition
  *[Return]:  true: continue to run
              false: stop to run
  ******************************************************************************/
  bool RunCondition() override;

  /******************************************************************************
  *[Function]:  Run fuction 
  *[Description]: override virtual function thread run 
  *[Return]:  0: success to run
              false: fail to run
  ******************************************************************************/
  int Run() override;

 protected:
  /******************************************************************************
  *[Function]:  ScanDir
  *[Description]: Scan Directory 
  *[In]:  std::string dir_path, scan directory path
  *[Return]:  std::vector<std::string>, array of sub directory path
  ******************************************************************************/
  std::vector<std::string> ScanDir(std::string dir_path);

  /******************************************************************************
  *[Function]:  BroadcastFileEvent
  *[Description]: Broadcast File Event
  *[In]:  struct inotify_event* event, event if inotify event
  *[Return]:  0 bra
  ******************************************************************************/
  int BroadcastFileEvent(struct inotify_event* event);

  /******************************************************************************
  *[Function]:  StartMultiplex
  *[Description]: Start Multiplext monitor, monitor thread will monitor 
                  multi-directory
  *[Return]:  0: success to start multiplex monitor
             other: fail to start multiplex monitor
  ******************************************************************************/
  int StartMultiplex();

  /******************************************************************************
  *[Function]:  StopMultiplex
  *[Description]: Stop Multiplext monitor, monitor thread will stop monitor 
                  multi-directory
  *[Return]:  0: success to stop multiplex monitor
             other: fail to stop multiplex monitor
  ******************************************************************************/
  int StopMultiplex();

  /******************************************************************************
  *[Function]:  MonitorDir
  *[Description]: Monitor Directory 
  *[Return]:  0: Success to monitor directory
             other: fail to monitor directory
  ******************************************************************************/
  int MonitorDir(std::string dir_path, FileEvent event,
                  CallBackHandler* handler, bool deep);

  /******************************************************************************
  *[Function]:  UnMonitorDir
  *[Description]: Unmonitor Directory 
  *[Return]:  0: Success to unmonitor directory
             other: fail to unmonitor directory
  ******************************************************************************/
  int UnMonitorDir(std::string dir_path, bool deep);

 private:
  /*Gobal inotify fd*/
  int notify_fd_;
  /*Monitor Name */
  std::string name_;
  /*inotify minitor interval milliseconds*/
  uint64_t interval_millisecnd_;
  /*watch_dir mutex*/
  HobotRWMutex watch_dir_mutex_;
  /*directory under monitor*/
  std::map<std::string, WatcherNode> dir_watcher_;
  /*fd of monitor watching*/
  std::map<int, WatcherNode> wfd_dir;
  /*is under monitor*/
  bool is_monitor_;
  /*epoll fd*/
  int epoll_fd_;
  /*epoll data */
  InotifyDataType* inotify_data_;
};
}  // namespace sunrise
#endif  //  SRC_COMMON_HOBOT_UTILILTY_H_
