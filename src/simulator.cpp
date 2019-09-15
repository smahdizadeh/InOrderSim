/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include <cstdio>
#include <iostream>
#include "simulator.h"


Simulator::Simulator(MemHrchyInfo* info){
	printf("initialize simulator\n\n");
	//initializing core
	pipe = new PipeState();

	//initializing memory hierarchy
	main_memory = new BaseMemory(info->read_delay_mem,
			info->write_delay_mem);
	main_memory->next = nullptr;

	//set the first memory in the memory-hierarchy
	pipe->data_mem = main_memory;
	pipe->inst_mem = main_memory;
}

void Simulator::cycle() {
  std::cerr<<"cycle:"<<pipe->stat_cycles<<"\n";
  pipe->pipeCycle();
  pipe->stat_cycles++;
}


void Simulator::run(int num_cycles) {
  int i;

  if (pipe->RUN_BIT == false) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++) {
    if (pipe->RUN_BIT == false) {
	    printf("Simulator halted\n\n");
	    break;
    }
    cycle();
  }
}


void Simulator::go() {
  if (pipe->RUN_BIT == false) {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (pipe->RUN_BIT)
	  cycle();
  printf("Simulator halted\n\n");
}



uint32_t Simulator::readMemForDump(uint32_t address)
{
    int i;
    for (i = 0; i < MEM_NREGIONS; i++) {
        if (address >= main_memory->MEM_REGIONS[i].start &&
                address < (main_memory->MEM_REGIONS[i].start + main_memory->MEM_REGIONS[i].size)) {
            uint32_t offset = address - main_memory->MEM_REGIONS[i].start;

            return
                (main_memory->MEM_REGIONS[i].mem[offset+3] << 24) |
                (main_memory->MEM_REGIONS[i].mem[offset+2] << 16) |
                (main_memory->MEM_REGIONS[i].mem[offset+1] <<  8) |
                (main_memory->MEM_REGIONS[i].mem[offset+0] <<  0);
        }
    }

    return 0;
}


void Simulator::registerDump() {
    int i;

    printf("PC: 0x%08x\n", pipe->PC);

    for (i = 0; i < 32; i++) {
        printf("R%d: 0x%08x\n", i, pipe->REGS[i]);
    }

    printf("HI: 0x%08x\n", pipe->HI);
    printf("LO: 0x%08x\n", pipe->LO);
    printf("Cycles: %u\n", pipe->stat_cycles);
    printf("FetchedInstr: %u\n", pipe->stat_inst_fetch);
    printf("RetiredInstr: %u\n", pipe->stat_inst_retire);
    printf("IPC: %0.3f\n", ((float) pipe->stat_inst_retire) / pipe->stat_cycles);
    printf("Flushes: %u\n", pipe->stat_squash);
}


void Simulator::memDump(int start, int stop) {
  int address;

  printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start; address <= stop; address += 4){
    printf("  0x%08x (%d) : 0x%08x\n", address, address, readMemForDump(address));
  }
  printf("\n");
}




Simulator::~Simulator() {
	delete main_memory;
	delete pipe;
}

