#ifndef _CHANNEL_
#define _CHANNEL_

#include <sys/time.h>
#include <string>
#include <string.h>

#include "event_driver.h"
#include "server.h"
#include "sock.h"
#include "message.h"

// Channel Not Actually Own The Socket
class Channel {
public:
	Channel():sk_(NULL) {

	}

	Channel(Socket *sk):sk_(sk) {

	}

	~Channel() {
	}
	
	// As A Client Launch A Connection
	int OnConnect(Socket* sk, const std::string &ip, int port, int timeout = 10);

	int SendRequest(const std::string &ip, int port, void *message, size_t len);

	int SendResponse(SMessage *msg);

	int SendMessage(Socket* sk, void *message, size_t len);
	
	void SetSocket(Socket *sk) {
		sk_ = sk;
	}

	Socket* GetSocket() {
		return sk_;
	}

	Socket* GetClientSocket(const std::string &ip, int port); 

	Socket* NewClientSocket(const std::string &ip, int port); 

private:
	Socket* sk_;
};

#endif
