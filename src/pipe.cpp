/*
 * Computer Architecture CSE530
 * MIPS pipeline cycle-accurate simulator
 * PSU
 */

#include "pipe.h"
#include "mips.h"
#include "base_memory.h"
#include <cstdio>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cassert>

//#define DEBUG

/* debug */
void printOp(Pipe_Op *op)
{
    if (op)
        printf("OP (PC=%08x inst=%08x) src1=R%d (%08x) src2=R%d (%08x) dst=R%d valid %d (%08x) br=%d taken=%d dest=%08x mem=%d addr=%08x\n",
                op->pc, op->instruction, op->reg_src1, op->reg_src1_value, op->reg_src2, op->reg_src2_value, op->reg_dst, op->reg_dst_value_ready,
                op->reg_dst_value, op->is_branch, op->branch_taken, op->branch_dest, op->is_mem, op->mem_addr);
    else
        printf("(null)\n");
}


PipeState::PipeState() : fetch_op(nullptr), decode_op(nullptr), execute_op(nullptr),
		mem_op(nullptr), wb_op(nullptr), data_mem(nullptr), inst_mem(nullptr),
		HI(0), LO(0), branch_recover(0),branch_dest(0), branch_flush(0),
		multiplier_stall(0), memory_stall(0), fetch_stall(0), RUN_BIT(true), stat_cycles(0),
		stat_inst_retire(0), stat_inst_fetch(0), stat_squash(0)
{
	fetch_op = (Pipe_Op *)malloc(sizeof(Pipe_Op));
	memset(fetch_op, 0, sizeof(Pipe_Op));
	for(int i=0; i<32; i++){
		REGS[i]=0;
	}
    PC = 0x00400000;
}

void PipeState::pipeCycle()
{
#ifdef DEBUG
    printf("\n\n----\n\nPIPELINE:\n");
    printf("DCODE: "); print_op(pipe.decode_op);
    printf("EXEC : "); print_op(pipe.execute_op);
    printf("MEM  : "); print_op(pipe.mem_op);
    printf("WB   : "); print_op(pipe.wb_op);
    printf("\n");
#endif

    pipeStageWb();
    pipeStageMem();
    pipeStageExecute();
    pipeStageDecode();
    pipeStageFetch();

    /* handle branch recoveries */
    if (branch_recover) {
#ifdef DEBUG
        printf("branch recovery: new dest %08x flush %d stages\n", pipe.branch_dest, pipe.branch_flush);
#endif

        PC = branch_dest;

        if (branch_flush >= 2) {
            if (decode_op) free(decode_op);
            decode_op = nullptr;
        }

        if (branch_flush >= 3) {
            if (execute_op) free(execute_op);
            execute_op = nullptr;
        }

        if (branch_flush >= 4) {
            if (mem_op) free(mem_op);
            mem_op = nullptr;
        }

        if (branch_flush >= 5) {
            if (wb_op) free(wb_op);
            wb_op = nullptr;
        }

        branch_recover = 0;
        branch_dest = 0;
        branch_flush = 0;

        stat_squash++;
    }
}

void PipeState::pipeRecover(int flush, uint32_t dest)
{
    /* if there is already a recovery scheduled, it must have come from a later
     * stage (which executes older instructions), hence that recovery overrides
     * our recovery. Simply return in this case. */
    if (branch_recover) return;

    /* schedule the recovery. This will be done once all pipeline stages simulate the current cycle. */
    branch_recover = 1;
    branch_flush = flush;
    branch_dest = dest;
}

void PipeState::pipeStageWb()
{
    /* if there is no instruction in this pipeline stage, we are done */
    if (!wb_op)
        return;

    /* grab the op out of our input slot */
    Pipe_Op *op = wb_op;
    wb_op = NULL;

    /* if this instruction writes a register, do so now */
    if (op->reg_dst != -1 && op->reg_dst != 0) {
        REGS[op->reg_dst] = op->reg_dst_value;
#ifdef DEBUG
        printf("R%d = %08x\n", op->reg_dst, op->reg_dst_value);
#endif
    }

    /* if this was a syscall, perform action */
    if (op->opcode == OP_SPECIAL && op->subop == SUBOP_SYSCALL) {
        if (op->reg_src1_value == 0xA) {
            PC = op->pc; /* fetch will do pc += 4, then we stop with correct PC */
            RUN_BIT = false;
        }
    }

    /* free the op */
    free(op);

    stat_inst_retire++;
}

