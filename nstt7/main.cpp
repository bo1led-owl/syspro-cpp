#include <fstream>
#include <iostream>
#include <print>

#include "emulator.hpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::println(stderr, "Not enough arguments");
    }

    std::ifstream binary(argv[1], std::ios::binary);

    std::cout << Emulator::emulate(binary, std::cout) << '\n';
}
