#include "Proc8008.h"
#include <iostream>

int main(int argc, char const* argv[])
{
    Proc8008 proc;
    proc.print();
    std::cout << std::endl;
    proc.execute();
    //proc.print();
    return 0;
}