void PipeState::pipeStageMem()
{
    /* if there is no instruction in this pipeline stage, we are done */
    if (!mem_op)
        return;

    /* grab the op out of our input slot */
    Pipe_Op *op = mem_op;

    if(op->is_mem == false) {
    	mem_op = NULL;
    	wb_op = op;
    	return;
    }

    if(memory_stall > 0) {
    	memory_stall--;
    	if(memory_stall == 0) {
    	    mem_op = NULL;
    	    wb_op = op;
    	}
    	return;
    }

    switch (op->opcode) {
        case OP_LW:
        case OP_LH:
        case OP_LHU:
        case OP_LB:
        case OP_LBU:
            {
            	uint32_t val;
            	std::cerr << "Inst : " << std::hex << op->instruction << std::dec << "\n";
            	memory_stall = data_mem->read((op->mem_addr & ~3),
            			4, (uint8_t*) &val) - 1;
                /* extract needed value */
                op->reg_dst_value_ready = 1;
                if (op->opcode == OP_LW) {
                    op->reg_dst_value = val;
                }
                else if (op->opcode == OP_LH || op->opcode == OP_LHU) {
                    if (op->mem_addr & 2)
                        val = (val >> 16) & 0xFFFF;
                    else
                        val = val & 0xFFFF;

                    // sign-extend
                    if (op->opcode == OP_LH)
                        val |= (val & 0x8000) ? 0xFFFF8000 : 0;

                    op->reg_dst_value = val;
                }
                else if (op->opcode == OP_LB || op->opcode == OP_LBU) {
                    switch (op->mem_addr & 3) {
                        case 0:
                            val = val & 0xFF;
                            break;
                        case 1:
                            val = (val >> 8) & 0xFF;
                            break;
                        case 2:
                            val = (val >> 16) & 0xFF;
                            break;
                        case 3:
                            val = (val >> 24) & 0xFF;
                            break;
                    }

                    // sign-extend
                    if (op->opcode == OP_LB)
                        val |= (val & 0x80) ? 0xFFFFFF80 : 0;

                    op->reg_dst_value = val;
                }
            }
            break;

        case OP_SB:
        {
        	uint8_t written_val = op->mem_value & 0xFF;
        	memory_stall = data_mem->write(op->mem_addr,
        			1, &written_val) - 1;
            break;
        }

        case OP_SH:
        {
            uint16_t written_val = op->mem_value & 0xFFFF;
            memory_stall = data_mem->write(op->mem_addr,
            		2, (uint8_t*) &written_val) - 1;
            break;
        }

        case OP_SW:
        {
        	uint32_t written_val = op->mem_value;
        	memory_stall = data_mem->write(op->mem_addr,
        			4, (uint8_t*) &written_val) - 1;
            break;
        }
    }

    if(memory_stall == 0) {
    	mem_op = NULL;
    	wb_op = op;
    }
    return;
}

