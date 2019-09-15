/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_SIMULATOR_H_
#define SRC_SIMULATOR_H_
//todo
//#include "cache.h"
#include "base_memory.h"
#include "pipe.h"
#include "util.h"
class Simulator {
public:
	Simulator(MemHrchyInfo* info);
	virtual ~Simulator();
	PipeState * pipe;
//	Cache * l1i_cache;
//	Cache * l1d_cache;
//	Cache * l2_cache;
	BaseMemory * main_memory;

	void cycle();
	void run(int num_cycles);
	void go();

	//debug
	uint32_t readMemForDump(uint32_t address);
	void registerDump();
	void memDump(int start, int stop);


};

#endif /* SRC_SIMULATOR_H_ */
