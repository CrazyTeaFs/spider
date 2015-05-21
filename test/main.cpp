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
#include <pthread.h>
#include <sys/time.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include "message.pb.h"

using namespace spider;
using namespace std;

#define VERIFY_DIGIT 10
#define THREADS 50

int const APLPHABET = 62;
char letter[62] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't'
, 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 
'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};

typedef struct {
	unsigned length;
	unsigned check_hash; 
	char hash[VERIFY_DIGIT];
} Header_t;

// BKDR Hash 
unsigned int BKDRHash(const string& str) {
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    for(size_t i = 0; i < str.length(); i++)
    {
        hash = (hash * seed) + str[i];
    }

    return hash;
}

static int int_rand() {
    return rand() % APLPHABET;
}

void *benchmark(void* args); 

// Generate A Random String
string generate_key(int digit) {
    srand(time(NULL));
    
    string pwd(digit, '0');
    for (int i = 0; i < digit; i++) {
        char flow_char = letter[int_rand()];
        pwd[i] = flow_char;
    }
    return pwd;
}


// 19:30:30 030 Hour Minute Second Millisecond 
string now_str() {
    char now_str[128];

    struct timeval now = {0};
    struct tm pnow;
    gettimeofday(&now, NULL);
    localtime_r(&now.tv_sec, &pnow);
	snprintf(now_str, sizeof(now_str) - 1, "%02d:%02d:%02d %06d", pnow.tm_hour, pnow.tm_min, pnow.tm_sec, now.tv_usec);

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
		printf("usage: %s ip port\n", argv[0]);
		exit(EXIT_FAILURE);
	}

    signal(SIGPIPE, SIG_IGN);

	pthread_t thread[THREADS];
	
	for (int t = 0; t < THREADS; t++) {
		int rc = pthread_create(&thread[t], NULL, benchmark, &t);
		if (rc) {
			printf("ERROR; return code %d\n", rc);
			exit(EXIT_FAILURE);
		}
	}

	for (int t = 0; t < THREADS; t++) {
		pthread_join(thread[t], NULL);
	}

	printf("%s Finish Ping-Pong Alive BenchMark\n", now_str().c_str());

	while (1) {
		sleep(300);
	}

	return 0;
}

void *benchmark(void* args) {
	const char *ip = "192.168.144.121";
	int port = 8888;
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
	for (int i = 0; i < 10; i++) {
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
		int length = size + sizeof(Header_t);
		printf("Message Size: %d Header Size:%d \n", length, length - size);
		void *buffer = malloc(length);
		Header_t h;
		h.length = htonl(length);
		strncpy(h.hash, generate_key(VERIFY_DIGIT).c_str(), VERIFY_DIGIT);
		h.check_hash = htonl(BKDRHash(string(h.hash)));
		memcpy(buffer, &h, sizeof(Header_t));
		message->SerializeToArray((char *)buffer + sizeof(Header_t), size);

		printf("Message: %s\n", message->DebugString().c_str());	
		printf("Send %lu Bytes\n", send(sockfd, buffer, length, 0));
		free(buffer);
		delete message;

		SMessage *recieve = new SMessage;
		buffer = malloc(1024 * sizeof(char));

		printf("Recv %lu Bytes\n", recv(sockfd, buffer, 1024, 0));
		recieve->ParseFromArray((char *)buffer + sizeof(Header_t), 1024);
		printf("Message: %s\n", recieve->DebugString().c_str());	

		delete recieve;
		free(buffer);
	}

	return NULL;
}
