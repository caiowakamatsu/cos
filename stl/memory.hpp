#ifndef __COS_STL_MEMORY_HPP
#define __COS_STL_MEMORY_HPP

#include <types.hpp>

namespace std {
static inline void memcpy(std::byte* dst, std::byte* src, std::uint64_t len) {
	for (std::uint64_t i = 0; i < len; i++) {
		dst[i] = src[i];
	}
}

}  // namespace std

#endif
