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
	header = pmsg->header();
	header.set_type(HEART_BEAT_RESPONSE);
	response_.set_allocated_header(&header);	
	HeartBeatResponse *res = response_.body().MutableExtension(heart_beat_response);
	res.mutable_rc()->set_retcode(0);
	res.mutable_rc()->set_error_msg("I Am OK");
	
	SendResponse(&response_);
	return FSM_FINISH;
}

HeartBeat::~HeartBeat() {

}
