#include "fsm.h"
#include "fsm_container.h"
#include "fsm_factory.h"

using namespace std;

map<int, map<int, state_cb_t> > Fsm::fsm_callbacks_;

static int machine_flow_number = 0;

static int generate_machine_number() {
	return ++machine_flow_number;
}

int Fsm::OnMessage(SMessage *pmessage, Socket *sk) {
	DEBUG("Incoming Message From %s:%d\n%s", iptostr(sk->GetPeerAddr().sin_addr.s_addr), ntohs(sk->GetPeerAddr().sin_port),
	pmessage->DebugString().c_str());
	int message_type = pmessage->header().type();
	int dst_fsm_id = pmessage->header().dst_fsm();

	Fsm* pstatemachine = NULL;
	// Initial State
	if (dst_fsm_id == 0)	{
		// Create StateMachine
		pstatemachine = FsmContainer::Instance()->NewStateMachine(message_type);
		pstatemachine->SetSocket(sk);
		if (pstatemachine == NULL) {
			ERROR("Cannot Find StateMachine To Handle Incoming Request");
			return FSM_NOTEXIST;
		}
		Status_t ret = pstatemachine->InvokeCb(pmessage);
		if (ret == FSM_ERROR || ret == FSM_FINISH) {
			delete pstatemachine;
		}
		return ret;
	} else {
		pstatemachine = FsmContainer::Instance()->GetStateMachine(dst_fsm_id);
		if (pstatemachine == NULL) {
			pstatemachine = FsmContainer::Instance()->NewStateMachine(message_type);
			if (pstatemachine == NULL) {
				ERROR("Cannot Find StateMachine To Handle Incoming Request");
				return FSM_NOTEXIST;
			}
			Status_t ret = pstatemachine->InvokeCb(pmessage);
			if (ret == FSM_ERROR || ret == FSM_FINISH) {
				delete pstatemachine;
			}
			return ret;
		} else {
			Status_t ret = pstatemachine->InvokeCb(pmessage);
			if (ret == FSM_ERROR || ret == FSM_FINISH) {
				delete pstatemachine;
			}
			return ret;
		}
	}
}

int Fsm::SetGlobalStateName(int type, int state, state_cb_t callback) {
    map<int, map<int, state_cb_t> >::iterator it = fsm_callbacks_.find(type);
    if ( it == fsm_callbacks_.end() ) {
        map<int, state_cb_t> tmp;
        tmp.insert(make_pair(state, callback));
        fsm_callbacks_.insert(make_pair(type, tmp));
    } else {
        pair<map<int, state_cb_t>::iterator, bool> ret;
        ret = it->second.insert(make_pair(state, callback));
        return (ret.second? 0: -1);
    }
	return 0;
}

Status_t Fsm::InvokeCb(SMessage *pmessage) {
	int mytype = this->FsmType();
	int mystate = state_;
	
	map<int, map<int, state_cb_t> >::iterator fsm_it;
	map<int, state_cb_t>::iterator cb_it;

	fsm_it = fsm_callbacks_.find(mytype);
	if (fsm_it == fsm_callbacks_.end()) {
		ERROR("Cannot Find Corresponding Fsm, MyType:%d, State:%d", mytype, mystate);
		return FSM_ERROR;
	}

	cb_it = fsm_it->second.find(mystate);	
	if (cb_it == fsm_it->second.end()) {
		ERROR("Cannot Find Callback, MyType:%d, State:%d", mytype, mystate);
		return FSM_ERROR;
	}
	
	return cb_it->second(this, pmessage);
}

Fsm::Fsm() {
	machine_id_ = generate_machine_number();
	FsmContainer::Instance()->AddStateMachine(machine_id_, this);
}

Fsm::~Fsm() {
	FsmContainer::Instance()->DelStateMachine(machine_id_);
}
