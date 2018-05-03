#include <iostream>
#include <fstream>

void writeProgram() {
    std::ofstream os("program.txt", std::ios::binary);
    unsigned char program[] = {0x06, 0x0F, 0x0E, 0xF0, 0xB9, 0x48, 0x00, 0x0C,    0x41, 0x44, 0x00, 0x0C, 0xFF};
    os.write((const char*)program, sizeof(program));
    os.close();
}

int main(int argc, char const* argv[])
{
    writeProgram();
    return 0;
}