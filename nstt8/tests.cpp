#include <gtest/gtest.h>

#include "fileimpls.hpp"
#include "stringimpls.hpp"

TEST(String, Reader) {
    StringReader sr("foobar\n42\n");
    EXPECT_EQ(sr.readLine(), "foobar");
    EXPECT_EQ(sr.readI16(), 42);
    EXPECT_TRUE(sr.isOpen());
    EXPECT_FALSE(sr.isEof());
    sr.close();
    EXPECT_TRUE(sr.isEof());
    EXPECT_FALSE(sr.isOpen());
}

TEST(String, Writer) {
    StringWriter sw("foo");
    sw.writeString("bar");
    sw.writeU16(42);
    EXPECT_TRUE(sw.isOpen());
    sw.close();
    EXPECT_EQ(sw.finish(), "foobar42");
    EXPECT_FALSE(sw.isOpen());
}

TEST(String, ReaderWriter) {
    StringReaderWriter srw("foo");
    EXPECT_EQ(srw.readLine(), "foo");
    srw.writeChar('x');
    srw.writeU8(255);
    EXPECT_TRUE(srw.isEof());
}

TEST(File, Reader) {
    FILE* f = tmpfile();
    std::string_view data = "foobar\n42\n";
    fwrite(data.data(), data.size(), 1, f);
    rewind(f);

    FileReader fr(f);
    EXPECT_EQ(fr.readLine(), "foobar");
    EXPECT_EQ(fr.readI16(), 42);
    EXPECT_TRUE(fr.isOpen());
    EXPECT_FALSE(fr.isEof());
    fr.close();
    EXPECT_TRUE(fr.isEof());
    EXPECT_FALSE(fr.isOpen());
}

TEST(File, Writer) {
    FILE* f = tmpfile();
    FileWriter fw(f);
    fw.writeString("foo");
    fw.writeString("bar");
    fw.writeU16(42);
    EXPECT_TRUE(fw.isOpen());

    std::string res;
    rewind(f);
    for (;;) {
        int c = fgetc(f);
        if (c == EOF) {
            break;
        }
        res.push_back(c);
    }

    fw.close();
    EXPECT_EQ(res, "foobar42");
    EXPECT_FALSE(fw.isOpen());
}

TEST(File, ReaderWriter) {
    FILE* f = tmpfile();
    fputs("foo", f);
    rewind(f);

    FileReaderWriter frw(f);
    EXPECT_EQ(frw.readLine(), "foo");

    frw.writeString("bar");
    frw.writeU16(42);

    rewind(f);
    std::string res;
    for (;;) {
        int c = fgetc(f);
        if (c == EOF) {
            break;
        }
        res.push_back(c);
    }

    EXPECT_EQ(res, "foobar42");

    EXPECT_TRUE(frw.isOpen());
    frw.close();
    EXPECT_FALSE(frw.isOpen());
}
