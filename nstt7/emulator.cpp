#include "emulator.hpp"

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

namespace Emulator {
using EncodedInstruction = uint16_t;
constexpr unsigned INSTRUCTION_BITS = sizeof(EncodedInstruction) * 8;
constexpr unsigned REG_ENCODING_BITS = 3;

enum class Reg : uint8_t {
    RZ = 0b000,
    R1 = 0b001,
    R2 = 0b010,
    R3 = 0b011,
    R4 = 0b100,
    R5 = 0b101,
    R6 = 0b110,
    R7 = 0b111,
};

std::ostream& operator<<(std::ostream& os, Reg reg) {
    switch (reg) {
        case Reg::RZ:
            return os << "rz";
        case Reg::R1:
            return os << "r1";
        case Reg::R2:
            return os << "r2";
        case Reg::R3:
            return os << "r3";
        case Reg::R4:
            return os << "r4";
        case Reg::R5:
            return os << "r5";
        case Reg::R6:
            return os << "r6";
        case Reg::R7:
            return os << "r7";
    }
}

class EmulatorState;

struct Instruction {
    virtual void eval(EmulatorState& state) const = 0;
    virtual ~Instruction() = default;
    virtual void print(std::ostream& os) const = 0;
};

class EmulatorState {
public:
    static constexpr size_t REGS = 8;
    static constexpr size_t MEM_SIZE = 65536;

private:
    std::array<Word, REGS> registers_{};
    std::unique_ptr<Word[]> mem_{new Word[MEM_SIZE]};

    Word pc_ = 0;
    bool running_ = true;

public:
    void halt() {
        running_ = false;
    }

    bool running() const {
        return running_;
    }

    size_t pc() const {
        return pc_;
    }

    void incPc() {
        pc_ += 1;
    }

    void setPc(Word value) {
        pc_ = value;
    }

    Word readReg(Reg reg) const {
        if (reg == Reg::RZ)
            return 0;
        return registers_[std::to_underlying(reg)];
    }

    void writeReg(Reg reg, Word value) {
        if (reg == Reg::RZ)
            return;
        registers_[std::to_underlying(reg)] = value;
    }

    Word readMem(Word offset) const {
        return mem_[offset];
    }

    void writeMem(Word offset, Word value) {
        mem_[offset] = value;
    }
};

class Halt final : public Instruction {
public:
    void eval(EmulatorState& state) const override {
        state.halt();
    }

    void print(std::ostream& os) const override {
        os << "hlt";
    }
};

class MovReg final : public Instruction {
    Reg dest_, src_;

public:
    MovReg(Reg dest, Reg src) : dest_{dest}, src_{src} {}

    void eval(EmulatorState& state) const override {
        state.writeReg(dest_, state.readReg(src_));
        state.incPc();
    }

    void print(std::ostream& os) const override {
        os << "mov " << dest_ << ", " << src_;
    }
};

class MovImm final : public Instruction {
    Reg dest_;
    Word imm_;

public:
    MovImm(Reg dest, Word imm) : dest_{dest}, imm_{imm} {}

    void eval(EmulatorState& state) const override {
        state.writeReg(dest_, imm_);
        state.incPc();
    }

    void print(std::ostream& os) const override {
        os << "mov " << dest_ << ", " << imm_;
    }
};

class BinaryInstrReg : public Instruction {
protected:
    Reg lhs_;
    Reg rhs_;

    virtual Word op(Word lhs, Word rhs) const = 0;

public:
    BinaryInstrReg(Reg lhs, Reg rhs) : lhs_{lhs}, rhs_{rhs} {}

    void eval(EmulatorState& state) const final override {
        state.writeReg(lhs_, op(state.readReg(lhs_), state.readReg(rhs_)));
        state.incPc();
    }
};

class BinaryInstrImm : public Instruction {
protected:
    Reg lhs_;
    Word rhs_;

    virtual Word op(Word lhs, Word rhs) const = 0;

public:
    BinaryInstrImm(Reg lhs, Word rhs) : lhs_{lhs}, rhs_{rhs} {}

