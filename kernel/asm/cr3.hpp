#ifndef KERNEL_ASM_CR3_HPP
#define KERNEL_ASM_CR3_HPP

#include <types.hpp>

namespace kernel::intrinsic {

[[nodiscard]] std::uint64_t cr3();

}

#endif
