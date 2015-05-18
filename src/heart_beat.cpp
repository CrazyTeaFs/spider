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
	
	Header *header = response_.mutable_header();
	Body *body = response_.mutable_body();
	*header = CopyRequestHeader(pmsg->header());

	HeartBeatResponse *res = body->MutableExtension(heart_beat_response); 

	res->mutable_rc()->set_retcode(0);
	res->mutable_rc()->set_error_msg("I Am OK");

	SendResponse(&response_); 
	return FSM_FINISH;
}

HeartBeat::~HeartBeat() {

}