    void eval(EmulatorState& state) const final override {
        state.writeReg(lhs_, op(state.readReg(lhs_), rhs_));
        state.incPc();
    }
};

#define STR(X) #X

#define MAKE_BINARY_INSTR_REG(NAME, OP, MNEMONIC)            \
    class NAME final : public BinaryInstrReg {               \
    protected:                                               \
        Word op(Word lhs, Word rhs) const override {         \
            return lhs OP rhs;                               \
        }                                                    \
                                                             \
    public:                                                  \
        NAME(Reg lhs, Reg rhs) : BinaryInstrReg(lhs, rhs) {} \
                                                             \
        void print(std::ostream& os) const override {        \
            os << STR(MNEMONIC) " " << lhs_ << ", " << rhs_; \
        }                                                    \
    }

#define MAKE_BINARY_INSTR_IMM(NAME, OP, MNEMONIC)             \
    class NAME final : public BinaryInstrImm {                \
    protected:                                                \
        Word op(Word lhs, Word rhs) const override {          \
            return lhs OP rhs;                                \
        }                                                     \
                                                              \
    public:                                                   \
        NAME(Reg lhs, Word rhs) : BinaryInstrImm(lhs, rhs) {} \
                                                              \
        void print(std::ostream& os) const override {         \
            os << STR(MNEMONIC) " " << lhs_ << ", " << rhs_;  \
        }                                                     \
    }

MAKE_BINARY_INSTR_REG(ShlReg, <<, shl);
MAKE_BINARY_INSTR_IMM(ShlImm, <<, shl);
MAKE_BINARY_INSTR_REG(ShrReg, >>, shr);
MAKE_BINARY_INSTR_IMM(ShrImm, >>, shr);
MAKE_BINARY_INSTR_REG(AddReg, +, add);
MAKE_BINARY_INSTR_IMM(AddImm, +, add);
MAKE_BINARY_INSTR_REG(SubReg, -, sub);
MAKE_BINARY_INSTR_IMM(SubImm, -, sub);
MAKE_BINARY_INSTR_REG(MulReg, *, mul);
MAKE_BINARY_INSTR_IMM(MulImm, *, mul);
MAKE_BINARY_INSTR_REG(DivReg, /, div);
MAKE_BINARY_INSTR_IMM(DivImm, /, div);

class Load final : public Instruction {
    Reg dest_;
    Reg src_;
    Reg offset_;

public:
    Load(Reg dest, Reg src, Reg offset) : dest_{dest}, src_{src}, offset_{offset} {}

    void eval(EmulatorState& state) const override {
        state.writeReg(dest_, state.readMem(state.readReg(src_) + state.readReg(offset_)));
        state.incPc();
    }

    void print(std::ostream& os) const override {
        os << "ld " << dest_ << ", [" << src_ << '+' << offset_ << ']';
    }
};

class Store final : public Instruction {
    Reg dest_;
    Reg offset_;
    Reg src_;

public:
    Store(Reg dest, Reg offset, Reg src) : dest_{dest}, offset_{offset}, src_{src} {}

    void eval(EmulatorState& state) const override {
        state.writeMem(state.readReg(dest_) + state.readReg(offset_), state.readReg(src_));
        state.incPc();
    }

    void print(std::ostream& os) const override {
        os << "st [" << dest_ << '+' << offset_ << "], " << src_;
    }
};

template <std::integral T, size_t BITS>
T signExtend(T x) {
    bool negative = (x >> (BITS - 1)) & 1;
    if (negative) {
        return (static_cast<T>(~0) << BITS) | x;
    }

    return x;
}

class Jmp final : public Instruction {
    Word offset_;

public:
    Jmp(Word offset) : offset_{offset} {}

    void eval(EmulatorState& state) const override {
        state.setPc(state.pc() + offset_);
    }

    void print(std::ostream& os) const override {
        os << "jmp " << offset_;
    }
};

class BranchInstr : public Instruction {
protected:
    Reg reg_;
    Word offset_;

    virtual bool cond(Word value) const = 0;

public:
    BranchInstr(Reg reg, Word offset) : reg_{reg}, offset_{offset} {}

