/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction *get_inst_info(uint32_t pc)
{
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction()
{
    if (CURRENT_STATE.PC >= (MEM_TEXT_START + (NUM_INST * 4)) || CURRENT_STATE.PC < MEM_TEXT_START)
    {
        RUN_BIT = FALSE;
        return;
    }

    /** Implement this function */
    instruction *instr = get_inst_info(CURRENT_STATE.PC);
    const short op = OPCODE(instr);

    switch (op)
    {
    // R format
    case 0x0:
    {
        const short funct = FUNC(instr);
        const uint32_t rs = CURRENT_STATE.REGS[RS(instr)];
        const uint32_t rt = CURRENT_STATE.REGS[RT(instr)];
        const unsigned char shamt = SHAMT(instr);
        switch (funct)
        {
        case 0x21: // ADDU
            CURRENT_STATE.REGS[RD(instr)] = rs + rt;
            CURRENT_STATE.PC += 4;
            break;
        case 0x24: // ADD
            CURRENT_STATE.REGS[RD(instr)] = (int)rs + (int)rt;
            CURRENT_STATE.PC += 4;
            break;
        case 0x08: // JR
            CURRENT_STATE.PC = rs;
            break;
        case 0x27: // NOR
            CURRENT_STATE.REGS[RD(instr)] = ~(rs | rt);
            CURRENT_STATE.PC += 4;
            break;
        case 0x25: // OR
            CURRENT_STATE.REGS[RD(instr)] = rs | rt;
            CURRENT_STATE.PC += 4;
            break;
        case 0x2b: // SLTU
            CURRENT_STATE.REGS[RD(instr)] = (rs < rt) ? 1 : 0;
            CURRENT_STATE.PC += 4;
            break;
        case 0x00: // SLL
            CURRENT_STATE.REGS[RD(instr)] = rt << shamt;
            CURRENT_STATE.PC += 4;
            break;
        case 0x02: // SRL
            CURRENT_STATE.REGS[RD(instr)] = rt >> shamt;
            CURRENT_STATE.PC += 4;
            break;
        case 0x23: // SUBU
            CURRENT_STATE.REGS[RD(instr)] = rs - rt;
            CURRENT_STATE.PC += 4;
            break;
        }
        break;
    }

    // I format
    case 0x9:  // ADDIU
    case 0xc:  // ANDI
    case 0xf:  // LUI
    case 0xd:  // ORI
    case 0xb:  // SLTIU
    case 0x23: // LW
    case 0x2b: // SW
    case 0x4:  // BEQ
    case 0x5:  // BNE
    {
        const uint32_t rs = CURRENT_STATE.REGS[RS(instr)];
        const uint32_t rt = CURRENT_STATE.REGS[RT(instr)];
        const uint32_t u_imm = IMM(instr);
        const int s_imm = SIGN_EX(IMM(instr));

        switch (op)
        {
        case 0x9:                                       // ADDIU
            CURRENT_STATE.REGS[RT(instr)] = rs + s_imm; // sign 더하는거 맞음
            CURRENT_STATE.PC += 4;
            break;
        case 0xc: // ANDI
            CURRENT_STATE.REGS[RT(instr)] = rs & u_imm;
            CURRENT_STATE.PC += 4;
            break;
        case 0xf: // LUI
            CURRENT_STATE.REGS[RT(instr)] = (u_imm << 16);
            CURRENT_STATE.PC += 4;
            break;
        case 0xd: // ORI
            CURRENT_STATE.REGS[RT(instr)] = rs | u_imm;
            CURRENT_STATE.PC += 4;
            break;
        case 0xb: // SLTIU
            CURRENT_STATE.REGS[RT(instr)] = (rs < (uint32_t)s_imm) ? 1 : 0;
            CURRENT_STATE.PC += 4;
            break;
        case 0x23: // LW
            CURRENT_STATE.REGS[RT(instr)] = mem_read_32(rs + s_imm);
            CURRENT_STATE.PC += 4;
            break;
        case 0x2b: // SW
            mem_write_32(rs + s_imm, rt);
            CURRENT_STATE.PC += 4;
            break;
        case 0x4: // BEQ
            if (rs == rt)
            {
                CURRENT_STATE.PC += 4 + (s_imm << 2);
            }
            else
            {
                CURRENT_STATE.PC += 4;
            }
            break;
        case 0x5: // BNE
            if (rs != rt)
            {
                CURRENT_STATE.PC += 4 + (s_imm << 2);
            }
            else
            {
                CURRENT_STATE.PC += 4;
            }
            break;
        }
        break;
    }

    // J format
    case 0x2: // J
    case 0x3: // JAL
    {
        if (op == 0x3)
        {
            CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        }
        CURRENT_STATE.PC = (CURRENT_STATE.PC & 0xf0000000) + (TARGET(instr) << 2);
        break;
    }
    }
}
