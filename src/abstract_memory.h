/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#ifndef __ABSTRACT_MEMORY_H__
#define __ABSTRACT_MEMORY_H__
#include<cstdint>
#include <queue>
#include "util.h"
#include "base_object.h"

class AbstractMemory : public BaseObject {
public:
	AbstractMemory(uint32_t delay, uint32_t reqQueueCapacity);
	virtual ~AbstractMemory();
	virtual void Tick() = 0;
	virtual bool sendReq(Packet * pkt) = 0;
	virtual void recvResp(Packet* readRespPkt) = 0;

	std::queue<Packet*> reqQueue;

	uint32_t accessDelay;
	uint32_t reqQueueCapacity;
	BaseObject* next;
	BaseObject* prev;

	virtual void dumpRead(uint32_t addr, uint32_t size, uint8_t* data) = 0;
};

#endif
