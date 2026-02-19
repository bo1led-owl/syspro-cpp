#include <cassert>
#include <cmath>
#include <concepts>
#include <optional>

template <std::floating_point T>
constexpr T CMP_EPS = 10e-5;

template <std::floating_point T>
bool floatPartialEq(T x, T y) {
    return std::abs(x - y) < CMP_EPS<T>;
}

template <std::floating_point T>
struct Vector;

template <std::floating_point T>
struct Point {
    T x, y;

    Point(T x, T y) : x{x}, y{y} {}
    explicit Point(const Vector<T>& v);

    bool operator==(const Point& that) const {
        return floatPartialEq(x, that.x) && floatPartialEq(y, that.y);
    }
};

template <std::floating_point T>
struct Vector {
    T x, y;

    Vector(T x, T y) : x{x}, y{y} {}

    Vector(const Point<T>& a, const Point<T>& b) : x{b.x - a.x}, y{b.y - a.y} {}

    bool operator==(const Vector& that) const {
        return floatPartialEq(x, that.x) && floatPartialEq(y, that.y);
    }

    Vector operator-() const {
        return Vector{-x, -y};
    }

    Vector& operator+=(const Vector& that) {
        x += that.x;
        y += that.y;
        return *this;
    }

    Vector operator+(const Vector& that) const {
        Vector res = *this;
        res += that;
        return res;
    }

    Vector& operator-=(const Vector& that) {
        x -= that.x;
        y -= that.y;
        return *this;
    }

    Vector operator-(const Vector& that) const {
        Vector res = *this;
        res -= that;
        return res;
    }

    bool collinearWith(const Vector& that) const {
        return floatPartialEq(std::abs(x * that.y), std::abs(y * that.x));
    }

    bool orthogonalTo(const Vector& that) const {
        return floatPartialEq<T>(dot(that), 0);
    }

    Vector orthogonal() const {
        return Vector{-y, x};
    }

    T dot(const Vector& that) const {
        return x * that.x + y * that.y;
    }

    T length() const {
        return std::sqrt(x * x + y * y);
    }

    bool nonZero() const {
        return !floatPartialEq<T>(x, 0) || !floatPartialEq<T>(y, 0);
    }

    Vector& normalize() {
        T l = length();
        x /= l;
        y /= l;
        return *this;
    }

    Vector normalized() const {
        Vector res = *this;
        res.normalize();
        return res;
    }

    bool isNormalized() const {
        return floatPartialEq(length(), static_cast<T>(1.0));
    }
};

template <std::floating_point T>
Point<T>::Point(const Vector<T>& v) : x{v.x}, y{v.y} {}

template <std::floating_point T, typename F>
    requires std::convertible_to<F, T>
Vector<T> operator*(F a, const Vector<T>& v) {
    return Vector{static_cast<T>(a) * v.x, static_cast<T>(a) * v.y};
}

template <std::floating_point T, typename F>
    requires std::convertible_to<F, T>
Vector<T> operator*(const Vector<T>& v, F a) {
    return static_cast<T>(a) * v;
}

template <std::floating_point T, typename F>
    requires std::convertible_to<F, T>
Vector<T> operator/(const Vector<T>& v, F a) {
    return Vector{v.x / static_cast<T>(a), v.y / static_cast<T>(a)};
}

template <std::floating_point T>
Point<T> operator+(const Point<T>& p, const Vector<T>& v) {
    return Point{p.x + v.x, p.y + v.y};
}

template <std::floating_point T>
struct Line {
    Point<T> start;
    Vector<T> direction;

    Line(const Point<T>& start, const Vector<T>& direction) : start{start}, direction{direction} {
        assert(direction.nonZero());
    }

    Line(const Point<T>& a, const Point<T>& b) : Line{a, Vector(a, b)} {}

    bool contains(const Point<T>& p) const {
        return direction.collinearWith(Vector{start, p});
    }

    std::optional<Point<T>> intersectionWith(const Line& that) const {
        if (parallelTo(that)) {
            return std::nullopt;
        }

        Point<T> p1 = start + direction;
        Point<T> p2 = that.start + that.direction;

        T c1 = start.y * p1.x - start.x * p1.y;
        T c2 = that.start.y * p2.x - that.start.x * p2.y;

        T d = direction.x * that.direction.y - direction.y * that.direction.x;

        return Point{(c1 * that.direction - c2 * direction) / d};
    }

    std::optional<Line> perpendicularAt(const Point<T>& p) const {
        if (!contains(p)) {
            return std::nullopt;
        }

        return Line{p, direction.orthogonal()};
    }

    bool parallelTo(const Line& that) const {
        return direction.collinearWith(that.direction);
    }

    bool operator==(const Line& that) const {
        return contains(that.start) && parallelTo(that);
    }
};
