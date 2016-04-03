#include <iostream>
#include <fstream>
#include <assert.h>
#include <iomanip>
#include "instruction.h"

using namespace std;

#define MAX_CYCLE   500000
#define MAX_MEMORY  0x400
#define MIN_MEMORY	0x0

#define WORD        4
#define BYTE        8
#define SIGN_BIT    0x80000000
#define IndexToAddr(x) ((x) << 2)

#define StackReg        29
#define ReturnAddrReg   31
#define PCReg           32
#define NextPCReg       33
#define PrevPCReg       34
#define RegNum          35

enum ERROR { WriteRegZero , NumOverflow, AddrOverflow, DataMisaligned };

const char* error_path = (char*)"error_dump.rpt";
const char* snapshot_path = (char*)"snapshot.rpt";
const char* iimage_path = (char*)"iimage.bin";
const char* dimage_path = (char*)"dimage.bin";

fstream snapshot, error;
Instruction instr(0);
int *raw_instr = new int[MAX_MEMORY/WORD];
int PC_init = 0;
int instr_num = 0;
int *raw_data = new int[MAX_MEMORY/WORD];
int data_num = 0;
int registers[RegNum];
int nowPC = 0;
int cycle = 0;

void Initialize();
void loadIimage();
void loadDimage();
void writeReg();

void checkNumOverflow(int, int, int*);
void writeError(int);
void setPC();

