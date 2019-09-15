/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_UTIL_H_
#define SRC_UTIL_H_
#include <cstdint>
struct MemHrchyInfo{
	uint64_t cache_size_l1;
	uint64_t cache_assoc_l1;
	uint64_t cache_size_l2;
	uint64_t cache_assoc_l2;
	uint64_t cache_blk_size;
	//todo for now keep it int
	int repl_policy_l1i;
	int repl_policy_l1d;
	int repl_policy_l2;
	uint64_t access_delay_l1;
	uint32_t access_delay_l2;
	uint32_t read_delay_mem;
	uint32_t write_delay_mem;
};

enum ReplacementPolicy{
	LRU
};



#endif /* SRC_UTIL_H_ */
