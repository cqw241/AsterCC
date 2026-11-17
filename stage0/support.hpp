#pragma once
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
using str = std::string;
using i64 = int64_t;
using u8 = uint8_t;
template <class T> struct Arr {
    std::shared_ptr<std::vector<T>> p = std::make_shared<std::vector<T>>();
    Arr() = default;
    Arr(std::initializer_list<T> xs) : p(std::make_shared<std::vector<T>>(xs)) {}
    T &operator[](i64 i) {
        return p->at((size_t)i);
    }
    const T &operator[](i64 i) const {
        return p->at((size_t)i);
    }
};
template <class T> i64 len(Arr<T> a) {
    return (i64)a.p->size();
}
template <class T> decltype(auto) index(Arr<T> a, i64 i) {
    return a.p->at((size_t)i);
}
inline u8 index(const str &s, i64 i) {
    return static_cast<u8>(s.at((size_t)i));
}
inline i64 len(str s) {
    return (i64)s.size();
}
template <class T> void push(Arr<T> a, T v) {
    a.p->push_back(v);
}
template <class T> void pop(Arr<T> a) {
    a.p->pop_back();
}
inline str slice(str s, i64 p, i64 n) {
    return s.substr((size_t)p, (size_t)n);
}
inline str chr(i64 n) {
    return str(1, (char)n);
}
inline str itoa(i64 n) {
    return std::to_string(n);
}
inline i64 parse_int(str s) {
    return std::bit_cast<i64>((uint64_t)std::strtoull(
        s.c_str(), nullptr, s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X') ? 16 : 10));
}
inline str env(str s) {
    auto p = std::getenv(s.c_str());
    return p ? p : "";
}
inline void print(str s) {
    std::cout << s;
}
inline void eprint(str s) {
    std::cerr << s;
}
inline void println(str s) {
    std::cout << s << '\n';
}
inline void print_int(i64 n) {
    std::cout << n;
}
inline void println_int(i64 n) {
    std::cout << n << '\n';
}
inline str read_file(str p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) {
        eprint("aster: cannot read " + p + "\n");
        std::exit(66);
    }
    std::ostringstream b;
    b << f.rdbuf();
    return b.str();
}
inline i64 write_file(str p, str s) {
    std::ofstream f(p, std::ios::binary);
    f << s;
    return f ? 0 : 1;
}
inline Arr<str> seed_args;
inline Arr<str> args() {
    return seed_args;
}
inline i64 run(Arr<str> a) {
    std::vector<char *> v;
    for (auto &s : *a.p)
        v.push_back(s.data());
    v.push_back(nullptr);
    std::cout.flush();
    std::cerr.flush();
    auto p = fork();
    if (!p) {
        execvp(v[0], v.data());
        _exit(127);
    }
    int s;
    waitpid(p, &s, 0);
    return WIFEXITED(s) ? WEXITSTATUS(s) : 128 + WTERMSIG(s);
}
inline i64 clock_ns() {
    return std::chrono::steady_clock::now().time_since_epoch().count();
}
inline str join(Arr<str> a, str sep) {
    str r;
    size_t n = 0;
    for (auto &s : *a.p)
        n += s.size() + sep.size();
    r.reserve(n);
    for (size_t i = 0; i < a.p->size(); i++) {
        if (i)
            r += sep;
        r += (*a.p)[i];
    }
    return r;
}
inline i64 wrap_add(i64 a, i64 b) {
    return std::bit_cast<i64>((uint64_t)a + (uint64_t)b);
}
inline str wrap_add(str a, str b) {
    return a + b;
}
inline i64 wrap_sub(i64 a, i64 b) {
    return std::bit_cast<i64>((uint64_t)a - (uint64_t)b);
}
inline i64 wrap_mul(i64 a, i64 b) {
    return std::bit_cast<i64>((uint64_t)a * (uint64_t)b);
}
inline i64 wrap_div(i64 a, i64 b) {
    if (!b)
        std::exit(70);
    return a == INT64_MIN && b == -1 ? a : a / b;
}
inline i64 wrap_rem(i64 a, i64 b) {
    if (!b)
        std::exit(70);
    return a == INT64_MIN && b == -1 ? 0 : a % b;
}
inline i64 wrap_shl(i64 a, i64 b) {
    return std::bit_cast<i64>((uint64_t)a << ((uint64_t)b & 63));
}
