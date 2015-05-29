#include "thread_pool.h"

using namespace std;

ThreadPool::ThreadPool(int thread_number, unsigned max_requests): 
        thread_number_(thread_number), max_requests_(max_requests), threads_(NULL), stop_(false)
{
    threads_ = new pthread_t[thread_number];

    for (int i = 0; i < thread_number_; ++i) {
        if (pthread_create(threads_ + i, NULL, Worker, this) != 0) {
            delete []threads_;
        }

        if (pthread_detach(threads_[i])) {
            delete []threads_;
        }
    }
}

ThreadPool::~ThreadPool() {
    delete [] threads_;
    stop_ = true;
}

bool ThreadPool::Append(TJob *request) {
    queuelocker_.Lock();
    if (workqueue_.size() > max_requests_) {
        queuelocker_.Unlock();
        return false;
    }
    workqueue_.push_back(request);
    queuelocker_.Unlock();
    return true;
}

// Static
void* ThreadPool::Worker( void* arg ) {
    ThreadPool* pool = (ThreadPool*)arg;
    pool->Run();
    return pool;
}

void ThreadPool::Run() {
    while (!stop_) {
        queuelocker_.Lock();
        if (workqueue_.empty()) {
            queuelocker_.Unlock();
            continue;
        }

        TJob* request = workqueue_.front();
        workqueue_.pop_front();
       	queuelocker_.Unlock();
        if (!request) {
            continue;
        }
        request->Process();
    }
}