int main() {

    Initialize();

    while(instr.operation != OP_HALT && cycle <= MAX_CYCLE){

        bool brk = false, halt = false;

        int shamt = instr.other >> 6;
        int func = instr.other & 0x3f;
        int sum, tmp, value, target;
        unsigned int uint;

        if((instr.operation >= 0x08 && instr.operation <= 0x25) && instr.rt == 0) {
            writeError(WriteRegZero);
            brk = true;
        }

        switch(instr.operation) {
            case R_FORMAT:


                if(func == FUNC_SLL && instr.rt == 0 && instr.rd == 0 && instr.other == 0) {
                    break;
                } else if(func != FUNC_JR && func != FUNC_ADD && func != FUNC_SUB && instr.rd == 0) {
                    writeError(WriteRegZero);
                    break;
                } else if(func != FUNC_JR && instr.rd == 0) {
                    writeError(WriteRegZero);
                    brk = true;
                }

                switch (func) {
                    case FUNC_ADD:
                        checkNumOverflow(registers[instr.rs], registers[instr.rt], &sum);
                        if(!brk)
                            registers[instr.rd] = sum;
                        break;
                    case FUNC_ADDU:
                        registers[instr.rd] = registers[instr.rs] + registers[instr.rt];
                        break;
                    case FUNC_SUB:
                        checkNumOverflow(registers[instr.rs], -registers[instr.rt], &sum);
                        if(!brk)
                            registers[instr.rd] = sum;
                        break;
                    case FUNC_AND:
                        registers[instr.rd] = registers[instr.rs] & registers[instr.rt];
                        break;
                    case FUNC_OR:
                        registers[instr.rd] = registers[instr.rs] | registers[instr.rt];
                        break;
                    case FUNC_XOR:
                        registers[instr.rd] = registers[instr.rs] ^ registers[instr.rt];
                        break;
                    case FUNC_NOR:
                        registers[instr.rd] = ~(registers[instr.rs] | registers[instr.rt]);
                        break;
                    case FUNC_NAND:
                        registers[instr.rd] = ~(registers[instr.rs] & registers[instr.rt]);
                        break;
                    case FUNC_SLT:
                        if (registers[instr.rs] < registers[instr.rt])
                            registers[instr.rd] = 1;
                        else
                            registers[instr.rd] = 0;
                        break;
                    case FUNC_SLL:
                        registers[instr.rd] = registers[instr.rt] << shamt;
                        break;
                    case FUNC_SRL:
                        uint = (unsigned int) registers[instr.rt];
                        uint >>= shamt;
                        registers[instr.rd] = uint;
                        break;
                    case FUNC_SRA:
                        registers[instr.rd] = registers[instr.rt] >> shamt;
                        break;
                    case FUNC_JR:
                        nowPC = registers[instr.rs];
                        break;
                    default:
                        return 0;
                }
                break;

            case OP_ADDI:
                checkNumOverflow(registers[instr.rs], instr.other, &sum);
                if(!brk)
                    registers[instr.rt] = sum;
                break;
            case OP_ADDIU:
                if(!brk)
                    registers[instr.rt] = registers[instr.rs] + instr.other;
                break;
            case OP_LW:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    halt = true;
                }
                if (tmp % WORD != 0x0) {
                    writeError(DataMisaligned);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(halt) {
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = raw_data[tmp / WORD];
                    registers[instr.rt] = value;
                }
                break;
            case OP_LH:
            case OP_LHU:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD/2) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    halt = true;
                }
                if (tmp % WORD != 0 && tmp % WORD != 2) {
                    writeError(DataMisaligned);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(halt) {
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = raw_data[tmp / WORD] << BYTE * (tmp % WORD);
                    if ((value & SIGN_BIT) && (instr.operation == OP_LH))
                        registers[instr.rt] = value >> 2 * BYTE;
                    else {
                        uint = (unsigned int) value;
                        registers[instr.rt] = uint >> 2 * BYTE;
                    }
                }
                break;
            case OP_LB:
            case OP_LBU:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD/4) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = raw_data[tmp / WORD] << BYTE * (tmp % WORD);
                    if ((value & SIGN_BIT) && (instr.operation == OP_LB))
                        registers[instr.rt] = value >> 3 * BYTE;
                    else {
                        uint = (unsigned int) value;
                        registers[instr.rt] = uint >> 3 * BYTE;
                    }
                }
                break;
            case OP_SW:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    halt = true;
                }
                if (tmp % WORD != 0x0) {
                    writeError(DataMisaligned);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(halt) {
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = registers[instr.rt];
                    raw_data[tmp / WORD] = value;
                }
                break;
            case OP_SH:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD/2) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    halt = true;
                }
                if (tmp % WORD != 0 && tmp % WORD != 2) {
                    writeError(DataMisaligned);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(halt) {
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = registers[instr.rt];
                    target = raw_data[tmp / WORD];
                    if (tmp % WORD == 0)
                        target = (value & 0xffff) << 2 * BYTE | (target & 0xffff);
                    if (tmp % WORD == 2)
                        target = (value & 0xffff) | (target & 0xffff0000);
                    raw_data[tmp / WORD] = target;
                }
                break;
            case OP_SB:
                checkNumOverflow(registers[instr.rs], instr.other, &tmp);
                if (tmp >= MAX_MEMORY || (tmp + WORD/4) > MAX_MEMORY || tmp < MIN_MEMORY) {
                    writeError(AddrOverflow);
                    cout << "Simulation succeed." << endl;
                    return 0;
                }
                if(!brk) {
                    value = registers[instr.rt];
                    target = raw_data[tmp / WORD];
                    if (tmp % WORD == 0)
                        target &= 0x00ffffff;
                    else if (tmp % WORD == 1)
                        target &= 0xff00ffff;
                    else if (tmp % WORD == 2)
                        target &= 0xffff00ff;
                    else
                        target &= 0xffffff00;
                    target |= (value & 0xff) << (3 - tmp % WORD) * BYTE;
                    raw_data[tmp / WORD] = target;
                }
                break;
            case OP_LUI:
                if(!brk)
                    registers[instr.rt] = instr.other << 16;
                break;
            case OP_ANDI:
                if(!brk)
                    registers[instr.rt] = registers[instr.rs] & (instr.other & 0xffff);
                break;
            case OP_ORI:
                if(!brk)
                    registers[instr.rt] = registers[instr.rs] | (instr.other & 0xffff);
                break;
            case OP_NORI:
                if(!brk)
                    registers[instr.rt] = ~(registers[instr.rs] | (instr.other & 0xffff));
                break;
            case OP_SLTI:
                if(!brk) {
                    if (registers[instr.rs] < instr.other)
                        registers[instr.rt] = 1;
                    else
                        registers[instr.rt] = 0;
                }
                break;
            case OP_BEQ:
                checkNumOverflow(nowPC, IndexToAddr(instr.other), &sum);
                if (registers[instr.rs] == registers[instr.rt])
                    nowPC = sum;
                break;
            case OP_BNE:
                checkNumOverflow(nowPC, IndexToAddr(instr.other), &sum);
                if (registers[instr.rs] != registers[instr.rt])
                    nowPC = sum;
                break;
            case OP_BGTZ:
                checkNumOverflow(nowPC, IndexToAddr(instr.other), &sum);
                if (registers[instr.rs] > 0)
                    nowPC = sum;
                break;

            case OP_J:
                nowPC = (nowPC & 0xf0000000) | IndexToAddr(instr.other);
                break;
            case OP_JAL:
                registers[ReturnAddrReg] = nowPC;
                nowPC = (nowPC & 0xf0000000) | IndexToAddr(instr.other);
                break;
            case OP_HALT:
            default:
                return 0;
        }

        setPC();

    }

    cout << "Simulation succeed." << endl;
    return 0;
}


