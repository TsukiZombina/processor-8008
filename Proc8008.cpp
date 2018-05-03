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
    for (unsigned char i = 0x03; i <= 0x3F; i = i + 0x04) { //Return
        InstructionSet[i] = &Proc8008::jmpcallret;
        i = i + 2;
        InstructionSet[i] = &Proc8008::jmpcallret;
        i = i + 2;
        InstructionSet[i] = &Proc8008::jmpcallret;
    }
    for (unsigned char i = 0x22; i <= 0x3A; i = i + 0x08) { //Nop Void
        InstructionSet[i] = &Proc8008::nop;
    }
    InstructionSet[0x38] = &Proc8008::nop; //Nop Void
    InstructionSet[0x39] = &Proc8008::nop; //Nop Void
    for (unsigned char i = 0x40; i <= 0x7E; i = i + 0x02) { //Jump y Call
        InstructionSet[i] = &Proc8008::jmpcallret;
    }
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
    for (unsigned char i = 0x41; i <= 0x7F; i = i + 0x02) {//Rotate
        InstructionSet[i] = &Proc8008::inout;
    }
    InstructionSet[0x00] = &Proc8008::hlt; //Halt 1
    InstructionSet[0x01] = &Proc8008::hlt; //Halt 2
    InstructionSet[0xFF] = &Proc8008::hlt; //Halt bueno
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
            Carry = (checkCarry & 0x0100) >> 8;
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
            Carry = (checkCarry & 0x0100) >> 8;
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
            Carry = (checkCarry & 0x0100) >> 8;
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
            Carry = (checkCarry & 0x0100) >> 8;
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
            Carry = (checkCarry & 0x0100) >> 8;
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
            Carry = (checkCarry & 0x0100) >> 8;
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
    Carry = (checkCarry & 0x0100) >> 8;
    return 1;
}

unsigned char Proc8008::rotate(unsigned char opcode) {
    unsigned char oldCarry = Carry;
    unsigned char op1 = (opcode & 0x38) >> 3;
    if (op1 == 0x00) {
        registerFile[A] = ((registerFile[A] & 0x80) >> 7) | (registerFile[A] << 1);
    }
    else if (op1 == 0x01) {
        registerFile[A] = ((registerFile[A] & 0x01) << 7) | (registerFile[A] >> 1);
    }
    else if (op1 == 0x10) {
        Carry = registerFile[A] & 0x80;
        registerFile[A] = oldCarry | (registerFile[A] << 1);
    }
    else {
        Carry = registerFile[A] & 0x01;
        registerFile[A] = (oldCarry << 7) | (registerFile[A] >> 1);
    }
    return 1;
}

unsigned char Proc8008::jmpcallret(unsigned char opcode) {
    unsigned short pc = stack.top();
    unsigned char op0 = opcode & 0x07;
    unsigned char op1 = (opcode & 0x38) >> 3;
    unsigned short addr = registerFile[H];
    addr = (addr << 8) | registerFile[L];
    bool jumpOccurs = false;
    if (op0 == 0x00) {
        if ((Carry == 0 && op1 == 0) || (Zero == 0 && op1 == 1) || 
            (Sign == 0 && op1 == 2) || (Parity == 0 && op1 == 3) ||
            (Carry == 1 && op1 == 4) || (Zero == 1 && op1 == 5) ||
            (Sign == 1 && op1 == 6) || (Parity == 1 && op1 == 7)) { 
            stack.top() = memory[pc + 1]; 
            stack.top() = (stack.top() << 8) | memory[pc + 2];
            jumpOccurs = true;
        }
    }
    else if (op0 == 0x02) {
        if ((Carry == 0 && op1 == 0) || (Zero == 0 && op1 == 1) || 
            (Sign == 0 && op1 == 2) || (Parity == 0 && op1 == 3) ||
            (Carry == 1 && op1 == 4) || (Zero == 1 && op1 == 5) ||
            (Sign == 1 && op1 == 6) || (Parity == 1 && op1 == 7)) {
            stack.push(pc); 
            stack.top() = memory[pc + 1]; 
            stack.top() = (stack.top() << 8) | memory[pc + 2];
            jumpOccurs = true;
        }
    }
    else if (op0 == 0x03) {
        if ((Carry == 0 && op1 == 0) || (Zero == 0 && op1 == 1) || 
            (Sign == 0 && op1 == 2) || (Parity == 0 && op1 == 3) ||
            (Carry == 1 && op1 == 4) || (Zero == 1 && op1 == 5) ||
            (Sign == 1 && op1 == 6) || (Parity == 1 && op1 == 7)) {
            stack.pop();
            jumpOccurs = true;
            }
    }
    else if (op0 == 0x04) {
        stack.top() = memory[pc + 1]; 
        stack.top() = (stack.top() << 8) | memory[pc + 2];
        jumpOccurs = true;
    }
    else if (op0 == 0x05) {
        stack.push(pc);
        stack.top() = (0 | op1) << 3;
        jumpOccurs = true;
    }
    else if (op0 == 0x06) {
        stack.push(pc); 
        stack.top() = memory[pc + 1]; 
        stack.top() = (stack.top() << 8) | memory[pc + 2];
        jumpOccurs = true;
    }
    else {
       stack.pop(); 
       jumpOccurs = true;
    }
    return (jumpOccurs) ? 0 : 3;
}

unsigned char Proc8008::inout(unsigned char opcode) {
    short a;
    short MMM = (opcode & 0x0E) >> 1;
    short RR = (opcode & 0x30) >> 4;
    short RRMMM = ((opcode & 0x30) | (opcode & 0x0E)) >> 1;
    if (RR) {
        std::cout << "Port " << RRMMM << ": Register A = " << registerFile[A] << std::endl;
    }
    else {
        std::cout << "Port " << MMM << ": Register A = " << std::endl;
        std::cin >> a;
        registerFile[A] = a;
    }
    return 1;
}

unsigned char Proc8008::nop(unsigned char opcode) {
    std::cout << "This Operation Code is invalid" <<std::endl;
    return 1;
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