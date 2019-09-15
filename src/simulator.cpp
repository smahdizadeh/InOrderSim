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
//	l1i_cache = new Cache(info->cache_size_l1, info->cache_assoc_l1,
//			info->cache_blk_size, info->repl_policy_l1i, info->access_delay_l1);
//	l1d_cache = new Cache(info->cache_size_l1, info->cache_assoc_l1,
//			info->cache_blk_size, info->repl_policy_l1d, info->access_delay_l1);
//	l2_cache = new Cache(info->cache_size_l2, info->cache_assoc_l2,
//			info->cache_blk_size, info->repl_policy_l2, info->access_delay_l2);
	main_memory = new BaseMemory(info->read_delay_mem,
			info->write_delay_mem);
//	l1d_cache->next = l2_cache;
//	l1i_cache->next = l2_cache;
//	l2_cache->next = main_memory;
	main_memory->next = nullptr;
	//set the first memory in the memory-hierarchy
	pipe->data_mem = main_memory;
	pipe->inst_mem = main_memory;
}

/***************************************************************/
/*            Execute a cycle                                   /*
/***************************************************************/
void Simulator::cycle() {
  std::cerr<<"cycle:"<<pipe->stat_cycles<<"\n";
  pipe->pipeCycle();
  pipe->stat_cycles++;
}


/***************************************************************/
/*            Simulation for n cycles                           /*
/***************************************************************/
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


/***************************************************************/
/*           Simulation until HALTed                            /*
/***************************************************************/
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


/***************************************************************/
/*                                                             */
/* Procedure: readMemForDump                                   */
/*                                                             */
/* Purpose: Read a 32-bit word from memory for memDump         */
/*                                                             */
/***************************************************************/
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

/***************************************************************/
/*                                                             */
/* Procedure : registerDump                                    */
/*                                                             */
/* Purpose   : Dump architectural registers and other stats    */
/*                                                             */
/***************************************************************/
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



/***************************************************************/
/*                                                             */
/* Procedure : memDump                                         */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void Simulator::memDump(int start, int stop) {
  int address;

  printf("\nMemory content [0x%08x..0x%08x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start; address <= stop; address += 4)
    printf("  0x%08x (%d) : 0x%08x\n", address, address, readMemForDump(address));
  printf("\n");
}




Simulator::~Simulator() {
	delete main_memory;
//	delete l2_cache;
//	delete l1d_cache;
//	delete l1i_cache;
	delete pipe;
}

