#include "Proc8008.h"
#include <fstream>
#include <iostream>
#include <climits>

Proc8008::Proc8008() : memory(1024), InstructionSet(256) {
    std::ifstream is("program.txt", std::ios::binary | std::ios::ate);
    auto size = is.tellg();
    is.seekg(0, std::ios::beg);
    is.read((char*)memory.data(), size);
    stack.push(0); //initializes pc
    for (unsigned char i = 0xC0; i <= 0xFE; ++i) { //Load to register/memory
        InstructionSet[i] = &Proc8008::mov;
    }
    for (unsigned char i = 0x06; i <= 0x3E; i = i + 0x08) {//Load immediate
        InstructionSet[i] = &Proc8008::mov;
    }
    for (unsigned char i = 0x08; i <=0x31; i = i + 0x07) {//Inc & Dec
        InstructionSet[i] = &Proc8008::incDec;
        i++;
        InstructionSet[i] = &Proc8008::incDec;
    }
    for (unsigned char i = 0x80; i <= 0xBF; i++) {//Arith with register/memory
        InstructionSet[i] = &Proc8008::arith;
    }
    for (unsigned char i = 0x04; i <= 0x3C; i = i + 0x08) {//Arith immediate
        InstructionSet[i] = &Proc8008::arith;
    }
    for (unsigned char i = 0x02; i <= 0x1A; i = i + 0x08) {//Rotate
        InstructionSet[i] = &Proc8008::rotate;
    }
    InstructionSet[0xFF] = &Proc8008::hlt;
}

unsigned char Proc8008::hlt(unsigned char opcode) {
    std::cout << "Program finished\n"; 
    return 0;
}

unsigned char Proc8008::mov(unsigned char opcode) {
    unsigned char op0 = opcode & 0x07;
    unsigned char op1 = (opcode & 0x38) >> 3;
    unsigned short addr = registerFile[H];
    addr = (addr << 8) | registerFile[L];
    if (op0 == 0x07) {
        registerFile[op1] = memory[addr];
    }
    else if (op1 == 0x07) {
        memory[addr] = registerFile[op0];
    }
    else if (opcode >= 0x06 && opcode <= 0x36) {
        registerFile[op1] = memory[stack.top()+1];
        return 2;
    }
    else if (opcode == 0x3E) {
        memory[addr] = memory[stack.top()+1];
    }
    else {
        registerFile[op1] = registerFile[op0];
    }
    return 1;
}

unsigned char Proc8008::incDec(unsigned char opcode) {
    unsigned char op0 = opcode & 0x07;
    unsigned char op1 = (opcode & 0x38) >> 3;
    if (op0) {
        registerFile[op1] = registerFile[op1]--;
    }
    else {
        registerFile[op1] = registerFile[op1]++; 
    }
    Sign = registerFile[op1] >> 7;
    Zero = 0;
    if (registerFile[op1] == 0x00) {
        Zero = 1;
    }
    Parity = ~(registerFile[A] | 254); 
    return 1;
}

