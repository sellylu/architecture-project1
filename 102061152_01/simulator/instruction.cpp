//
// Created by selly on 2016/3/23.
//

#include "instruction.h"


Instruction::Instruction(int in) {
    ori = in;
    Decode();
}

void Instruction::Decode() {

    operation = (ori >> 26) & 0x3f;
    rs = (ori >> 21) & 0x1f;
    rt = (ori >> 16) & 0x1f;
    rd = (ori >> 11) & 0x1f;

    if(operation == R_FORMAT) {
        other = ori & 0x7ff;
    } else if (operation == OP_J || operation == OP_JAL) {
        other = ori & 0x3ffffff;
    } else {
        other = ori & 0xffff;
        if(other & 0x8000) {
            other |= 0xffff0000;
        }
    }


}
