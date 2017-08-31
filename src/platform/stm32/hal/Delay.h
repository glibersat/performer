#pragma once

#include "Config.h"

#include <cstdint>

namespace hal {
namespace Delay {

template<uint64_t cycles>
void nop() {
    asm volatile ("nop");
    nop<cycles - 1>();
}
template<>
void nop<0>() {}

constexpr uint64_t ceil(double num) {
    return (static_cast<double>(static_cast<uint64_t>(num)) == num)
        ? static_cast<uint64_t>(num)
        : static_cast<uint64_t>(num) + ((num > 0) ? 1 : 0);
}

constexpr uint64_t ns_to_cycles(uint64_t ns) {
    return ceil(double(ns) / (double(1000000000) / double(CONFIG_CPU_FREQUENCY)));
}

// nanosecond delay based on nops
template<uint64_t ns>
void delay_ns() {
    nop<ns_to_cycles(ns)>();
}

} // namespace Delay
} // namespace hal
