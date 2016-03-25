#include <iostream>
#include <fstream>
#include <assert.h>
#include <iomanip>
#include "instruction.h"

using namespace std;

#define MAX_CYCLE   500000

#define WORD        4
#define BYTE        8
#define SIGN_BIT    0x80000000
#define IndexToAddr(x) ((x) << 2)

#define StackReg        29
#define ReturnAddrReg   31
#define PCReg           33
#define NextPCReg       34
#define PrevPCReg       35
#define RegNum          36

int *raw_instr;
int PC_init;
int instr_num;
int *raw_data = new int[0x400/WORD];
int data_num;
int registers[RegNum];


void loadIimage();
void loadDimage();
fstream snapshot, error;
void writeState(fstream *);
void writeError();

int main() {

    loadIimage();
    loadDimage();
    Instruction instr(raw_instr[0]);
    snapshot.open("../testcase/snapshot.rpt", ios::out);
    int cycle = 0;

    while(instr.operation != OP_HALT && cycle <= MAX_CYCLE){

        snapshot << "cycle " << dec << cycle << endl;
        writeState(&snapshot);
        cycle++;

        int nowPC = registers[NextPCReg];
        int sum, tmp, value, target;
        unsigned int uint;

        switch(instr.operation) {
            case R_FORMAT:

                switch (instr.other & 0x3f) {
                    case FUNC_ADD:
                        sum = registers[instr.rs] + registers[instr.rt];
                        //if (!((registers[instr.rs] ^ registers[instr.rt]) & SIGN_BIT) && ((registers[instr.rs] ^ sum) & SIGN_BIT)) {
                        //RaiseException(OverflowException, 0);
                        //continue;
                        //}
                        registers[instr.rd] = sum;
                        break;
                    case FUNC_ADDU:
                        registers[instr.rd] = registers[instr.rs] + registers[instr.rt];
                        break;
                    case FUNC_SUB:
                        sum = registers[instr.rs] - registers[instr.rt];
                        if (!((registers[instr.rs] ^ registers[instr.rt]) & SIGN_BIT) &&
                            ((registers[instr.rs] ^ sum) & SIGN_BIT)) {
                            //RaiseException(OverflowException, 0);
                            continue;
                        }
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
                        registers[instr.rd] = registers[instr.rt] << instr.other;
                        break;
                    case FUNC_SRL:
                        uint = (unsigned int)registers[instr.rt];
                        uint >>= instr.other;
                        registers[instr.rd] = uint;
                        break;
                    case FUNC_SRA:
                        registers[instr.rd] = registers[instr.rt] >> instr.other;
                        break;
                    case FUNC_JR:
                        nowPC = registers[instr.rs];
                        break;
                    default:
                        assert(false);
                }
                break;
            case OP_ADDI:
                sum = registers[instr.rs] + instr.other;
                //if (!((registers[instr.rs] ^ instr.other) & SIGN_BIT) && ((instr.other ^ sum) & SIGN_BIT)) {
                //RaiseException(OverflowException, 0);
                //continue;
                //}
                registers[instr.rt] = sum;
                break;
            case OP_ADDIU:
                registers[instr.rt] = registers[instr.rs] + instr.other;
                break;
            case OP_LW:
                tmp = registers[instr.rs] + instr.other;
                if (tmp & 0x3) {
                    //RaiseException(AddressErrorException, tmp);
                    continue;
                }
                value = raw_data[tmp / WORD];
                registers[instr.rt] = value;
                break;
            case OP_LH:
            case OP_LHU:
                tmp = registers[instr.rs] + instr.other;
                if (tmp & 0x1) {
                    //RaiseException(AddressErrorException, tmp);
                    continue;
                }
                value = raw_data[tmp / WORD] << BYTE*(tmp%WORD);
                if ((value & SIGN_BIT) && (instr.operation == OP_LH))
                    registers[instr.rt] = value >> 2*BYTE;
                else {
                    uint = (unsigned int)value;
                    registers[instr.rt] = uint >> 2*BYTE;
                }
                break;
            case OP_LB:
            case OP_LBU:
                tmp = registers[instr.rs] + instr.other;
                value = raw_data[tmp/WORD] << BYTE*(tmp%WORD);
                if ((value & SIGN_BIT) && (instr.operation == OP_LB))
                    registers[instr.rt] = value >> 3*BYTE;
                else {
                    uint = (unsigned int)value;
                    registers[instr.rt] = uint >> 3*BYTE;
                }
                break;
            case OP_SW:
                tmp = registers[instr.rs] + instr.other;
                value = registers[instr.rt];
                raw_data[tmp/WORD] = value;
                break;
            case OP_SH:
                tmp = registers[instr.rs] + instr.other;
                value = registers[instr.rt];
                target = raw_data[tmp/WORD];
                target = (value & 0xffff) << 2*BYTE | (target & 0xffff);
                raw_data[tmp/WORD] = target;
                break;
            case OP_SB:
                tmp = registers[instr.rs] + instr.other;
                value = registers[instr.rt];
                target = raw_data[tmp/WORD];
                if(tmp%WORD == 0)
                    target &= 0xffffff;
                else if(tmp%WORD == 1)
                    target &= 0xff00ffff;
                else if(tmp%WORD == 2)
                    target &= 0xffff00ff;
                else
                    target &= 0xffffff00;
                target |= (value & 0xff) << (3-tmp%WORD)*BYTE;
                raw_data[tmp/WORD] = target;
                break;
            case OP_LUI:
                registers[instr.rt] = instr.other << 16;
                break;
            case OP_ANDI:
                registers[instr.rt] = registers[instr.rs] & (instr.other & 0xffff);
                break;
            case OP_ORI:
                registers[instr.rt] = registers[instr.rs] | (instr.other & 0xffff);
                break;
            case OP_NORI:
                registers[instr.rt] = ~(registers[instr.rs] & (instr.other & 0xffff));
                break;
            case OP_SLTI:
                if (registers[instr.rs] < instr.other)
                    registers[instr.rt] = 1;
                else
                    registers[instr.rt] = 0;
                break;
            case OP_BEQ:
                if (registers[instr.rs] == registers[instr.rt])
                    nowPC += IndexToAddr(instr.other);
                break;
            case OP_BNE:
                if (registers[instr.rs] != registers[instr.rt])
                    nowPC += IndexToAddr(instr.other);
                break;
            case OP_BGTZ:
                if (registers[instr.rs] > 0)
                    nowPC += IndexToAddr(instr.other);
                break;

            case OP_J:
                nowPC = (nowPC & 0xf0000000) | IndexToAddr(instr.other);
                break;
            case OP_JAL:
                registers[ReturnAddrReg] = nowPC;
                nowPC = (nowPC & 0xf0000000) | IndexToAddr(instr.other);
                break;
            case OP_HALT:
                break;
            default:
                assert(false);
        }

        registers[PrevPCReg] = registers[PCReg];
        registers[PCReg] = nowPC;
        registers[NextPCReg] = nowPC + WORD;

        int index = (registers[PCReg] - PC_init)/WORD;
        if(index > instr_num)
            assert(false);
        Instruction buf_inst(raw_instr[index]);
        instr = buf_inst;

    }

    snapshot << "cycle " << dec << cycle << endl;
    writeState(&snapshot);
    snapshot.close();

    cout << "Simulation succeed." << endl;
    return 0;
}

