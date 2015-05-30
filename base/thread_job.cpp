#include "thread_job.h"

using namespace std;
	
TJob::TJob():event_fd_(-1) {
	// No Matter Block
	event_fd_ = eventfd(0, 0);
	if (event_fd_ < 0) {
		CRIT("Cannot Open Eventfd");
		exit(EXIT_FAILURE);
	}
	// Default Edge Triggle
	EventDriver::Instance()->AddEventFd(event_fd_);
}

TJob::~TJob() {
	EventDriver::Instance()->DelEventFd(event_fd_);
	close(event_fd_);
}
