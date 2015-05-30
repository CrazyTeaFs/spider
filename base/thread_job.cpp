#include "thread_job.h"

using namespace std;
	
TJob::TJob():event_fd_(-1) {
	event_fd_ = eventfd(O_NONBLOCK, 0);
	// Default Edge Triggle
	EventDriver::Instance()->AddEventFd(event_fd_);
}

TJob::~TJob() {
	EventDriver::Instance()->DelEventFd(event_fd_);
	close(event_fd_);
}