void Initialize() {



    loadIimage();
    loadDimage();
    error.open(error_path, ios::out);
    error.close();

    instr = Instruction(raw_instr[0]);
    snapshot.open(snapshot_path, ios::out);
    snapshot.close();
    nowPC = registers[NextPCReg];
    writeReg();
}

void loadIimage() {
    fstream iimage;
    iimage.open(iimage_path, ios::binary | ios::in);
    assert(iimage);

    for(int i = 0; i < 2; i++) {
        unsigned char tmp[4];
        iimage.read((char*)tmp, WORD);
        unsigned int num = (unsigned int)tmp[0]<<24 | (unsigned int)tmp[1]<<16 | (unsigned int)tmp[2]<<8 | (unsigned int)tmp[3];
        if(i == 0) {
            registers[PCReg] = num;
            registers[PrevPCReg] = registers[PCReg] - WORD;
            registers[NextPCReg] = registers[PCReg] + WORD;
            PC_init = num;
        } else {
            instr_num = num;
        }
    }

    for(int i = 0; i < instr_num; i++) {
        unsigned char tmp[4];
        iimage.read((char*)tmp, WORD);
        int num = (int)tmp[0] << 24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        raw_instr[i] = num;
    }

    iimage.close();

}

void loadDimage() {
    fstream dimage;
    dimage.open(dimage_path, ios::binary | ios::in);
    assert(dimage);

    for(int i = 0; i < 2; i++) {
        unsigned char tmp[4];
        dimage.read((char*)tmp, WORD);
        unsigned int num = (unsigned int)tmp[0]<<24 | (unsigned int)tmp[1]<<16 | (unsigned int)tmp[2]<<8 | (unsigned int)tmp[3];
        if(i == 0) {
            registers[StackReg] = num;
        } else {
            data_num = num;
        }
    }

    for(int i = 0; i < data_num; i++) {
        unsigned char tmp[4];
        dimage.read((char*)tmp, WORD);
        int num = (int)tmp[0] << 24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        raw_data[i] = num;
    }

    dimage.close();
}

void writeReg() {
    snapshot.open(snapshot_path, ios::out | ios::app);

    snapshot << "cycle " << dec << cycle << endl;
    for(int i = 0; i < PCReg; i++) {
        snapshot << "$" << setw(2) << setfill('0') << dec << i << ": 0x" << setw(8) << setfill('0') << hex << uppercase << registers[i] << endl;
    }
    snapshot << "PC: 0x" << setw(8) << setfill('0') << hex << uppercase << registers[PCReg] << "\n\n\n";
    snapshot.close();
    cycle++;
}

void checkNumOverflow(int a, int b, int *sum) {
    *sum = a + b;
    if (!((a ^ b) & SIGN_BIT) && ((a ^ *sum) & SIGN_BIT)) {
        writeError(NumOverflow);
    }
}

void writeError(int type) {
    error.open(error_path, ios::out | ios::app);

    error << "In cycle " << cycle << ": ";
    switch(type) {
        case WriteRegZero:
            error << "Write $0 Error" << endl;
            break;
        case NumOverflow:
            error << "Number Overflow" << endl;
            break;
        case AddrOverflow:
            error << "Address Overflow" << endl;
            break;
        case DataMisaligned:
            error << "Misalignment Error" << endl;
            break;
        default:
            error << "Something Wrong" << endl;
    }
    error.close();
}

void setPC() {
    registers[PrevPCReg] = registers[PCReg];
    registers[PCReg] = nowPC;
    registers[NextPCReg] = nowPC + WORD;
    nowPC = registers[NextPCReg];

    int index = (registers[PCReg] - PC_init)/WORD;
    if(index > instr_num)
        assert(false);
    if(index < 0)
        instr = Instruction(0);
    else
        instr = Instruction(raw_instr[index]);
    writeReg();
}

