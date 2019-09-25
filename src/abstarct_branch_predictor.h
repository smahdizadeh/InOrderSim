/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_ABSTARCT_BRANCH_PREDICTOR_H_
#define SRC_ABSTARCT_BRANCH_PREDICTOR_H_
#include <cstdint>

class AbstarctBranchPredictor {
public:
	AbstarctBranchPredictor();
	virtual ~AbstarctBranchPredictor();
	virtual uint32_t getTarget(uint32_t PC) = 0;
	virtual void update(uint32_t PC, bool taken, uint32_t target) = 0;
};

#endif /* SRC_ABSTARCT_BRANCH_PREDICTOR_H_ */
