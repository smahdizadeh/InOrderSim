/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __ABSTRACT_BRANCH_PREDICTOR_H__
#define __ABSTRACT_BRANCH_PREDICTOR_H__
#include <cstdint>

class AbstractBranchPredictor {
public:
	AbstractBranchPredictor();
	virtual ~AbstractBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) = 0;
	virtual void update(uint32_t PC, bool taken, uint32_t target) = 0;
};

#endif
