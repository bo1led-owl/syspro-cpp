#pragma once

#include <cstdint>
#include <istream>

namespace Emulator {
using Word = uint16_t;

Word emulate(std::istream& binary, std::ostream& disassembly);
}  // namespace Emulator
