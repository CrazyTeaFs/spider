#ifndef _THREAD_JOB_H_
#define _THREAD_JOB_H_

#include <unistd.h>
#include <sys/eventfd.h>

#include "event_driver.h"

class TJob {
public:
	TJob();

	~TJob();

	void Process(void) {
	}

private:
	int event_fd_;
};

#endif
