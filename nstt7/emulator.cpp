#include <gtest/gtest.h>

#include <memory>
#include <utility>
#include <vector>

/*
*/

namespace Emulator {
using Word = int;

enum class Reg {
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
};

class EmulatorState;

struct Instruction {
    virtual void eval(EmulatorState& state) = 0;
    virtual ~Instruction() = default;
};

class EmulatorState {
public:
    static constexpr size_t REGS = 4;
    static constexpr size_t MEM_SIZE = 1024;

private:
    std::array<Word, REGS> registers_{};
    std::unique_ptr<Word[]> mem_{new Word[MEM_SIZE]};

    size_t pc_ = 0;

public:
    size_t pc() const {
        return pc_;
    }

    Word readReg(Reg reg) const {
        return registers_[std::to_underlying(reg)];
    }

    void writeReg(Reg reg, Word value) {
        registers_[std::to_underlying(reg)] = value;
    }

    Word readMem(Word offset) const {
        if (offset < 0 || static_cast<size_t>(offset) >= MEM_SIZE) {
            throw std::invalid_argument{std::format("illegal address {}", offset)};
        }

        return mem_[offset];
    }

    void writeMem(Word offset, Word value) {
        if (offset < 0 || static_cast<size_t>(offset) >= MEM_SIZE) {
            throw std::invalid_argument{std::format("illegal address {}", offset)};
        }

        mem_[offset] = value;
    }
};

class MovReg final : public Instruction {
    Reg dest_, src_;

public:
    MovReg(Reg dest, Reg src) : dest_{dest}, src_{src} {}

    void eval(EmulatorState& state) override {
        state.writeReg(dest_, state.readReg(src_));
    }
};

class MovImm final : public Instruction {
    Reg dest_;
    Word imm_;

public:
    MovImm(Reg dest, Word imm) : dest_{dest}, imm_{imm} {}

    void eval(EmulatorState& state) override {
        state.writeReg(dest_, imm_);
    }
};

class Program {
    std::vector<std::unique_ptr<Instruction>> instructions_;

public:
};

Program parse(std::string_view program) {
    // TODO: implement it!
    return {};
}

Word emulate(std::string_view program_text) {
    std::vector<std::unique_ptr<Instruction>> program = parse(program_text);

    EmulatorState state;

    while (state.pc() < program.size()) {
        program[state.pc()]->eval(state);
    }

    return state.readReg(Reg::R0);
}
}  // namespace Emulator

int main() {
    std::string_view factorial = R"(
    mov r0 5
    mov r1 1
    mov
    jmpz 6
   
    mul r1 r0
    sub r0 1
    jmp 2

    mov R0 R1
  )";

    Emulator::Word fact5 = Emulator::emulate(factorial);
    EXPECT_EQ(fact5, 120);

    return 0;
}
