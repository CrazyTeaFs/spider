#ifndef _THREAD_JOB_H_
#define _THREAD_JOB_H_

#include <unistd.h>
#include <sys/eventfd.h>

#include "event_driver.h"

// Callback May Block
typedef int (*job_cb_t) (void *);

class TJob {
public:
	TJob();

	~TJob();

	void SetCallback(job_cb_t callback)	 {
		callback_ = callback;
	}

	int Process(void *args);

private:
	bool running_;
	int event_fd_;
	job_cb_t callback_;
};

#endif
