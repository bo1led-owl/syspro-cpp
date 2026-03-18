#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <memory>
#include <numeric>
#include <ranges>
#include <span>

template <typename T>
concept Num = std::convertible_to<int, T> && std::copyable<T> && std::equality_comparable<T> &&
              requires(T x, T y) {
                  { x + y } -> std::convertible_to<T>;
                  { x - y } -> std::convertible_to<T>;
                  { x * y } -> std::convertible_to<T>;
                  { x / y } -> std::convertible_to<T>;
              };

template <Num T>
class Matrix {
public:
    using size_type = size_t;

private:
    size_type dimension_;
    std::unique_ptr<T[]> items_;

    static std::unique_ptr<T[]> newBuffer(size_type size) {
        return std::unique_ptr<T[]>{new T[size * size]};
    }

public:
    using iterator = T*;
    using const_iterator = const T*;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    explicit Matrix(size_type dimension)
        : dimension_{dimension}, items_{newBuffer(dimension * dimension)} {
        std::ranges::fill(*this, 0);
    }

    template <typename R>
        requires(!std::same_as<R, Matrix>) && std::ranges::sized_range<R>
    explicit Matrix(const R& diagonal) : Matrix(std::ranges::size(diagonal)) {
        size_type i = 0;
        for (T item : diagonal) {
            this->operator[](i, i) = item;
            ++i;
        }
    }

    explicit Matrix(std::initializer_list<T> diagonal)
        : Matrix(std::ranges::views::all(diagonal)) {}

    Matrix(const Matrix& m) : dimension_{m.dimension_}, items_{newBuffer(m.dimension_)} {
        std::ranges::copy(m, this->begin());
    }

    Matrix(Matrix&& m)
        : dimension_{std::exchange(m.dimension_, 0)}, items_{std::exchange(m.items_, nullptr)} {}

    Matrix& operator=(const Matrix& that) {
        if (&that == this) {
            return *this;
        }

        if (dimension_ != that.dimension_) {
            items_ = newBuffer(that.dimension_);
            dimension_ = that.dimension_;
        }

        std::ranges::copy(that, this->begin());

        return *this;
    }

    size_type dimension() const {
        return dimension_;
    }

    explicit operator T() const {
        return std::reduce(begin(), end(), 0);
    }

    T& operator[](size_type i, size_type j) {
        assert(i < dimension_ && j < dimension_);
        return items_[i * dimension_ + j];
    }

    const T& operator[](size_type i, size_type j) const {
        assert(i < dimension_ && j < dimension_);
        return items_[i * dimension_ + j];
    }

    std::span<T> operator[](size_type i) {
        assert(i < dimension_);
        return std::span{items_.get() + i * dimension_, dimension_};
    }

    std::span<const T> operator[](size_type i) const {
        assert(i < dimension_);
        return std::span{items_.get() + i * dimension_, dimension_};
    }

    Matrix& operator+=(const Matrix& that) {
        assert(dimension_ == that.dimension_);

        for (size_type i = 0; i < dimension_; ++i) {
            for (size_type j = 0; j < dimension_; ++j) {
                this->operator[](i, j) += that[i, j];
            }
        }

        return *this;
    }

    Matrix operator+(const Matrix& that) const {
        assert(dimension_ == that.dimension_);

        Matrix res(*this);
        return res += that;
    }

    Matrix operator-() const {
        Matrix res(*this);
        for (T& x : res) {
            x = -x;
        }

        return res;
    }

    Matrix& operator-=(const Matrix& that) {
        assert(dimension_ == that.dimension_);

        for (size_type i = 0; i < dimension_; ++i) {
            for (size_type j = 0; j < dimension_; ++j) {
                this->operator[](i, j) -= that[i, j];
            }
        }

        return *this;
    }

    Matrix operator-(const Matrix& that) const {
        assert(dimension_ == that.dimension_);

        Matrix res(*this);
        return res -= that;
    }

    Matrix operator*(const Matrix& that) const {
        assert(dimension_ == that.dimension_);

        Matrix res(dimension_);

        for (size_type i = 0; i < dimension_; ++i) {
            for (size_type j = 0; j < dimension_; ++j) {
                for (size_type k = 0; k < dimension_; ++k) {
                    res[i, j] += this->operator[](i, k) * that[k, j];
                }
            }
        }

        return res;
    }

    Matrix& operator*=(const Matrix& that) {
        assert(dimension_ == that.dimension_);
        return *this = std::move(*this * that);
    }

    Matrix& operator*=(T scalar) {
        for (T& x : *this) {
            x *= scalar;
        }
        return *this;
    }

    friend Matrix operator*(T scalar, Matrix& mat) {
        Matrix res(mat);
        res *= scalar;
        return res;
    }

    friend Matrix operator*(Matrix& mat, T scalar) {
        return scalar * mat;
    }

    bool operator==(const Matrix& that) const {
        return dimension_ == that.dimension_ && std::ranges::equal(*this, that);
    }

    iterator begin() {
        return items_.get();
    }
    iterator end() {
        return begin() + dimension_ * dimension_;
    }
    const_iterator begin() const {
        return cbegin();
    }
    const_iterator end() const {
        return cend();
    }
    const const_iterator cbegin() const {
        return items_.get();
    }
    const const_iterator cend() const {
        return cbegin() + dimension_ * dimension_;
    }

    reverse_iterator rbegin() {
        return std::reverse_iterator{items_.get() + dimension_ * dimension_};
    }
    reverse_iterator rend() {
        return std::reverse_iterator{items_.get()};
    }
    const_reverse_iterator rbegin() const {
        return crbegin();
    }
    const_reverse_iterator rend() const {
        return crend();
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator{items_.get() + dimension_ * dimension_};
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator{items_.get()};
    }
};
