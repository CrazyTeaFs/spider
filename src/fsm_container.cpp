#include "fsm_container.h"

using namespace std;

FsmContainer* FsmContainer::p_container_ = NULL;

FsmContainer::~FsmContainer() {
}

// Singleton, Not Multi-Thread Safe
FsmContainer* FsmContainer::Instance() {
	if (p_container_ == NULL) {
		p_container_ = new FsmContainer();
	}
	return p_container_;
}

int FsmContainer::AddStateMachine(int machine_id, Fsm *pfsm) {
	fsm_container_.insert(make_pair(machine_id, pfsm));
	return 0;
}

int FsmContainer::ActivateCb(SMessage *pmessage, int message_type, int dst_state) {

	return 0;
}

int FsmContainer::DelStateMachine(int message_type) {
	fsm_container_.erase(message_type);
	return 0;
}
