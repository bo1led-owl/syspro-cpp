#include <gtest/gtest.h>

#include <print>

#include "matrix.hpp"

using M = Matrix<float>;

namespace {
template <typename T>
void printMatrix(const Matrix<T>& mat) {
    for (size_t i = 0; i < mat.dimension(); ++i) {
        for (size_t j = 0; j < mat.dimension(); ++j) {
            std::print("{} ", mat[i, j]);
        }
        std::println();
    }
}
}  // namespace

TEST(Matrix, Construction) {
    {
        M mat(4);
        std::vector<float> items(16, 0);
        EXPECT_EQ(mat.dimension(), 4);
        EXPECT_TRUE(std::ranges::equal(mat, items));
    }
    {
        M mat{1, 2, 3, 4, 5};
        std::vector<float> items(25, 0);
        for (size_t i = 0; i < 5; ++i) {
            items[i * 5 + i] = i + 1;
        }
        EXPECT_EQ(mat.dimension(), 5);
        EXPECT_TRUE(std::ranges::equal(mat, items));
    }
    {
        M mat(std::vector{1, 2, 3, 4, 5});

        std::vector<float> items(25, 0);
        for (size_t i = 0; i < 5; ++i) {
            items[i * 5 + i] = i + 1;
        }

        EXPECT_EQ(mat.dimension(), 5);
        EXPECT_TRUE(std::ranges::equal(mat, items));
    }
}

TEST(Matrix, Subscript) {
    static constexpr size_t DIMENSION = 7;

    M mat1(DIMENSION);
    M mat2(DIMENSION);

    std::vector<float> items(DIMENSION * DIMENSION);
    for (size_t i = 0; i < DIMENSION * DIMENSION; ++i) {
        items[i] = i;
        mat1[i / DIMENSION, i % DIMENSION] = i;
        mat2[i / DIMENSION][i % DIMENSION] = i;
    }

    EXPECT_TRUE(std::ranges::equal(mat1, items));
    EXPECT_TRUE(std::ranges::equal(mat2, items));
}

TEST(Matrix, Equality) {
    static constexpr size_t DIMENSION = 7;

    M mat1(DIMENSION);
    M mat2(DIMENSION);

    for (size_t i = 0; i < DIMENSION; ++i) {
        for (size_t j = 0; j < DIMENSION; ++j) {
            mat1[i, j] = i * DIMENSION + j;
            mat2[i, j] = i * DIMENSION + j;
        }
    }

    EXPECT_EQ(mat1, mat2);

    mat1[2, 3] = 42;
    EXPECT_NE(mat1, mat2);
}

TEST(Matrix, Copying) {
    static constexpr size_t DIMENSION = 5;

    M mat1(DIMENSION);

    for (size_t i = 0; i < DIMENSION; ++i) {
        for (size_t j = 0; j < DIMENSION; ++j) {
            mat1[i, j] = i * DIMENSION + j;
        }
    }

    M mat2(mat1);
    EXPECT_EQ(mat1, mat2);

    mat2[0, 0] = 37;
    EXPECT_NE(mat1, mat2);
}

TEST(Matrix, Arithmetic) {
    static constexpr size_t DIMENSION = 14;

    M mat(DIMENSION);

    {
        M mat2(DIMENSION);

        for (size_t i = 0; i < DIMENSION * DIMENSION; ++i) {
            mat[i / DIMENSION, i % DIMENSION] = i;
            mat2[i / DIMENSION, i % DIMENSION] = i * 2;
        }

        EXPECT_EQ(mat2, 2 * mat);
        EXPECT_EQ(mat2, mat * 2);
    }

    {
        M mat2 = mat + 3 * mat;
        EXPECT_EQ(mat2, 4 * mat);
        EXPECT_EQ(mat2 - 4 * mat, M(DIMENSION));
        EXPECT_EQ(M(DIMENSION) - mat2, -mat2);
    }
}
