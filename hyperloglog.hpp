#ifndef __HYPERLOGLOG_HPP__
#define __HYPERLOGLOG_HPP__

#include <algorithm>
#include <cmath>
#include <iostream>

#ifndef __EMSCRIPTEN__
#include <bit>
#endif

#include <vector>
#include <functional>

namespace HLL {

template <typename T>
class HyperLogLog {
    /* the number of index bits */
    const size_t p_;
    /* the number of registers */
    const size_t m_;
    /* alpha */
    const double alpha_;
    /* registers which hold max leading zero count */
    std::vector<uint32_t> registers_;
    /* hash function */
    std::function<uint32_t(T)> hash_fn_;

    static size_t count_leading_zeros(uint32_t val);
    static size_t get_first_n_bits(uint32_t x, size_t n);
    double get_raw_estimate();
    
    public:
    HyperLogLog(size_t p, std::function<uint32_t(T)> hash_fn);

    void add(T v);
    double get_cardinality();

};
const int BITS_PER_BYTE = 8;
const double TWO_POW_32 = 4294967296.0;

template <typename T>
double SC_DBL(T t) {
    return static_cast<double>(t);
}

double get_alpha(int m) {
    const double alpha_16 = 0.673;
    const double alpha_32 = 0.697;
    const double alpha_64 = 0.709;
    if (m < 32) return alpha_16;
    if (m < 64) return alpha_32;
    if (m < 128) return alpha_64;
    return 0.7213/(1.0 + 1.079/SC_DBL(m));
}

template <typename T>
HyperLogLog<T>::HyperLogLog(size_t p_arg, std::function<uint32_t(T)> hash_fn): p_(p_arg), m_(1<<p_), registers_(m_), hash_fn_(hash_fn), alpha_(get_alpha(m_)) {
    if (p_ < 4 || p_ > 16) {
        throw std::invalid_argument("bad p value");
    }
    
}

template <typename T>
size_t HyperLogLog<T>::count_leading_zeros(uint32_t val) {
    #ifdef __EMSCRIPTEN__
    return std::__countl_zero(val);
    #else
    return std::countl_zero(val);
    #endif
}

template <typename T>
size_t HyperLogLog<T>::get_first_n_bits(uint32_t x, size_t n) {
    return
        (x >> (sizeof(x)*BITS_PER_BYTE-n));
}

template <typename T>
void HyperLogLog<T>::add(T v) {
    auto x = hash_fn_(v);
    size_t idx = get_first_n_bits(x, p_);
    uint32_t leading_zeros_plus_one = 1 + count_leading_zeros(x << p_);
    registers_[idx] = std::max(registers_[idx], leading_zeros_plus_one);
}

template <typename T>
double HyperLogLog<T>::get_raw_estimate() {
    double inverse_sum = std::transform_reduce(
        registers_.begin(),
        registers_.end(),
        0.0,
        std::plus{},
        [] (auto val) -> double {
            return 1.0/SC_DBL(1 << val);
        }
    );

    double raw_estimate = alpha_ * m_ * m_ / inverse_sum;
    return raw_estimate;
}

template <typename T>
double HyperLogLog<T>::get_cardinality() {
    double raw_estimate = get_raw_estimate();
    if (raw_estimate <= 2.5*m_) {
        size_t count_of_zero_registers = std::transform_reduce(
            registers_.begin(),
            registers_.end(),
            0,
            std::plus<>(),
            [&](auto const& val) -> size_t {
                return val == 0 ? 1 : 0;
            });
        if (count_of_zero_registers == 0) {
            return raw_estimate;
        }
        else {
            return m_*std::log(SC_DBL(m_)/SC_DBL(count_of_zero_registers));
        }
    } else if (raw_estimate <= TWO_POW_32/30.0) {
        return raw_estimate;
    }
    return -1*TWO_POW_32*std::log(SC_DBL(1.0-SC_DBL(raw_estimate)/TWO_POW_32));
}

}
#else
#endif