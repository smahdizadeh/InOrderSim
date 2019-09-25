/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_STATIC_NT_BRANCH_PREDICTOR_H_
#define SRC_STATIC_NT_BRANCH_PREDICTOR_H_

#include "abstarct_branch_predictor.h"

class StaticNTBranchPredictor: public AbstarctBranchPredictor {
public:
	StaticNTBranchPredictor();
	virtual ~StaticNTBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) override;
	virtual void update(uint32_t PC, bool taken, uint32_t target) override;
};

#endif /* SRC_STATIC_NT_BRANCH_PREDICTOR_H_ */