void loadIimage() {
    fstream iimage;
    iimage.open("../testcase/iimage.bin", ios::binary | ios::in);
    assert(iimage);

    for(int i = 0; i < 2; i++) {
        char tmp[4];
        iimage.read(tmp, WORD);
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

    raw_instr = new int[instr_num];

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
    dimage.open("../testcase/dimage.bin", ios::binary | ios::in);
    assert(dimage);

    for(int i = 0; i < 2; i++) {
        char tmp[4];
        dimage.read(tmp, WORD);
        unsigned int num = (unsigned int)tmp[0]<<24 | (unsigned int)tmp[1]<<16 | (unsigned int)tmp[2]<<8 | (unsigned int)tmp[3];
        if(i == 0) {
            registers[StackReg] = num;
        } else {
            data_num = num;
        }
    }

    //raw_data = new unsigned int[data_num];

    for(int i = 0; i < data_num; i++) {
        unsigned char tmp[4];
        dimage.read((char*)tmp, WORD);
        int num = (int)tmp[0] << 24 | (int)tmp[1]<<16 | (int)tmp[2]<<8 | (int)tmp[3];
        raw_data[i] = num;
    }

    dimage.close();
}

void writeState(fstream *snapshot) {
    for(int i = 0; i < PCReg-1; i++) {
        *snapshot << "$" << setw(2) << setfill('0') << dec << i << ": 0x" << setw(8) << setfill('0') << hex << uppercase << registers[i] << endl;
    }
    *snapshot << "PC: 0x" << setw(8) << setfill('0') << hex << uppercase << registers[PCReg] << "\n\n\n";
}

void writeError() {
	error.open("../testcase/error_dump.rpt", ios::out);
	error.close();
}
