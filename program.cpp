#include <iostream>
#include <fstream>

void writeProgram() {
    std::ofstream os("program.txt", std::ios::binary);
    unsigned char program[] = {0x04, 0xFF, 0x0E, 0xFF, 0x81, 0xA1, 0x0A, 0x3F, 0xFF};
    os.write((const char*)program, sizeof(program));
    os.close();
}

int main(int argc, char const* argv[])
{
    writeProgram();
    return 0;
}