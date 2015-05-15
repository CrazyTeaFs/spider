#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "message.pb.h"

using namespace spider;
using namespace std;

typedef struct {
	unsigned length;
	unsigned message_id; 
} header;

// 19:30:30 030 Hour Minute Second Millisecond 
string now_str() {
    char now_str[128];

    struct timeval now = {0};
    struct tm pnow;
    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &pnow);
	snprintf(now_str, sizeof(now_str) - 1, "%02d:%02d:%02d %06ld", pnow.tm_hour, pnow.tm_min, pnow.tm_sec, now.tv_usec);

    return string(now_str);
}

google::protobuf::Message* CreateMessage(const std::string &name) {
	google::protobuf::Message* message = NULL;
	const google::protobuf::Descriptor* descriptor =
		google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(name);
	if (descriptor) {
		const google::protobuf::Message* prototype =
			google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
		if (prototype) {
			message = prototype->New();
		}
	}

	return message;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("usage: %s ip port\n", basename(argv[0]));
		exit(EXIT_FAILURE);
	}

    signal(SIGPIPE, SIG_IGN);

	const char *ip = argv[1];
	int port = atoi(argv[2]);

	struct sockaddr_in address;
	bzero(&address, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd <= 0) {
		printf("Create Socket Failure Errno: %d, ErrStr: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	if (connect(sockfd, (struct sockaddr*) &address, sizeof(address)) < 0) {
		printf("Connect Failure Errno: %d, ErrStr: %s\n", errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	printf("%s Now Start Ping-Pong Alive BenchMark\n", now_str().c_str());
	for (int i = 0; i < 100000; i++) {
		SMessage *message = new SMessage;
		Header *head = new Header;
		head->set_flow_no(1234);
		head->set_src_fsm(0);
		head->set_dst_fsm(0);
		head->set_src_state(0);
		head->set_dst_state(0);
		head->set_type(HEART_BEAT_REQUEST);
		message->set_allocated_header(head);

		int size = message->ByteSize();
		int length = size + sizeof(header);
		//	printf("Message Size: %d Header Size:%d \n", length, length - size);
		void *buffer = malloc(length);
		header h;
		h.length = htonl(length);
		h.message_id = htonl(LOGIN_REQUEST);
		memcpy(buffer, &h, sizeof(h));
		message->SerializeToArray((char *)buffer + sizeof(header), size);

		//	printf("Message: %s\n", message->DebugString().c_str());	
		//	printf("Send %lu Bytes\n", send(sockfd, buffer, length, 0));
		send(sockfd, buffer, length, 0);
		free(buffer);
		delete message;

		SMessage *recieve = new SMessage;
		buffer = malloc(1024 * sizeof(char));

		//	printf("Recv %lu Bytes\n", recv(sockfd, buffer, 1024, 0));
		recv(sockfd, buffer, 1024, 0);
		recieve->ParseFromArray((char *)buffer + sizeof(header), 1024);
		//	printf("Message: %s\n", recieve->DebugString().c_str());	

		delete recieve;
		free(buffer);
	}

	printf("%s Finish Ping-Pong Alive BenchMark\n", now_str().c_str());
	return 0;
}
