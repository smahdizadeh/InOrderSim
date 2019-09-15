/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "base_memory.h"

BaseMemory::BaseMemory(uint32_t mem_read_delay, uint32_t mem_write_delay)
:mem_read_delay(mem_read_delay),mem_write_delay(mem_write_delay) {
	/* memory will be dynamically allocated at initialization */
	MEM_REGIONS[0] = { MEM_TEXT_START, MEM_TEXT_SIZE, nullptr };
	MEM_REGIONS[1] = { MEM_DATA_START, MEM_DATA_SIZE, nullptr };
	MEM_REGIONS[2] = { MEM_STACK_START, MEM_STACK_SIZE, nullptr };
	MEM_REGIONS[3] = { MEM_KDATA_START, MEM_KDATA_SIZE, nullptr };
	MEM_REGIONS[4] = { MEM_KTEXT_START, MEM_KTEXT_SIZE, nullptr };

	for (int i = 0; i < MEM_NREGIONS; i++) {
		MEM_REGIONS[i].mem = (uint8_t *)malloc(MEM_REGIONS[i].size);
		memset(MEM_REGIONS[i].mem, 0, MEM_REGIONS[i].size);
	}
}

BaseMemory::~BaseMemory() {
	// TODO Auto-generated destructor stub
	for (int i = 0; i < MEM_NREGIONS; i++)
		free(MEM_REGIONS[i].mem);
}

uint32_t
BaseMemory::read(uint32_t addr, uint32_t size, uint8_t* data){
	mem_region_t* mem_region = getMemRegion(addr, size);
	if(mem_region){
		int index = addr - mem_region->start;
		for(uint32_t i = 0; i < size; i++){
			*(data+i)= mem_region->mem[index+i];
		}
		return mem_read_delay;
	}
	else{
		for(int i = 0; i < MEM_NREGIONS; i++){
			std::cerr << "MemoryRegion #" << i << " : " <<
					std::hex << MEM_REGIONS[i].start << " " <<
					MEM_REGIONS[i].start + MEM_REGIONS[i].size << std::dec << "\n";
		}
		std::cerr << "Access to a unallocated region of memory : addr : " <<
				std::hex << addr << " " << addr + size << std::dec << "\n";
		assert(false);
	}
}

uint32_t
BaseMemory::write(uint32_t addr, uint32_t size, uint8_t* data) {
	mem_region_t* mem_region = getMemRegion(addr, size);
	if (mem_region) {
		int index = addr - mem_region->start;
		for (uint32_t i = 0; i < size; i++) {
			mem_region->mem[index+i] = *(data+i);
		}
		return mem_write_delay;
	} else {
		for(int i = 0; i < MEM_NREGIONS; i++){
			std::cerr << "MemoryRegion #" << i << " : " <<
					std::hex << MEM_REGIONS[i].start << " " <<
					MEM_REGIONS[i].start + MEM_REGIONS[i].size << std::dec << "\n";
		}
		std::cerr << "Access to a unallocated region of memory : addr : " <<
				std::hex << addr << " " << addr + size << std::dec << "\n";
		assert(false);
	}
}

mem_region_t*
BaseMemory::getMemRegion(uint32_t addr, uint32_t size){
	for(int i = 0; i < MEM_NREGIONS; i++){
		if(addr >= MEM_REGIONS[i].start && (addr + size) < (MEM_REGIONS[i].start + MEM_REGIONS[i].size)){
			return &MEM_REGIONS[i];
		}
	}
	return nullptr;
}

