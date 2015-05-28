#ifndef _EVENT_DRIVER_
#define _EVENT_DRIVER_

#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <map>

#include "sock.h"
#include "timer.h"

#define MAX_EVENT_NO 1024

typedef enum {
	LEVEL_TRIGGER = 10,
	EDGE_TRIGGER,
} Trigger_t;

class EventDriver {
public:
	~EventDriver();
	
	static EventDriver* Instance();

	void CreateDriver();

	void AddEvent(int fd, Socket *sk, Trigger_t type = EDGE_TRIGGER);

	void ModifyEvent(int fd, int active_type);

	void DelEvent(int fd);

	int AddTimer(int sec, int msec, bool once_only, int (*callback) (void *), void *args);

	void DelTimer(Timer *timer);

	void StartLoop(int timeout_usec = 1000);

private:
	// Private Constructor For Singletion
	EventDriver():epfd_(0) {};

	void Tick(int fd, void *args);

private:
	int epfd_; 

	std::map <int, Socket *> event_container_;

	std::map <int, Timer *> timer_container_;

private:
	static EventDriver* p_event_driver_;
};

#endif
