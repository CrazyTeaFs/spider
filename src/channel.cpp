#include <arpa/inet.h>

#include "channel.h"

using namespace std;

extern map<sockaddr_in, Socket *> gmap_tcpdest;
extern char *message_check_key;

int CheckConnectTimeoutCb(void *data) {
	Socket *sk = (Socket *)data;
	if (sk->State() != SOCK_TCP_ENSTABLISHED) {
		ALERT("Connection To %s:%d Time Out", iptostr(sk->GetPeerAddr().sin_addr.s_addr), sk->GetPeerAddr().sin_port);
		EventDriver::Instance()->DelEvent(sk->GetFd());
		delete sk;
		sk = NULL;
		return -1;
	}
	return 0;
}

// Connect With A Timeout
int Channel::OnConnect(const string &ip,  int port, int timeout) {
	int ret = sk_->Connect(ip, port, timeout);
	if (ret == 0) {
		INFO("Connection Complete Immediately");
		return 0;
	} else if (ret == SOCK_CONNECTTING) {
		// After Timeout, Check If The Socket Is Writable
		EventDriver::Instance()->AddTimer(timeout, 0, true, CheckConnectTimeoutCb, sk_);
		return SOCK_CONNECTTING; 
	} else {
		return -1;
	}
}
	
int Channel::SendRequest(const string &ip, int port, void *message, size_t len) {
	int ret = 0;
	struct sockaddr_in address;	
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;

	inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
	address.sin_port = htons(port);

	// Check If We Have Already Have A TCP Connection To Peer
	map<sockaddr_in, Socket *>::iterator it = gmap_tcpdest.find(address);
	if (it != gmap_tcpdest.end()) {
		sk_ = it->second; 	
		SendMessage(message, len);
	} else {
		OnConnect(ip, port);
		// When Writable, SendMessage, If Failed To Connect, Message Will Be Lost  
		SendMessage(message, len);
	}

	return ret;
}

int Channel::SendResponse(SMessage *msg) {
	int size = msg->ByteSize();
	int length = size + sizeof(Header_t);
	DEBUG("Total Length To Send: %d Bytes, Header %d Bytes, Content: %s", length, sizeof(Header_t), msg->DebugString().c_str());

	Header_t *h = (Header_t *)(sk_->GetWriteIndex());
	h->length = ntohl(length);
	strncpy(h->hash, generate_key(VERIFY_DIGIT).c_str(), VERIFY_DIGIT);
	h->check_hash = ntohl(BKDRHash(string(h->hash)));

	msg->SerializeToArray(sk_->GetWriteIndex() + sizeof(Header_t), msg->ByteSize());
	sk_->AppendSend(length);
	
	return 0;
}

// Not Doing Real Socket I/O Actions, Merely Append The Socket Object's output_ buff
int Channel::SendMessage(void *message, size_t len) {
	memcpy(sk_->GetWriteIndex(), message, len);	
	return 0;
}
