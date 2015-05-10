#include "fsm_factory.h"
#include "heart_beat.h"

using namespace spider;

static const int request_type = HEART_BEAT_REQUEST;

REFLECT_CREATE(HeartBeat, request_type);

HeartBeat::HeartBeat() {
}

int HeartBeat::FsmType() {
	return request_type;
}


Status_t HeartBeat::AliveResponse(void *smessage) {
	INFO("I am Ok");

	return FSM_FINISH;
}

HeartBeat::~HeartBeat() {

}
