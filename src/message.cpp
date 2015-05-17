#include "message.h"
#include "sock.h"

using namespace std;

//const char *message_check_key = "0c675432-fbb0-11e4-acf8-60eb69eac53d";

// Note: Don't Forget Delete Pointer.
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

// Copy Incoming Request Header To MakeResponse, Except Message Type
Header CopyRequestHeader(const Header &header) {
	Header result;
	result.set_flow_no(header.flow_no());
	result.set_src_fsm(header.src_fsm());
	result.set_dst_fsm(header.dst_fsm());
	result.set_src_state(header.src_state());
	result.set_dst_state(header.dst_state());
	result.set_type((MessageType)(header.type() + 1));

	return result;
}

// Use Header_t check_hash and hash Field to Check Whether Incoming Message Is Our Private Protocol Encoded Message
bool ValidMessage(void *buffer, int len) {		
	if (len < (int) sizeof(Header_t)) {
		return false;
	}

	Header_t *h = (Header_t *)buffer;
	unsigned check_value = htonl(h->check_hash);

	if (check_value != BKDRHash(string(h->hash))) {
		return false;
	}
	
	return true;	
}
