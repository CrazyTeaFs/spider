#ifndef _LOCKER_
#define _LOCKER_
#include <pthread.h>

class Locker {
public:
    Locker() {
		Init();
    }

	int Init() {
		return pthread_mutex_init(&mutex_, NULL);
	}

    ~Locker() {
        pthread_mutex_destroy(&mutex_);
    }

   	int Lock() {
        return pthread_mutex_lock(&mutex_);
    }

    bool Unlock() {
        return pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t mutex_;
};

#endif
