#pragma once

#include <array>
#include <cctype>
#include <concepts>
#include <cstdint>
#include <expected>
#include <numeric>
#include <optional>
#include <string>

struct IO {
    virtual ~IO() = default;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

class Reader : public IO {
public:
    enum class Error {
        Eof,
        InvalidChar,
    };

protected:
    virtual void advance() = 0;
    virtual std::optional<char> getChar() {
        auto res = peekChar();
        advance();
        return res;
    }
    virtual std::optional<char> peekChar() = 0;

private:
    template <std::integral T>
    std::expected<T, Error> readIntegral() {
        T res = 0;
        auto c = peekChar();

        bool negative = false;
        if constexpr (std::is_signed_v<T>) {
            if (c == '-') {
                negative = true;
                getChar();
                c = peekChar();
            }
        }

        if (!c.has_value()) {
            return std::unexpected(Error::Eof);
        }

        if (!std::isdigit(*c)) {
            return std::unexpected(Error::InvalidChar);
        }

        while (c.has_value() && std::isdigit(*c)) {
            getChar();
            res *= 10;
            res += *c - '0';

            c = peekChar();
        }

        if (negative) {
            res = -res;
        }

        return res;
    }

public:
    virtual size_t readOffset() const = 0;

    virtual bool isEof() = 0;

    std::string readLine() {
        std::string res;
        auto c = getChar();

        while (c.has_value() && c != '\n' && c != EOF) {
            res.push_back(*c);
            c = getChar();
        }

        return res;
    }

    std::expected<char, Error> readChar() {
        auto c = getChar();
        if (!c.has_value()) {
            return std::unexpected(Error::Eof);
        }

        return *c;
    }

    std::expected<uint8_t, Error> readU8() {
        return readIntegral<uint8_t>();
    }

    std::expected<int8_t, Error> readI8() {
        return readIntegral<int8_t>();
    }

    std::expected<uint16_t, Error> readU16() {
        return readIntegral<uint16_t>();
    }

    std::expected<int16_t, Error> readI16() {
        return readIntegral<int16_t>();
    }

    std::expected<uint32_t, Error> readU32() {
        return readIntegral<uint32_t>();
    }

    std::expected<int32_t, Error> readI32() {
        return readIntegral<int32_t>();
    }

    std::expected<uint64_t, Error> readU64() {
        return readIntegral<uint64_t>();
    }

    std::expected<int64_t, Error> readI64() {
        return readIntegral<int64_t>();
    }
};

class Writer : public IO {
    template <typename T>
    struct Digits10 {
        static constexpr size_t value = 0;
    };

    template <>
    struct Digits10<int8_t> : std::integral_constant<size_t, 3> {};
    template <>
    struct Digits10<uint8_t> : std::integral_constant<size_t, 3> {};
    template <>
    struct Digits10<int16_t> : std::integral_constant<size_t, 5> {};
    template <>
    struct Digits10<uint16_t> : std::integral_constant<size_t, 5> {};
    template <>
    struct Digits10<int32_t> : std::integral_constant<size_t, 10> {};
    template <>
    struct Digits10<uint32_t> : std::integral_constant<size_t, 10> {};
    template <>
    struct Digits10<int64_t> : std::integral_constant<size_t, 20> {};
    template <>
    struct Digits10<uint64_t> : std::integral_constant<size_t, 20> {};

protected:
    virtual void putChar(char c) = 0;

private:
    template <std::integral T>
    bool writeIntegral(T x) {
        if (!isOpen()) {
            return false;
        }

        if (x == 0) {
            putChar('0');
            return true;
        }

        if (x < 0) {
            putChar('-');
            x = -x;
        }

        size_t charsWritten = 0;
        std::array<char, Digits10<T>::value> chars;
        while (x > 0) {
            chars[charsWritten] = x % 10 + '0';
            x /= 10;
            charsWritten += 1;
        }

        for (size_t i = 0; i < charsWritten; ++i) {
            putChar(chars[charsWritten - i - 1]);
        }

        return true;
    }

public:
    virtual size_t writeOffset() const = 0;

    bool writeString(std::string_view s) {
        if (!isOpen()) {
            return false;
        }

        for (char c : s) {
            putChar(c);
        }

        return true;
    }

    bool writeChar(char c) {
        if (!isOpen()) {
            return false;
        }

        putChar(c);
        return true;
    }

    bool writeU8(uint8_t x) {
        return writeIntegral<uint8_t>(x);
    }

    bool writeI8(int8_t x) {
        return writeIntegral<int8_t>(x);
    }

    bool writeU16(uint16_t x) {
        return writeIntegral<uint16_t>(x);
    }

    bool writeI16(int16_t x) {
        return writeIntegral<int16_t>(x);
    }

    bool writeU32(uint32_t x) {
        return writeIntegral<uint32_t>(x);
    }

    bool writeI32(int32_t x) {
        return writeIntegral<int32_t>(x);
    }

    bool writeU64(uint64_t x) {
        return writeIntegral<uint64_t>(x);
    }

    bool writeI64(int64_t x) {
        return writeIntegral<int64_t>(x);
    }
};

struct ReaderWriter : public Reader, public Writer {};
