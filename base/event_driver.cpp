#include <sys/types.h>
#include <sys/socket.h>

#include "event_driver.h"

using namespace std;

static int create_timerfd() {
	int timerfd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
	if (timerfd < 0) {
		ERROR("Failed In timerfd_create");
  	}
  	return timerfd;
}

EventDriver* EventDriver::p_event_driver_ = NULL;

EventDriver::~EventDriver() {
	close(epfd_);
}

// Singleton, Not Multi-Thread Safe
EventDriver* EventDriver::Instance() {
	if (p_event_driver_ == NULL) {
		p_event_driver_ = new EventDriver();
	}
	return p_event_driver_;
}

void EventDriver::CreateDriver() {
	// Create Kernel Event Table
	epfd_ = epoll_create(5);
}

// Default Edge-Trigger
void EventDriver::AddEvent(int fd, Socket *sk, Trigger_t type) {
	epoll_event event;
	event.data.fd = fd;
	if (type == LEVEL_TRIGGER) {
		event.events = EPOLLIN | EPOLLRDHUP; 
	} else {
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; 
	}
	epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
	event_container_.insert(make_pair(fd, sk));	
}

// Support Linux SYS-API eventfd, Used By Thread Job Invoke Callback
void EventDriver::AddEventFd(int event_fd, Trigger_t type) {
	epoll_event event;
	event.data.fd = event_fd;
	if (type == LEVEL_TRIGGER) {
		event.events = EPOLLIN | EPOLLRDHUP; 
	} else {
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; 
	}
	epoll_ctl(epfd_, EPOLL_CTL_ADD, event_fd, &event);
}

// Under Edge-Trigger  
void EventDriver::ModifyEvent(int fd, int active_type) {
	epoll_event event;
	event.data.fd = fd;
	if (active_type == EPOLLIN) {
		event.events = EPOLLIN | EPOLLET | EPOLLRDHUP; 
	} else if (active_type == EPOLLOUT) {
		event.events = EPOLLIN | EPOLLET | EPOLLOUT | EPOLLRDHUP; 
	}
	epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &event);
}

// The Only Way To Delete A Socket
void EventDriver::DelEvent(int fd) {
	epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
	delete event_container_[fd];
	event_container_.erase(fd);
}

// Support Linux SYS-API eventfd, Used By Thread Job Invoke Callback
void EventDriver::DelEventFd(int fd) {
	epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, NULL);
}

// Only This Function Can Create Timer, User Must Check Return Value
int EventDriver::AddTimer(int sec, int msec, bool once_only, int (*callback) (void *), void *args) {
	int ret = 0;
	int fd = create_timerfd();
	if (fd < 0) {
		ALERT("Failed To Create Timer Fd");
		return fd;
	}

	Timer* ptimer = new Timer(fd, once_only);

	//Trigger The Timer And Set CallBack
	ptimer->SetInterval(sec, msec);
	ptimer->SetCallback(callback);
	ptimer->SetArgs(args);

	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET; 

	ret = epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &event);
	
	if (ret != 0) {
		ALERT("Fail To Add Epoll Timer\n");
		return ret;
	}

	timer_container_.insert(make_pair(ptimer->GetFd(), ptimer));
	return 0;
} 

void EventDriver::DelTimer(Timer *timer) {
	epoll_ctl(epfd_, EPOLL_CTL_DEL, timer->GetFd(), NULL);
	timer_container_.erase(timer->GetFd());
	timer->Disarm();
	delete timer;
}

void EventDriver::Tick(int fd, void *args) {
	map <int, Timer *>::iterator it = timer_container_.find(fd);

	// If Not Timerfd
	if (it == timer_container_.end()) {
		return;
	}

	// Edge-Trigger, Drain All Data
	uint64_t times;
	while(true) {
		int ret = read(fd, &times, sizeof(times));
		if (ret < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK) {
				break;
			} else if (errno == EINTR) {
				continue;
			} else {
				DelTimer(it->second);
				return;
			}
		}
	}

	it->second->ActivateCb(args);
	if (it->second->OnceOnly()) {
		DelTimer(it->second);
	}

	return;
}

void EventDriver::StartLoop(int timeout_usec) {
	epoll_event events[MAX_EVENT_NO];

	while (true) {
		int number = epoll_wait(epfd_, events, MAX_EVENT_NO, timeout_usec); 
		if (number < 0 ) {
			if (errno == EINTR) {
				continue;
			}
			ERROR("Errno: %d, ErrStr: %s", errno, strerror(errno));
			break;	
		}

		map<int, Socket *>::iterator it;

		for (int i = 0; i < number; i++) {
			int event_fd = events[i].data.fd; // event_fd May Both Be socket OR timerfd
		
			// Connection Closed By Peer
			if (events[i].events & EPOLLRDHUP) {
				if ((it = event_container_.find(event_fd)) != event_container_.end()) {
					it->second->Close();
					DelEvent(event_fd);
				}
			}
			// Recieve New Data
			if (events[i].events & EPOLLIN) {
				// Accept A New Connection, Level-Trigger
				if ((it = event_container_.find(event_fd)) != event_container_.end()) {
					if (it->second->State() == SOCK_LISTENNING) {
						// Note That New Socket Object Will Be Created
						int ret = Socket::Accept(event_fd);
						if (ret < 0) {
							ERROR("Errno: %d, ErrStr: %s", errno, strerror(errno));
						}
						continue;
					}
				}
				// CheckTimer
				Tick(event_fd, NULL);

				// Drain All Data From TCP Recv Buffer, If TCP_FIN Recieved, Remove The Event
				if ((it = event_container_.find(event_fd)) != event_container_.end()) {
					it->second->Read();
					// Remove Closed Socket
					if (it->second->State() == SOCK_CLOSED) {
						DelEvent(event_fd);
					} else {
						ModifyEvent(event_fd, EPOLLOUT);
					}
				}
			} 

			if (events[i].events & EPOLLOUT) { // Ready To Send Data
				// Send All Data
				if ((it = event_container_.find(event_fd)) != event_container_.end()) {
					it->second->Write();
					// Remove Closed Socket
					if (it->second->State() == SOCK_CLOSED) {
						DelEvent(event_fd);
					} else {
						ModifyEvent(event_fd, EPOLLIN);
					}
				}

			}	
		}
	}
	return;
}
