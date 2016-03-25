//
// Created by selly on 2016/3/23.
//

#ifndef CPU_SIMULATOR_INSTRUCTION_H
#define CPU_SIMULATOR_INSTRUCTION_H

#define R_FORMAT    0x0
#define OP_J        0x2
#define OP_JAL      0x3
#define OP_BEQ      0x4
#define OP_BNE      0x5
#define OP_BGTZ     0x7
#define OP_ADDI     0x8
#define OP_ADDIU    0x9
#define OP_SLTI     0xA
#define OP_ANDI     0xC
#define OP_ORI      0xD
#define OP_NORI     0xE
#define OP_LUI      0xF
#define OP_LB       0x20
#define OP_LH       0x21
#define OP_LW       0x23
#define OP_LBU      0x24
#define OP_LHU      0x25
#define OP_SB       0x28
#define OP_SH       0x29
#define OP_SW       0x2B
#define OP_HALT     0x3F

#define FUNC_SLL    0x0
#define FUNC_SRL    0x2
#define FUNC_SRA    0x3
#define FUNC_JR     0x8
#define FUNC_ADD    0x20
#define FUNC_ADDU   0x21
#define FUNC_SUB    0x22
#define FUNC_AND    0x24
#define FUNC_OR     0x25
#define FUNC_XOR    0x26
#define FUNC_NOR    0x27
#define FUNC_NAND   0x28
#define FUNC_SLT    0x2A

class Instruction {
public:
	
    Instruction(int);

    void Decode();

    int ori;
    int operation;
    char rs, rt, rd;
    int other;

};



#endif //CPU_SIMULATOR_INSTRUCTION_H

