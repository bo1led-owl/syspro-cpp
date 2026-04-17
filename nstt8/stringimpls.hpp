#pragma once

#include "io.hpp"

class StringReader : public virtual Reader {
    size_t offset_ = 0;
    std::string_view input_;

protected:
    void updateInput(std::string_view s) {
        input_ = s;
    }

    std::optional<char> getCharRaw() override {
        if (isEof()) {
            return std::nullopt;
        }
        return input_[offset_++];
    }

public:
    StringReader(std::string_view input) : input_{input} {}

    size_t getReadOffset() const override {
        return offset_;
    }

    void setReadOffset(size_t offset) override {
        offset_ = std::min(offset, input_.size());
    }

    bool isEof() override {
        return isClosed() || (offset_ >= input_.size() && !bufferedChar());
    }
};

class StringWriter : public virtual Writer {
    std::string buffer_;
    size_t offset_;

protected:
    std::string_view curBuffer() {
        return buffer_;
    }

    void putChar(char c) override {
        buffer_.insert(buffer_.begin() + offset_, c);
        offset_ += 1;
    }

public:
    template <typename T>
    StringWriter(T&& initial) : buffer_{std::forward<T>(initial)}, offset_{buffer_.size()} {}

    size_t getWriteOffset() const override {
        return offset_;
    }

    void setWriteOffset(size_t offset) override {
        offset_ = std::min(offset, buffer_.size());
    }

    std::string finish() {
        close();
        return std::move(buffer_);
    }
};

class StringReaderWriter : public StringWriter, public StringReader {
protected:
    void putChar(char c) override {
        bool shouldAdvance = getReadOffset() >= getWriteOffset();

        StringWriter::putChar(c);
        StringReader::updateInput(curBuffer());

        if (shouldAdvance)
            setReadOffset(getReadOffset() + 1);
    }

public:
    template <typename T>
    StringReaderWriter(T&& initial)
        : StringWriter{std::forward<T>(initial)}, StringReader(curBuffer()) {}

    void close() override {
        StringWriter::close();
    }

    bool isClosed() const {
        return StringWriter::isClosed();
    }

    bool isOpen() const {
        return StringWriter::isOpen();
    }
};
