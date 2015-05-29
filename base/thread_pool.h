#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <list>
#include <pthread.h>
#include "locker.h"
#include "thread_job.h"
#include "log.h"

class ThreadPool {
public:
    ThreadPool(int thread_number = 8, unsigned max_requests = 10000);
    ~ThreadPool();
    bool Append(TJob *);

private:
    static void* Worker( void* arg );
    void Run();

private:
    int thread_number_;
    unsigned max_requests_;
    pthread_t* threads_;
    std::list<TJob*> workqueue_;
    Locker queuelocker_;
    bool stop_;
};

#endif
