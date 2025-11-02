#include "cr3.hpp"

namespace kernel::intrinsic {
std::uint64_t cr3() {
	volatile std::uint64_t cr3 = 0;
	asm volatile("mov %%cr3, %0" : "=a"(cr3)::"memory");
	return cr3;
}
}  // namespace kernel::intrinsic