unsigned char Proc8008::arith(unsigned char opcode) {
    unsigned char op0 = opcode & 0x07;
    unsigned char op1 = (opcode & 0x38) >> 3;
    unsigned short addr = registerFile[H];
    addr = (addr << 8) | registerFile[L];
    if (op1 == 0x00) {
        if (op0 == 0x07) {
            checkCarry = registerFile[A] + memory[addr];
            registerFile[A] = checkCarry;
        }
        else if (op0 == 0x04) { 
            checkCarry = registerFile[A] + memory[stack.top() + 1];
            registerFile[A] = checkCarry;
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            checkCarry = registerFile[A] + registerFile[op0];
            registerFile[A] = checkCarry;
        }
    }
    else if (op1 == 0x01 || op1 == 0x03) {
        if (op0 == 0x07) {
            checkCarry = Carry + memory[addr];
            if (op1 == 0x01) {
                registerFile[A] = checkCarry;
            }
            if (op1 == 0x03) {
                checkCarry = registerFile[A] - checkCarry;
                registerFile[A] = checkCarry;
            }
        }
        else if (op0 == 0x04) {
            checkCarry = Carry + memory[stack.top() + 1];
            if (op1 == 0x01) {
                registerFile[A] = checkCarry;
            }
            if (op1 == 0x03) {
                checkCarry = registerFile[A] - checkCarry;
                registerFile[A] = checkCarry;
            }
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            checkCarry = Carry + registerFile[op0];
            if (op1 == 0x01) {
                registerFile[A] = checkCarry;
            }
            if (op1 == 0x03) {
                checkCarry = registerFile[A] - checkCarry;
                registerFile[A] = checkCarry;
            }
        }
    }
    else if (op1 == 0x02 || op1 == 0x07) {
        if (op0 == 0x07) {
            checkCarry = registerFile[A] - memory[addr];
            registerFile[A] = checkCarry;
        }
        else if (op0 == 0x04) {
            checkCarry = registerFile[A] - memory[stack.top() + 1];
            registerFile[A] = checkCarry;
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            checkCarry = registerFile[A] - registerFile[op0];
            registerFile[A] = checkCarry;
        }
    }
    else if (op1 == 0x04) {
        if (op0 == 0x07) {
            registerFile[A] &= memory[addr];
        }
        else if (op0 == 0x04) {
            registerFile[A] &= memory[stack.top() + 1];
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            registerFile[A] &= registerFile[op0];
        }
    }
    else if (op1 == 0x05) {
        if (op0 == 0x07) {
            registerFile[A] ^= memory[addr];
        }
        else if (op0 == 0x04) {
            registerFile[A] ^= memory[stack.top() + 1];
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            registerFile[A] = registerFile[A] ^ registerFile[op0];
        }
    }
    else if (op1 == 0x06) {
        if (op0 == 0x07) {
            registerFile[A] |= memory[addr];
        }
        else if (op0 == 0x04) {
            registerFile[A] |= memory[stack.top() + 1];
            Sign = registerFile[A] >> 7;
            Zero = 0;
            if (registerFile[A] == 0x00) {
                Zero = 1;
            }
            Parity = ~(registerFile[A] | 254);
            Carry = checkCarry >> 8;
            return 2;
        }
        else {
            registerFile[A] |= registerFile[op0];
        }
    }
    Sign = registerFile[A] >> 7;
    Zero = 0;
    if (registerFile[A] == 0x00) {
        Zero = 1;
    }
    Parity = ~(registerFile[A] | 254);
    Carry = checkCarry >> 8;
    return 1;
}

unsigned char Proc8008::rotate(unsigned char opcode) {
    unsigned char oldCarry = Carry;
    unsigned char op1 = (opcode & 0x38) >> 3;
    if (op1 == 0x00) {
        Carry = registerFile[A] & 0x80;
        registerFile[A] = oldCarry | (registerFile[A] << 1);
    }
    else if (op1 == 0x01) {
        Carry = registerFile[A] & 0x01;
        registerFile[A] = (oldCarry << 7) | (registerFile[A] >> 1);
    }
    else if (op1 == 0x10) {
        
    }
    else {
        
    }
    Carry = checkCarry >> 8;
    return 1;
}

unsigned char nop(unsigned char opcode) {
    std::cout << "This Operation Code is invalid" <<std::endl;
}

void Proc8008::execute() {
    do {
        std::cout << "opcode: " << int(memory[stack.top()]) << '\n';
        unsigned char inc = InstructionSet[memory[stack.top()]](*this, memory[stack.top()]);
        stack.top() += inc;
        print();
        std::cout << std::endl;
    } while(memory[stack.top()] != HLT);
    InstructionSet[memory[stack.top()]](*this, memory[stack.top()]);
}

void Proc8008::print() {
    for (unsigned char i = 0; i < 5; ++i) {
        char registerName = 'A';
        std::cout << (char)(registerName + i) << ":" << int(registerFile[i]) << '\n';
    }
    std::cout << "H: " << int(registerFile[H]) << '\n'
            << "L: " << int(registerFile[L]) << '\n' 
            << "PC: "<< int(stack.top()) << '\n'
            << "Carry: " << int(Carry) << '\n'
            << "Zero: " << int(Zero) << '\n'
            << "Parity: " << int(Parity) << '\n'
            << "Sign: " << int(Sign) << std::endl;
}