void PipeState::pipeStageExecute()
{
    /* if a multiply/divide is in progress, decrement cycles until value is ready */
    if (multiplier_stall > 0)
        multiplier_stall--;

    /* if downstream stall, return (and leave any input we had) */
    if (mem_op != NULL)
        return;

    /* if no op to execute, return */
    if (execute_op == NULL)
        return;

    /* grab op and read sources */
    Pipe_Op *op = execute_op;

    /* read register values, and check for bypass; stall if necessary */
    int stall = 0;
    if (op->reg_src1 != -1) {
        if (op->reg_src1 == 0)
            op->reg_src1_value = 0;
        else if (mem_op && mem_op->reg_dst == op->reg_src1) {
            if (!mem_op->reg_dst_value_ready)
                stall = 1;
            else
                op->reg_src1_value = mem_op->reg_dst_value;
        }
        else if (wb_op && wb_op->reg_dst == op->reg_src1) {
            op->reg_src1_value = wb_op->reg_dst_value;
        }
        else
            op->reg_src1_value = REGS[op->reg_src1];
    }
    if (op->reg_src2 != -1) {
        if (op->reg_src2 == 0)
            op->reg_src2_value = 0;
        else if (mem_op && mem_op->reg_dst == op->reg_src2) {
            if (!mem_op->reg_dst_value_ready)
                stall = 1;
            else
                op->reg_src2_value = mem_op->reg_dst_value;
        }
        else if (wb_op && wb_op->reg_dst == op->reg_src2) {
            op->reg_src2_value = wb_op->reg_dst_value;
        }
        else
            op->reg_src2_value = REGS[op->reg_src2];
    }

    /* if bypassing requires a stall (e.g. use immediately after load),
     * return without clearing stage input */
    if (stall) 
        return;

    /* execute the op */
    switch (op->opcode) {
        case OP_SPECIAL:
            op->reg_dst_value_ready = 1;
            switch (op->subop) {
                case SUBOP_SLL:
                    op->reg_dst_value = op->reg_src2_value << op->shamt;
                    break;
                case SUBOP_SLLV:
                    op->reg_dst_value = op->reg_src2_value << op->reg_src1_value;
                    break;
                case SUBOP_SRL:
                    op->reg_dst_value = op->reg_src2_value >> op->shamt;
                    break;
                case SUBOP_SRLV:
                    op->reg_dst_value = op->reg_src2_value >> op->reg_src1_value;
                    break;
                case SUBOP_SRA:
                    op->reg_dst_value = (int32_t)op->reg_src2_value >> op->shamt;
                    break;
                case SUBOP_SRAV:
                    op->reg_dst_value = (int32_t)op->reg_src2_value >> op->reg_src1_value;
                    break;
                case SUBOP_JR:
                case SUBOP_JALR:
                    op->reg_dst_value = op->pc + 4;
                    op->branch_dest = op->reg_src1_value;
                    op->branch_taken = 1;
                    break;

                case SUBOP_MULT:
                    {
                        /* we set a result value right away; however, we will
                         * model a stall if the program tries to read the value
                         * before it's ready (or overwrite HI/LO). Also, if
                         * another multiply comes down the pipe later, it will
                         * update the values and re-set the stall cycle count
                         * for a new operation.
                         */
                        int64_t val = (int64_t)((int32_t)op->reg_src1_value) * (int64_t)((int32_t)op->reg_src2_value);
                        uint64_t uval = (uint64_t)val;
                        HI = (uval >> 32) & 0xFFFFFFFF;
                        LO = (uval >>  0) & 0xFFFFFFFF;

                        /* four-cycle multiplier latency */
                        multiplier_stall = 4;
                    }
                    break;
                case SUBOP_MULTU:
                    {
                        uint64_t val = (uint64_t)op->reg_src1_value * (uint64_t)op->reg_src2_value;
                        HI = (val >> 32) & 0xFFFFFFFF;
                        LO = (val >>  0) & 0xFFFFFFFF;

                        /* four-cycle multiplier latency */
                        multiplier_stall = 4;
                    }
                    break;

                case SUBOP_DIV:
                    if (op->reg_src2_value != 0) {

                        int32_t val1 = (int32_t)op->reg_src1_value;
                        int32_t val2 = (int32_t)op->reg_src2_value;
                        int32_t div, mod;

                        div = val1 / val2;
                        mod = val1 % val2;

                        LO = div;
                        HI = mod;
                    } else {
                        // really this would be a div-by-0 exception
                        HI = LO = 0;
                    }

                    /* 32-cycle divider latency */
                    multiplier_stall = 32;
                    break;

                case SUBOP_DIVU:
                    if (op->reg_src2_value != 0) {
                        HI = (uint32_t)op->reg_src1_value % (uint32_t)op->reg_src2_value;
                        LO = (uint32_t)op->reg_src1_value / (uint32_t)op->reg_src2_value;
                    } else {
                        /* really this would be a div-by-0 exception */
                        HI = LO = 0;
                    }

                    /* 32-cycle divider latency */
                    multiplier_stall = 32;
                    break;

                case SUBOP_MFHI:
                    /* stall until value is ready */
                    if (multiplier_stall > 0)
                        return;

                    op->reg_dst_value = HI;
                    break;
                case SUBOP_MTHI:
                    /* stall to respect WAW dependence */
                    if (multiplier_stall > 0)
                        return;

                    HI = op->reg_src1_value;
                    break;

                case SUBOP_MFLO:
                    /* stall until value is ready */
                    if (multiplier_stall > 0)
                        return;

                    op->reg_dst_value = LO;
                    break;
                case SUBOP_MTLO:
                    /* stall to respect WAW dependence */
                    if (multiplier_stall > 0)
                        return;

                    LO = op->reg_src1_value;
                    break;

                case SUBOP_ADD:
                case SUBOP_ADDU:
                    op->reg_dst_value = op->reg_src1_value + op->reg_src2_value;
                    break;
                case SUBOP_SUB:
                case SUBOP_SUBU:
                    op->reg_dst_value = op->reg_src1_value - op->reg_src2_value;
                    break;
                case SUBOP_AND:
                    op->reg_dst_value = op->reg_src1_value & op->reg_src2_value;
                    break;
                case SUBOP_OR:
                    op->reg_dst_value = op->reg_src1_value | op->reg_src2_value;
                    break;
                case SUBOP_NOR:
                    op->reg_dst_value = ~(op->reg_src1_value | op->reg_src2_value);
                    break;
                case SUBOP_XOR:
                    op->reg_dst_value = op->reg_src1_value ^ op->reg_src2_value;
                    break;
                case SUBOP_SLT:
                    op->reg_dst_value = ((int32_t)op->reg_src1_value <
                            (int32_t)op->reg_src2_value) ? 1 : 0;
                    break;
                case SUBOP_SLTU:
                    op->reg_dst_value = (op->reg_src1_value < op->reg_src2_value) ? 1 : 0;
                    break;
            }
            break;

        case OP_BRSPEC:
            switch (op->subop) {
                case BROP_BLTZ:
                case BROP_BLTZAL:
                    if ((int32_t)op->reg_src1_value < 0) op->branch_taken = 1;
                    break;

                case BROP_BGEZ:
                case BROP_BGEZAL:
                    if ((int32_t)op->reg_src1_value >= 0) op->branch_taken = 1;
                    break;
            }
            break;

        case OP_BEQ:
            if (op->reg_src1_value == op->reg_src2_value) op->branch_taken = 1;
            break;

        case OP_BNE:
            if (op->reg_src1_value != op->reg_src2_value) op->branch_taken = 1;
            break;

        case OP_BLEZ:
            if ((int32_t)op->reg_src1_value <= 0) op->branch_taken = 1;
            break;

        case OP_BGTZ:
            if ((int32_t)op->reg_src1_value > 0) op->branch_taken = 1;
            break;

        case OP_ADDI:
        case OP_ADDIU:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = op->reg_src1_value + op->se_imm16;
            break;
        case OP_SLTI:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = (int32_t)op->reg_src1_value < (int32_t)op->se_imm16 ? 1 : 0;
            break;
        case OP_SLTIU:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = (uint32_t)op->reg_src1_value < (uint32_t)op->se_imm16 ? 1 : 0;
            break;
        case OP_ANDI:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = op->reg_src1_value & op->imm16;
            break;
        case OP_ORI:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = op->reg_src1_value | op->imm16;
            break;
        case OP_XORI:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = op->reg_src1_value ^ op->imm16;
            break;
        case OP_LUI:
            op->reg_dst_value_ready = 1;
            op->reg_dst_value = op->imm16 << 16;
            break;

        case OP_LW:
        case OP_LH:
        case OP_LHU:
        case OP_LB:
        case OP_LBU:
            op->mem_addr = op->reg_src1_value + op->se_imm16;
            break;

        case OP_SW:
        case OP_SH:
        case OP_SB:
            op->mem_addr = op->reg_src1_value + op->se_imm16;
            op->mem_value = op->reg_src2_value;
            break;
    }

    /* handle branch recoveries at this point */
    if (op->branch_taken)
        pipeRecover(3, op->branch_dest);

    /* remove from upstream stage and place in downstream stage */
    execute_op = NULL;
    mem_op = op;
}

