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
#include "util.h"

BaseMemory::BaseMemory(uint32_t memDelay) :
		AbstractMemory(memDelay, 100) {
	/* memory will be dynamically allocated at initialization */
	MEM_REGIONS[0] = {MEM_TEXT_START, MEM_TEXT_SIZE, nullptr};
	MEM_REGIONS[1] = {MEM_DATA_START, MEM_DATA_SIZE, nullptr};
	MEM_REGIONS[2] = {MEM_STACK_START, MEM_STACK_SIZE, nullptr};
	MEM_REGIONS[3] = {MEM_KDATA_START, MEM_KDATA_SIZE, nullptr};
	MEM_REGIONS[4] = {MEM_KTEXT_START, MEM_KTEXT_SIZE, nullptr};

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

bool BaseMemory::sendReq(Packet * pkt) {
	std::cerr << currCycle << " request received : " << std::hex << pkt->addr <<
			std::dec << " " << pkt->size << " " << pkt->type <<
			" " << pkt->ready_time << "\n";
	mem_region_t* mem_region = getMemRegion(pkt->addr, pkt->size);
	if (mem_region) {
		pkt->ready_time += accessDelay;
		std::cerr << "Packet is added to memory reqQueue "
				"with readyTime : " << pkt->ready_time << "\n";
		reqQueue.push(pkt);
		return true;
	} else {
		for (int i = 0; i < MEM_NREGIONS; i++) {
			std::cerr << "MemoryRegion #" << i << " : " << std::hex
					<< MEM_REGIONS[i].start << " "
					<< MEM_REGIONS[i].start + MEM_REGIONS[i].size << std::dec
					<< "\n";
		}
		std::cerr << "Access to a unallocated region of memory : addr : "
				<< std::hex << pkt->addr << " " << pkt->addr + pkt->size << std::dec << "\n";
		assert(false);
	}
}

void BaseMemory::recvResp(Packet* pkt) {
	assert(false && "No one send respond to memory");
	return;
}

mem_region_t*
BaseMemory::getMemRegion(uint32_t addr, uint32_t size) {
	for (int i = 0; i < MEM_NREGIONS; i++) {
		if (addr >= MEM_REGIONS[i].start
				&& (addr + size)
						< (MEM_REGIONS[i].start + MEM_REGIONS[i].size)) {
			return &MEM_REGIONS[i];
		}
	}
	return nullptr;
}

void BaseMemory::Tick() {
	std::cerr << currCycle << " BaseMemory::Tick()\n";
	while (!reqQueue.empty()) {
		if (reqQueue.front()->ready_time <= currCycle) {
			Packet* respPkt = reqQueue.front();
			reqQueue.pop();
			std::cerr << "sending response of packet : " <<
					std::hex << respPkt->addr << std::dec <<
					" " << respPkt->ready_time << "\n";
			if (respPkt->isWrite) {
				mem_region_t* mem_region = getMemRegion(respPkt->addr,
						respPkt->size);
				int index = respPkt->addr - mem_region->start;
				for (uint32_t i = 0; i < respPkt->size; i++) {
					mem_region->mem[index + i] = *(respPkt->data + i);
				}
				respPkt->isReq = false;
				delete respPkt->data;
				respPkt->data = nullptr;
				prev->recvResp(respPkt);
			} else {
				mem_region_t* mem_region = getMemRegion(respPkt->addr,
						respPkt->size);
				int index = respPkt->addr - mem_region->start;
				for (uint32_t i = 0; i < respPkt->size; i++) {
					*(respPkt->data + i) = mem_region->mem[index + i];
				}
				respPkt->isReq = false;
				prev->recvResp(respPkt);
			}
		}
		else {
			// Assumed that reqQueue is sorted by ready_time
			break;
		}
	}
	return;
}

void BaseMemory::dumpRead(uint32_t addr, uint32_t size, uint8_t* data) {
	mem_region_t* mem_region = getMemRegion(addr, size);
	int index = addr - mem_region->start;
	for (uint32_t i = 0; i < size; i++) {
		*(data + i) = mem_region->mem[index + i];
	}
}
