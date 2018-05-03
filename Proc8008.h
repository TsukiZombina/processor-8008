#include <cstdint>
#include <stack>
#include <vector>
#include <functional>
#include <unordered_map>

#define A 0
#define B 1
#define C 2
#define D 3
#define E 4
#define H 5
#define L 6
#define HLT 0xFF

class Proc8008 {
    typedef std::function<unsigned char(Proc8008&, unsigned char)> func;
    
    unsigned char registerFile[7] = {0};
    unsigned char Carry{0}, Parity{0}, Zero{0}, Sign{0};
    unsigned short checkCarry;
    std::stack<unsigned short> stack;
    std::vector<unsigned char> memory;
    std::vector<func> InstructionSet;
    void readProgram();

    unsigned char hlt(unsigned char);
    unsigned char mov(unsigned char);
    unsigned char incDec(unsigned char);
    unsigned char arith(unsigned char);
    unsigned char rotate(unsigned char);
    unsigned char nop(unsigned char);
    unsigned char jmpcallret(unsigned char);

    public:
        Proc8008();
        void execute();
        void print();
};
