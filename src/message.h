#ifndef _MESSAGE_
#define _MESSAGE_

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "message.pb.h"

using namespace spider;

const char *message_check_key = "0c675432-fbb0-11e4-acf8-60eb69eac53d";

typedef struct {
	unsigned length;
	unsigned check_hash; 
} Header_t;

google::protobuf::Message* CreateMessage(const std::string &name);

Header CopyRequestHeader(const Header &);

// Use Header_t check_ip Field to Check Whether Incoming Message Is Our Private Protocol Encoded Message
bool ValidMessage(void *buffer, int len);

#endif