    void eval(EmulatorState& state) const final override {
        if (cond(state.readReg(reg_))) {
            state.setPc(state.pc() + offset_);
        } else {
            state.incPc();
        }
    }
};

#define MAKE_BRANCH_INSTR(NAME, OP, MNEMONIC)                    \
    class NAME final : public BranchInstr {                      \
    protected:                                                   \
        bool cond(Word value) const override {                   \
            return value OP 0;                                   \
        }                                                        \
                                                                 \
    public:                                                      \
        NAME(Reg reg, Word offset) : BranchInstr(reg, offset) {} \
                                                                 \
        void print(std::ostream& os) const override {            \
            os << STR(MNEMONIC) " " << reg_ << ", " << offset_;  \
        }                                                        \
    }

MAKE_BRANCH_INSTR(Blt, <, blt);
MAKE_BRANCH_INSTR(Ble, <=, ble);
MAKE_BRANCH_INSTR(Bgt, >, bgt);
MAKE_BRANCH_INSTR(Bge, >=, bge);
MAKE_BRANCH_INSTR(Beq, ==, beq);
MAKE_BRANCH_INSTR(Bne, !=, bne);

EncodedInstruction extractBits(EncodedInstruction instr, unsigned offset, unsigned size) {
    return (instr >> (INSTRUCTION_BITS - offset - size)) &
           ((static_cast<EncodedInstruction>(~0)) >> (INSTRUCTION_BITS - size));
}

Reg extractReg(EncodedInstruction instr, unsigned offset) {
    return static_cast<Reg>(extractBits(instr, offset, REG_ENCODING_BITS));
}

std::unique_ptr<Instruction> decodeInstr(EncodedInstruction instr) {
    if (extractBits(instr, 0, 1) == 1) {
        // mov rx imm
        return std::make_unique<MovImm>(extractReg(instr, 1),
                                        extractBits(instr, 4, INSTRUCTION_BITS - 4));
    }

    if (extractBits(instr, 1, 1) == 1) {
        // jumps

        switch (extractBits(instr, 2, 3)) {
            case 0b000:
                return std::make_unique<Blt>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b001:
                return std::make_unique<Ble>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b010:
                return std::make_unique<Bgt>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b011:
                return std::make_unique<Bge>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b100:
                return std::make_unique<Beq>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b101:
                return std::make_unique<Bne>(extractReg(instr, 5),
                                             signExtend<Word, 8>(extractBits(instr, 8, 8)));
            case 0b111:
                return std::make_unique<Jmp>(signExtend<Word, 11>(extractBits(instr, 5, 11)));
        }
    }

    if (extractBits(instr, 2, 1) == 1) {
        // arithmetic with immediate second operand

        switch (extractBits(instr, 3, 2)) {
            case 0b00:
                return std::make_unique<AddImm>(extractReg(instr, 5), extractBits(instr, 8, 8));
            case 0b01:
                return std::make_unique<SubImm>(extractReg(instr, 5), extractBits(instr, 8, 8));
            case 0b10:
                return std::make_unique<MulImm>(extractReg(instr, 5), extractBits(instr, 8, 8));
            case 0b11:
                return std::make_unique<DivImm>(extractReg(instr, 5), extractBits(instr, 8, 8));
        }
    }

    if (extractBits(instr, 3, 1) == 1) {
        // memory operations

        if (extractBits(instr, 4, 1) == 0) {
            return std::make_unique<Load>(
                extractReg(instr, 5), extractReg(instr, 8), extractReg(instr, 13));
        } else {
            return std::make_unique<Store>(
                extractReg(instr, 5), extractReg(instr, 8), extractReg(instr, 13));
        }
    }

    if (extractBits(instr, 4, 1) == 1) {
        // arithmetic with register second operand

        switch (extractBits(instr, 5, 2)) {
            case 0b00:
                return std::make_unique<AddReg>(extractReg(instr, 7), extractReg(instr, 10));
            case 0b01:
                return std::make_unique<SubReg>(extractReg(instr, 7), extractReg(instr, 10));
            case 0b10:
                return std::make_unique<MulReg>(extractReg(instr, 7), extractReg(instr, 10));
            case 0b11:
                return std::make_unique<DivReg>(extractReg(instr, 7), extractReg(instr, 10));
        }
    }

    if (extractBits(instr, 5, 1) == 1) {
        // shifts

        switch (extractBits(instr, 6, 2)) {
            case 0b00:
                return std::make_unique<ShlReg>(extractReg(instr, 8), extractReg(instr, 11));
            case 0b01:
                return std::make_unique<ShlImm>(extractReg(instr, 8), extractBits(instr, 11, 4));
            case 0b10:
                return std::make_unique<ShrReg>(extractReg(instr, 8), extractReg(instr, 11));
            case 0b11:
                return std::make_unique<ShrImm>(extractReg(instr, 8), extractBits(instr, 11, 4));
        }
    }

    if (extractBits(instr, 6, 1) == 0) {
        return std::make_unique<Halt>();
    } else {
        return std::make_unique<MovReg>(extractReg(instr, 7), extractReg(instr, 10));
    }

    throw std::invalid_argument{"invalid instruction"};
}

std::vector<std::unique_ptr<Instruction>> decode(std::istream& binary) {
    std::vector<std::unique_ptr<Instruction>> res;

    EncodedInstruction instr;
    while (binary.read(reinterpret_cast<char*>(&instr), sizeof(instr))) {
        res.emplace_back(decodeInstr(instr));
    }

    return res;
}

Word emulate(std::istream& binary, std::ostream& disassembly) {
    std::vector<std::unique_ptr<Instruction>> program = decode(binary);
    assert(!program.empty());

    for (const auto& i : program) {
        i->print(disassembly);
        disassembly << '\n';
    }

    EmulatorState state;

    while (state.running()) {
        if (state.pc() >= program.size()) {
            throw std::runtime_error(std::format("invalid program counter value {}", state.pc()));
        }

        program[state.pc()]->eval(state);
    }

    return state.readReg(Reg::R1);
}
}  // namespace Emulator
