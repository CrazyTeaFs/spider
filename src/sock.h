#ifndef _SOCK_H_
#define _SOCK_H_

#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string>
#include <string.h>

#include "log.h"

// 20KB InBuffer And OutBuffer
#define SOCKET_BUFFER_SIZE (20*1024)
#define INVALID_MESSAGE -1

// 15 Minutes
#define MAX_IDLE_TIME (900)

typedef enum {
	SOCK_IDLE,
	SOCK_LISTENNING,
	SOCK_CONNECTTING,
	SOCK_TCP_ENSTABLISHED,	
	SOCK_CLOSED,
	SOCK_TIMEOUT, // Unused Now
	SOCK_SHUT_ALL,
	SOCK_SHUT_READ,
	SOCK_SHUT_WRITE,
} Sockstate_t;

typedef enum {
	TYPE_TCP,
	TYPE_UDP,
	TYPE_RAW,
} Socktype_t;

// Only For TCP Stream And Only For Non-Blocking Operations
class Socket
{
 public:
	Socket(int fd); 
	
	~Socket();

	void Close();

	int GetFd() {
		return sockfd_;
	}

	Sockstate_t State() {
		return state_;
	}

	int BindListen(int port);

	// Only Support Edge Trigger In Epoll Model
	int Read();

	// Only Support Edge Trigger In Epoll Model
	int Write();
	
	// Only For Non-Blocking Connect With A Timout 10 Second As Default
	int Connect(const std::string &ip, int port, int timeout = 10);

	static int Accept(int listen_fd);

	int SetTcpInBuffsize(size_t size);

	int SetTcpOutBuffsize(size_t size);

	int GetTcpInBuffsize();

	int GetTcpOutBuffsize();

	void ShutdownAll();

	void ClearRBuffer() {
		memset(inbuf_, 0, SOCKET_BUFFER_SIZE * sizeof(char));
		r_offset_ = 0;
	}

	void ClearWBuffer() {
		memset(outbuf_, 0, SOCKET_BUFFER_SIZE * sizeof(char));
		w_offset_ = 0;
		append_offset_ = 0;
	}

	char *GetWriteIndex() {
		return outbuf_ + append_offset_;
	}

	// Add Data To Output Buffer, Wait To Be Sent
	void AppendSend(size_t len) {
		append_offset_ += len;
	}

	// Will Send All Remaining Data In TCP Sending Buffer And Then Send A TCP_FIN
	void ShutdownW();

	void ShutdownR();

	sockaddr_in GetPeerAddr() {
		return peer_;
	}

	void UpdateTimeStamp() {
		last_io_time_ = time(NULL);
	}

	int GetLastTimeStamp() {
		return last_io_time_;
	}

	bool IsConnected() {
		return (state_ == SOCK_TCP_ENSTABLISHED);
	}	

	static int IdleCtrlCb (void *);
	
private:
	// Non-Blocking I/O
	void SetNonBlocking();

	// Turn Off Nagle's Algorithm And Delayed ACK
	void SetNoTcpDelay();

	// Address Reuse  
	void SetResueAddr();

	// Enable TCP Keep-Alive
	void SetKeepAlive();

	// Cannot Be Invoked By User
	void SetState(Sockstate_t state) {
		state_ = state;
	}

	void SetPeerAddr(sockaddr_in address) {
		peer_ = address;
	}

public:	
	static std::map<sockaddr_in, Socket *> conn_ctrl_;

	static std::map<sockaddr_in, Socket *> client_ctrl_;

private:
	int sockfd_;
	sockaddr_in peer_;
	Sockstate_t state_;
	
	size_t r_offset_;
	size_t w_offset_;
	size_t append_offset_;
	char *inbuf_;
	char *outbuf_;
	int recv_buff_size_;
	int send_buff_size_;
	Socktype_t type_;

	int last_io_time_;
};

static inline bool operator==(const sockaddr_in &foo, const sockaddr_in &bar) {
	return ((foo.sin_addr.s_addr == bar.sin_addr.s_addr) && (foo.sin_port == bar.sin_port));
}

static inline bool operator>(const sockaddr_in &foo, const sockaddr_in &bar) {
	if (foo.sin_addr.s_addr != bar.sin_addr.s_addr) {
		return (foo.sin_addr.s_addr > bar.sin_addr.s_addr);
	} else {
		return (foo.sin_port > bar.sin_port);
	}
}

static inline bool operator<(const sockaddr_in &foo, const sockaddr_in &bar) {
	if (foo.sin_addr.s_addr != bar.sin_addr.s_addr) {
		return (foo.sin_addr.s_addr < bar.sin_addr.s_addr);
	} else {
		return (foo.sin_port < bar.sin_port);
	}
}

static inline bool operator>=(const sockaddr_in &foo, const sockaddr_in &bar) {
	if (foo.sin_addr.s_addr != bar.sin_addr.s_addr) {
		return (foo.sin_addr.s_addr >= bar.sin_addr.s_addr);
	} else {
		return (foo.sin_port >= bar.sin_port);
	}
}

static inline bool operator<=(const sockaddr_in &foo, const sockaddr_in &bar) {
	if (foo.sin_addr.s_addr != bar.sin_addr.s_addr) {
		return (foo.sin_addr.s_addr <= bar.sin_addr.s_addr);
	} else {
		return (foo.sin_port <= bar.sin_port);
	}
}

#endif 
