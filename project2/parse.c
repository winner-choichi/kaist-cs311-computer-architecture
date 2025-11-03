/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   parse.c                                                   */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "parse.h"
#include "run.h"

int text_size;
int data_size;

instruction parsing_instr(const char *buffer, const int index)
{
	instruction instr;
	/** Implement this function */
	uint32_t uint_buffer = (uint32_t)fromBinary(buffer);
	short op = (uint_buffer >> 26) & 0x3f;
	SET_OPCODE(&instr, op);
	switch (op)
	{
	// J format
	case 0x2: // J
	case 0x3: // JAL
	{
		uint32_t target = (uint32_t)uint_buffer & 0x3ffffff;
		// SET_TARGET(&instr, target); // 이거 mem_addr타입 util.h에 정의 안돼있어서 못쓰는거 아니야?
		instr.r_t.target = target;
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
		unsigned char rs = (uint_buffer >> 21) & 0x1f;
		unsigned char rt = (uint_buffer >> 16) & 0x1f;
		short imm = uint_buffer & 0xffff;
		SET_RS(&instr, rs);
		SET_RT(&instr, rt);
		SET_IMM(&instr, imm);
		break;
	}

	// R format
	case 0x0: // ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
	{
		short funct = uint_buffer & 0x3f;
		unsigned char rs = (uint_buffer >> 21) & 0x1f;
		unsigned char rt = (uint_buffer >> 16) & 0x1f;
		unsigned char rd = (uint_buffer >> 11) & 0x1f;
		unsigned char shamt = (uint_buffer >> 6) & 0x1f;
		SET_FUNC(&instr, funct);
		SET_RS(&instr, rs);
		SET_RT(&instr, rt);
		SET_RD(&instr, rd);
		SET_SHAMT(&instr, shamt);
		break;
	}
	}
	instr.value = uint_buffer;

	uint32_t address = MEM_TEXT_START + (uint32_t)index;
	mem_write_32(address, uint_buffer);
	return instr;
}

void parsing_data(const char *buffer, const int index)
{
	/** Implement this function */
	uint32_t uint_buffer = (uint32_t)fromBinary(buffer);
	uint32_t address = MEM_DATA_START + (uint32_t)index;
	mem_write_32(address, uint_buffer);
	return;
}

void print_parse_result()
{
	int i;
	printf("Instruction Information\n");

	for (i = 0; i < text_size / 4; i++)
	{
		printf("INST_INFO[%d].value : %x\n", i, INST_INFO[i].value);
		printf("INST_INFO[%d].opcode : %d\n", i, INST_INFO[i].opcode);

		switch (INST_INFO[i].opcode)
		{
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
			printf("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].imm : %d\n", i, INST_INFO[i].r_t.r_i.r_i.imm);
			break;

			// R format
		case 0x0: // ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU if JR
			printf("INST_INFO[%d].func_code : %d\n", i, INST_INFO[i].func_code);
			printf("INST_INFO[%d].rs : %d\n", i, INST_INFO[i].r_t.r_i.rs);
			printf("INST_INFO[%d].rt : %d\n", i, INST_INFO[i].r_t.r_i.rt);
			printf("INST_INFO[%d].rd : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.rd);
			printf("INST_INFO[%d].shamt : %d\n", i, INST_INFO[i].r_t.r_i.r_i.r.shamt);
			break;

			// J format
		case 0x2: // J
		case 0x3: // JAL
			printf("INST_INFO[%d].target : %d\n", i, INST_INFO[i].r_t.target);
			break;

		default:
			printf("Not available instruction\n");
			assert(0);
		}
	}

	printf("Memory Dump - Text Segment\n");
	for (i = 0; i < text_size; i += 4)
		printf("text_seg[%d] : %x\n", i, mem_read_32(MEM_TEXT_START + i));
	for (i = 0; i < data_size; i += 4)
		printf("data_seg[%d] : %x\n", i, mem_read_32(MEM_DATA_START + i));
	printf("Current PC: %x\n", CURRENT_STATE.PC);
}