void PipeState::pipeStageDecode()
{
    /* if downstream stall, return (and leave any input we had) */
    if (execute_op != NULL)
        return;

    /* if no op to decode, return */
    if (decode_op == NULL)
        return;

    /* grab op and remove from stage input */
    Pipe_Op *op = decode_op;
    decode_op = NULL;

    /* set up info fields (source/dest regs, immediate, jump dest) as necessary */
    uint32_t opcode = (op->instruction >> 26) & 0x3F;
    uint32_t rs = (op->instruction >> 21) & 0x1F;
    uint32_t rt = (op->instruction >> 16) & 0x1F;
    uint32_t rd = (op->instruction >> 11) & 0x1F;
    uint32_t shamt = (op->instruction >> 6) & 0x1F;
    uint32_t funct1 = (op->instruction >> 0) & 0x1F;
    uint32_t funct2 = (op->instruction >> 0) & 0x3F;
    uint32_t imm16 = (op->instruction >> 0) & 0xFFFF;
    uint32_t se_imm16 = imm16 | ((imm16 & 0x8000) ? 0xFFFF8000 : 0);
    uint32_t targ = (op->instruction & ((1UL << 26) - 1)) << 2;

    op->opcode = opcode;
    op->imm16 = imm16;
    op->se_imm16 = se_imm16;
    op->shamt = shamt;

    switch (opcode) {
        case OP_SPECIAL:
            /* all "SPECIAL" insts are R-types that use the ALU and both source
             * regs. Set up source regs and immediate value. */
            op->reg_src1 = rs;
            op->reg_src2 = rt;
            op->reg_dst = rd;
            op->subop = funct2;
            if (funct2 == SUBOP_SYSCALL) {
                op->reg_src1 = 2; // v0
                op->reg_src2 = 3; // v1
            }
            if (funct2 == SUBOP_JR || funct2 == SUBOP_JALR) {
                op->is_branch = 1;
                op->branch_cond = 0;
            }

            break;

        case OP_BRSPEC:
            /* branches that have -and-link variants come here */
            op->is_branch = 1;
            op->reg_src1 = rs;
            op->reg_src2 = rt;
            op->is_branch = 1;
            op->branch_cond = 1; /* conditional branch */
            op->branch_dest = op->pc + 4 + (se_imm16 << 2);
            op->subop = rt;
            if (rt == BROP_BLTZAL || rt == BROP_BGEZAL) {
                /* link reg */
                op->reg_dst = 31;
                op->reg_dst_value = op->pc + 4;
                op->reg_dst_value_ready = 1;
            }
            break;

        case OP_JAL:
            op->reg_dst = 31;
            op->reg_dst_value = op->pc + 4;
            op->reg_dst_value_ready = 1;
            op->branch_taken = 1;
            /* fallthrough */
        case OP_J:
            op->is_branch = 1;
            op->branch_cond = 0;
            op->branch_taken = 1;
            op->branch_dest = (op->pc & 0xF0000000) | targ;
            break;

        case OP_BEQ:
        case OP_BNE:
        case OP_BLEZ:
        case OP_BGTZ:
            /* ordinary conditional branches (resolved after execute) */
            op->is_branch = 1;
            op->branch_cond = 1;
            op->branch_dest = op->pc + 4 + (se_imm16 << 2);
            op->reg_src1 = rs;
            op->reg_src2 = rt;
            break;

        case OP_ADDI:
        case OP_ADDIU:
        case OP_SLTI:
        case OP_SLTIU:
            /* I-type ALU ops with sign-extended immediates */
            op->reg_src1 = rs;
            op->reg_dst = rt;
            break;

        case OP_ANDI:
        case OP_ORI:
        case OP_XORI:
        case OP_LUI:
            /* I-type ALU ops with non-sign-extended immediates */
            op->reg_src1 = rs;
            op->reg_dst = rt;
            break;

        case OP_LW:
        case OP_LH:
        case OP_LHU:
        case OP_LB:
        case OP_LBU:
        case OP_SW:
        case OP_SH:
        case OP_SB:
            /* memory ops */
            op->is_mem = 1;
            op->reg_src1 = rs;
            if (opcode == OP_LW || opcode == OP_LH || opcode == OP_LHU || opcode == OP_LB || opcode == OP_LBU) {
                /* load */
                op->mem_write = 0;
                op->reg_dst = rt;
            }
            else {
                /* store */
                op->mem_write = 1;
                op->reg_src2 = rt;
            }
            break;
    }

    /* we will handle reg-read together with bypass in the execute stage */

    /* place op in downstream slot */
    execute_op = op;
}

void PipeState::pipeStageFetch()
{
    /* if pipeline is stalled (our output slot is not empty), return */
    if (decode_op != NULL)
		return;

	if (fetch_stall > 0) {
		fetch_stall--;
		if (fetch_stall == 0) {
			PC += 4;
			decode_op = fetch_op;
			fetch_op = (Pipe_Op *)malloc(sizeof(Pipe_Op));
			memset(fetch_op, 0, sizeof(Pipe_Op));
			stat_inst_fetch++;
		}
		return;
	}
	assert(fetch_op != nullptr);

	fetch_op->reg_src1 = fetch_op->reg_src2 = fetch_op->reg_dst = -1;
    fetch_op->pc = PC;
    fetch_stall = inst_mem->read(PC, 4, (uint8_t*) &(fetch_op->instruction)) - 1;
    std::cerr << "fetch_stall " << fetch_stall << "\n";
    if(fetch_stall == 0) {
    	PC += 4;
    	decode_op = fetch_op;
    	memset(fetch_op, 0, sizeof(Pipe_Op));
    	stat_inst_fetch++;
    }

}
