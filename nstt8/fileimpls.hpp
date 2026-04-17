#pragma once

#include <cstdio>

#include "io.hpp"

class FileReader final : public Reader {
    FILE* file_;
    bool closed_ = false;

protected:
    void advance() override {
        fseek(file_, 1, SEEK_CUR);
    }

    std::optional<char> getChar() override {
        if (isEof()) {
            return std::nullopt;
        }

        return fgetc(file_);
    }

    std::optional<char> peekChar() override {
        if (isEof()) {
            return std::nullopt;
        }

        char res = fgetc(file_);
        ungetc(res, file_);
        return res;
    }

public:
    FileReader(FILE* f) : file_{f} {}

    FileReader(const char* path) : file_{fopen(path, "r")} {}

    ~FileReader() {
        close();
    }

    size_t readOffset() const override {
        return ftell(file_);
    }

    bool isEof() override {
        return closed_ || feof(file_);
    }

    void close() override {
        if (closed_)
            return;
        closed_ = true;
        fclose(file_);
    }

    bool isOpen() const override {
        return !closed_;
    }
};

class FileWriter final : public Writer {
    FILE* file_;
    bool closed_ = false;

protected:
    void putChar(char c) override {
        fputc(c, file_);
    }

public:
    FileWriter(FILE* f) : file_{f} {}

    FileWriter(const char* path) : file_{fopen(path, "w")} {}

    ~FileWriter() {
        close();
    }

    void close() override {
        if (closed_)
            return;
        closed_ = true;
        fclose(file_);
    }

    bool isOpen() const override {
        return !closed_;
    }

    size_t writeOffset() const override {
        return ftell(file_);
    }
};

class FileReaderWriter final : public ReaderWriter {
    FILE* file_;
    bool closed_ = false;

protected:
    void putChar(char c) override {
        fputc(c, file_);
    }

    void advance() override {
        fseek(file_, 1, SEEK_CUR);
    }

    std::optional<char> getChar() override {
        if (isEof()) {
            return std::nullopt;
        }

        return fgetc(file_);
    }

    std::optional<char> peekChar() override {
        if (isEof()) {
            return std::nullopt;
        }

        char res = fgetc(file_);
        ungetc(res, file_);
        return res;
    }

public:
    FileReaderWriter(FILE* f) : file_{f} {}

    FileReaderWriter(const char* path) : file_{fopen(path, "rw")} {}

    ~FileReaderWriter() {
        close();
    }

    size_t readOffset() const override {
        return ftell(file_);
    }

    size_t writeOffset() const override {
        return ftell(file_);
    }

    bool isEof() override {
        return closed_ || feof(file_);
    }

    void close() override {
        if (closed_)
            return;
        fclose(file_);
        closed_ = true;
    }

    bool isOpen() const override {
        return !closed_;
    }
};
