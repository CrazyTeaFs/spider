#include "fsm_factory.h"
#include "heart_beat.h"

using namespace spider;

static const int request_type = HEART_BEAT_REQUEST;

REFLECT_CREATE(HeartBeat, request_type);
STATE_CALLBACK(HEART_BEAT_REQUEST, INIT_STATE, HeartBeat, AliveResponse);

HeartBeat::HeartBeat() {
}

int HeartBeat::FsmType() {
	return request_type;
}

Status_t HeartBeat::AliveResponse(void *smessage) {
	SMessage *pmsg = (SMessage *)smessage;
	INFO("I Am OKay");
	
	Header header;
	Body body;
	header = CopyHeader(pmsg->header());

	HeartBeatResponse *res = body.MutableExtension(heart_beat_response); 

	res->mutable_rc()->set_retcode(0);
	res->mutable_rc()->set_error_msg("I Am OK");

	SMessage response_;	
	response_.set_allocated_header(&header);	
	response_.set_allocated_body(&body);	
	
	SendResponse(&response_); 
	return FSM_FINISH;
}

HeartBeat::~HeartBeat() {

}
