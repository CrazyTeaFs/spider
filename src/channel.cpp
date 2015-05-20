#include <arpa/inet.h>

#include "channel.h"

using namespace std;

int CheckConnectTimeoutCb(void *data) {
	Socket *sk = (Socket *)data;
	if (sk->State() != SOCK_TCP_ENSTABLISHED) {
		ALERT("Connection To %s:%d Time Out", iptostr(sk->GetPeerAddr().sin_addr.s_addr), sk->GetPeerAddr().sin_port);
		EventDriver::Instance()->DelEvent(sk->GetFd());
		delete sk;
		sk = NULL;
		return -1;
	} else {
		INFO("Connection To %s:%d Enstablished", iptostr(sk->GetPeerAddr().sin_addr.s_addr), sk->GetPeerAddr().sin_port);
	}
	return 0;
}

Socket* Channel::GetClientSocket(const string &ip, int port) {
	struct sockaddr_in address;	
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;

	inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
	address.sin_port = htons(port);

	map<sockaddr_in, Socket *>::iterator it = Socket::client_ctrl_.find(address);
	if (it != Socket::client_ctrl_.end()) {
		return it->second;
	} else {
		return NewClientSocket(ip, port);
	}
} 

Socket* Channel::NewClientSocket(const std::string &ip, int port) {
	int ret = 0;
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd <= 0) {
		ALERT("Create Socket Failure Errno: %d, ErrStr: %s\n", errno, strerror(errno));
		return NULL;
	}

	Socket *sk = new Socket(sockfd);
	struct sockaddr_in address;	
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;

	inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
	address.sin_port = htons(port);

	// Set 10 Seconds Timeout, Add The Socket To Event Poller, OnConnect Will SetPeerAddr
	ret = OnConnect(sk, ip, port, 10);
	if (ret > 0) {
		Socket::client_ctrl_.insert(make_pair(address, sk));	
		return sk;
	} else {
		return NULL;
	}
}

// Connect With A Timeout
int Channel::OnConnect(Socket* sk, const string &ip, int port, int timeout) {
	int ret = sk->Connect(ip, port, timeout);
	if (ret == 0) {
		INFO("Connection Complete Immediately");
		return 0;
	} else if (ret == SOCK_CONNECTTING) {
		// After Timeout, Check If The Socket Is Writable
		EventDriver::Instance()->AddTimer(timeout, 0, true, CheckConnectTimeoutCb, (void*)sk);
		return SOCK_CONNECTTING; 
	} else {
		return -1;
	}
}
	
int Channel::SendRequest(const string &ip, int port, void *message, size_t len) {
	Socket *sk = GetClientSocket(ip, port);
	if (sk == NULL) {
		ERROR("Unable To Launch A Request Connection To Peer Server->%s:%d", ip.c_str(), port);
		return -1;
	}

	// Write Data To The Socket's Writable Buffer, When Connection Is Enstalished Within Timeout, Data Will Be Sent
	// If Connection Failed To Enstablish, Data WIll Be Lost
	return SendMessage(sk, message, len);
}

int Channel::SendResponse(SMessage *msg) {
	int size = msg->ByteSize();
	int length = size + sizeof(Header_t);
	DEBUG("Destination %s:%d, Total Length To Send: %d Bytes, Header %d Bytes, Content:\n%s", iptostr(sk_->GetPeerAddr().sin_addr.s_addr), 
	ntohs(sk_->GetPeerAddr().sin_port), length, sizeof(Header_t), msg->DebugString().c_str());

	Header_t *h = (Header_t *)(sk_->GetWriteIndex());
	h->length = htonl(length);
	strncpy(h->hash, generate_key(VERIFY_DIGIT).c_str(), VERIFY_DIGIT);
	h->check_hash = htonl(BKDRHash(string(h->hash)));

	msg->SerializeToArray(sk_->GetWriteIndex() + sizeof(Header_t), msg->ByteSize());
	sk_->AppendSend(length);
	
	return 0;
}

// Not Doing Real Socket I/O Actions, Merely Append The Socket Object's output_ buff
int Channel::SendMessage(Socket* sk, void *message, size_t len) {
	memcpy(sk->GetWriteIndex(), message, len);	
	return 0;
}
