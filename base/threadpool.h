#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"
#include "log.h"

template< typename T >
class ThreadPool
{
public:
    ThreadPool( int thread_number = 8, int max_requests = 10000 );
    ~ThreadPool();
    bool Append( T* request );

private:
    static void* Worker( void* arg );
    void Run();

private:
    int thread_number_;
    int max_requests_;
    pthread_t* threads_;
    std::list< T* > workqueue_;
    locker queuelocker_;
    bool stop_;
};
#endif
