#ifndef _FSM_CONTAINER_
#define _FSM_CONTAINER_

#include <map>

#include "sock.h"
#include "fsm.h"

int FsmTimeoutCb(void *data);

class FsmContainer {
public:
	~FsmContainer();

	int AddStateMachine(int machine_id, Fsm *pfsm);

	Fsm* GetStateMachine(int machine_id);

	Fsm* NewStateMachine(int type);

	int DelStateMachine(int machine_id);

	void HandleTimeoutCb();

	static FsmContainer* Instance();

private:
	// Private Constructor For Singletion
	FsmContainer() {};

private:
	std::map <int, Fsm *> fsm_container_; // Machine Id <-> Machine Pointer

	static FsmContainer* p_container_;
};

#endif
