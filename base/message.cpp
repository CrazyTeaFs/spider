#include "message.h"
#include "sock.h"

using namespace std;

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
bool ValidMessage(char *buffer, int len) {		
	if (len < (int) sizeof(Header_t)) {
		ERROR("Insufficient Length, Less Then Header");
		return false;
	}

	Header_t *h = (Header_t *)buffer;
	unsigned crc32 = ntohl(h->data_crc32);
	CCrc32 exam;
	unsigned result = exam.Crc32((unsigned char *)(buffer + sizeof(Header_t)), (uint32_t)(len - sizeof(Header_t)));
	if (crc32 != result) {
		ERROR("Hash Check Failed, Value: %u, CheckValue %u", crc32, result);
		return false;
	}
	
	return true;	
}
