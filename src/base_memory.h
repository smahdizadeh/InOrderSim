/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_BASE_MEMORY_H_
#define SRC_BASE_MEMORY_H_
#include <cstdint>
#include <cstdlib>
#include <assert.h>
#include "abstract_memory.h"


#define MEM_DATA_START  0x10000000
#define MEM_DATA_SIZE   0x00100000
#define MEM_TEXT_START  0x00400000
#define MEM_TEXT_SIZE   0x00100000
#define MEM_STACK_START 0x7ff00000
#define MEM_STACK_SIZE  0x00100000
#define MEM_KDATA_START 0x90000000
#define MEM_KDATA_SIZE  0x00100000
#define MEM_KTEXT_START 0x80000000
#define MEM_KTEXT_SIZE  0x00100000
#define MEM_NREGIONS 5


typedef struct {
    uint32_t start, size;
    uint8_t *mem;
} mem_region_t;




class BaseMemory: public AbstractMemory {
public:
	BaseMemory(uint32_t mem_read_delay, uint32_t mem_write_delay);
	virtual ~BaseMemory();
	virtual uint32_t read(uint32_t addr, uint32_t size, uint8_t* data) override;
	virtual uint32_t write(uint32_t addr, uint32_t size, uint8_t* data) override;
	mem_region_t* getMemRegion(uint32_t addr, uint32_t size);
	uint32_t mem_read_delay;
	uint32_t mem_write_delay;


	mem_region_t MEM_REGIONS[MEM_NREGIONS];

};

#endif /* SRC_BASE_MEMORY_H_ */
