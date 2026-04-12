#pragma once

#include <stdexcept>

template <typename Derived, unsigned Limit = 1>
struct Limiter {
    static unsigned current;

    static void check() {
        if (current == Limit) {
            throw std::runtime_error("tried to create too many instances of a class");
        }
    }

protected:
    Limiter() {
        check();
        current += 1;
    }

    Limiter(const Limiter&) {
        check();
        current += 1;
    }

    Limiter(Limiter&&) {
        check();
        current += 1;
    }

    Limiter& operator=(const Limiter&) {
        return *this;
    }
    Limiter& operator=(Limiter&&) {
        return *this;
    }

    ~Limiter() {
        current -= 1;
    }

public:
    static constexpr unsigned instanceLimit = Limit;
};

template <typename D, unsigned L>
unsigned Limiter<D, L>::current = 0;  // why should I do it like that :(
