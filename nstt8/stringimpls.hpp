#pragma once

#include "io.hpp"

class StringReader final : public Reader {
    bool closed_ = false;
    size_t offset_ = 0;
    std::string_view input_;

    void advance() override {
        offset_ += 1;
    }

    std::optional<char> peekChar() override {
        if (isEof()) {
            return std::nullopt;
        }
        return input_[offset_];
    }

public:
    StringReader(std::string_view input) : input_{input} {}

    bool isEof() override {
        return closed_ || offset_ >= input_.size();
    }

    void close() override {
        closed_ = true;
    }

    bool isOpen() const override {
        return !closed_;
    }

    size_t readOffset() const override {
        return offset_;
    }
};

class StringWriter final : public Writer {
    bool closed_ = false;
    std::string buffer_;

    void putChar(char c) override {
        buffer_.push_back(c);
    }

public:
    template <typename T>
    StringWriter(T&& initial) : buffer_{std::forward<T>(initial)} {}

    std::string finish() {
        close();
        return std::move(buffer_);
    }

    void close() override {
        closed_ = true;
    }

    bool isOpen() const override {
        return !closed_;
    }

    size_t writeOffset() const override {
        return buffer_.size();
    }
};

class StringReaderWriter final : public ReaderWriter {
    std::string buffer_;
    bool closed_ = false;
    size_t readOffset_ = 0;

protected:
    void putChar(char c) override final {
        bool shouldAdvance = readOffset() >= writeOffset();

        buffer_.push_back(c);
        if (shouldAdvance)
            advance();
    }

    void advance() override {
        readOffset_ = std::min(readOffset_ + 1, buffer_.size());
    }

    std::optional<char> peekChar() override {
        if (isEof()) {
            return std::nullopt;
        }

        return buffer_[readOffset_];
    }

public:
    template <typename T>
    StringReaderWriter(T&& initial) : buffer_{std::forward<T>(initial)} {}

    bool isEof() override {
        return closed_ || readOffset_ >= buffer_.size();
    }

    void close() override {
        closed_ = true;
    }

    bool isOpen() const override {
        return !closed_;
    }

    size_t readOffset() const override {
        return readOffset_;
    }

    size_t writeOffset() const override {
        return buffer_.size();
    }
};
