#pragma once

#include <cstdio>

#include "io.hpp"

class FileReader : public virtual Reader {
    FILE* file_;

protected:
    std::optional<char> getCharRaw() override {
        if (isEof()) {
            return std::nullopt;
        }

        int c = fgetc(file_);
        if (c == EOF) {
            return std::nullopt;
        }

        return c;
    }

public:
    FileReader(FILE* f) : file_{f} {
        setbuf(file_, nullptr);
    }

    FileReader(const char* path) : FileReader{fopen(path, "r")} {}

    ~FileReader() override {
        close();
    }

    void close() override {
        if (isClosed())
            return;

        IO::close();
        fclose(file_);
    }

    size_t getReadOffset() const override {
        return ftell(file_);
    }

    void setReadOffset(size_t offset) override {
        fseek(file_, offset, SEEK_SET);
    }

    bool isEof() override {
        return isClosed() || feof(file_);
    }
};

class FileWriter : public virtual Writer {
    FILE* file_;

protected:
    void putChar(char c) override {
        fputc(c, file_);
    }

public:
    FileWriter(FILE* f) : file_{f} {
        setbuf(file_, nullptr);
    }

    FileWriter(const char* path) : FileWriter{fopen(path, "w")} {}

    ~FileWriter() {
        close();
    }

    void close() override {
        if (isClosed())
            return;

        IO::close();
        fclose(file_);
    }

    size_t getWriteOffset() const override {
        return ftell(file_);
    }

    void setWriteOffset(size_t offset) override {
        fseek(file_, offset, SEEK_SET);
    }
};

class FileReaderWriter : public FileWriter, public FileReader {
protected:
    void putChar(char c) override {
        bool shouldAdvance = getReadOffset() >= getWriteOffset();

        FileWriter::putChar(c);

        if (shouldAdvance)
            setReadOffset(getReadOffset() + 1);
    }

public:
    FileReaderWriter(FILE* sink, FILE* source) : FileWriter(sink), FileReader(source) {}

    FileReaderWriter(const char* path) : FileReaderWriter{fopen(path, "w"), fopen(path, "r")} {}

    ~FileReaderWriter() {
        close();
    }

    void close() override {
        if (isClosed()) {
            return;
        }

        FileReader::close();
        FileWriter::close();
    }

    bool isOpen() const {
        return FileWriter::isOpen() && FileReader::isOpen();
    }

    bool isClosed() const {
        return FileWriter::isClosed() || FileReader::isClosed();
    }
};
