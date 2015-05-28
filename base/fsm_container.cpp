#include "fsm_container.h"
#include "fsm_factory.h"

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

Fsm* FsmContainer::GetStateMachine(int machine_id) {
	if (fsm_container_.find(machine_id) == fsm_container_.end()) {
		ERROR("Cannot Find A State Machine To Process Incoming Message");
		return NULL;
	} else {
		return fsm_container_[machine_id];
	}
}

int FsmContainer::DelStateMachine(int machine_id) {
	fsm_container_.erase(machine_id);
	return 0;
}

Fsm* FsmContainer::NewStateMachine(int type) {
	return FsmFactory::Instance().NewFsm(type);
}

void FsmContainer::HandleTimeoutCb() {
	int now = time(NULL);
	map <int, Fsm *>::iterator it; 

	for (it = fsm_container_.begin(); it != fsm_container_.end(); it++) {
		if (now - it->second->GetSocket()->GetLastTimeStamp() >= FSM_STATE_TIMEOUT) {
			ALERT("Fsm Id %d Timeout, Will Be Deleted");
			delete it->second;
		}
	}
}

int FsmTimeoutCb(void *data) {
	FsmContainer::Instance()->HandleTimeoutCb();
	return 0;
}
