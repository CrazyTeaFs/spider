#ifndef _MESSAGE_
#define _MESSAGE_

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "message.pb.h"
#include "str_util.h"
#include "crc32.h"

#define VERIFY_DIGIT 10

using namespace spider;

typedef struct {
	unsigned length;
	unsigned data_crc32; 
} Header_t;

google::protobuf::Message* CreateMessage(const std::string &name);

Header CopyRequestHeader(const Header &);

// Use Header_t check_ip Field to Check Whether Incoming Message Is Our Private Protocol Encoded Message
bool ValidMessage(char *buffer, int len);

#endif
