/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __UTIL_H__
#define __UTIL_H__
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
	uint32_t memDelay;
};

enum ReplacementPolicy{
	RandomReplPolicy,
	LRUReplPolicy
};

enum PacketSrcType {
	PacketTypeFetch = 0,
	PacketTypeLoad = 1,
	PacketTypeStore = 2,
	PacketTypePrefetch = 3
};


uint64_t extern currCycle;


#endif
