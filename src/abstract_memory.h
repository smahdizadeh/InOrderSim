/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef SRC_ABSTRACT_MEMORY_H_
#define SRC_ABSTRACT_MEMORY_H_
#include<cstdint>


class AbstractMemory {
public:
	AbstractMemory();
	virtual ~AbstractMemory();
	virtual uint32_t read(uint32_t addr, uint32_t size, uint8_t* data)=0;
	virtual uint32_t write(uint32_t addr, uint32_t size, uint8_t* data)=0;
	uint32_t access_delay;
	AbstractMemory* next;

};

#endif /* SRC_ABSTRACT_MEMORY_H_ */
