#include "message.h"

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
