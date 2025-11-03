#ifndef __COS_STL_BITS_HPP
#define __COS_STL_BITS_HPP

#include "memory.hpp"

namespace std {

template <typename To, typename From>
[[nodiscard]] To bit_cast(From source) {
	static_assert(sizeof(To) == sizeof(From));

	auto out = To();
	std::memcpy(reinterpret_cast<std::byte*>(&out), reinterpret_cast<std::byte*>(&source), sizeof(To));

	return out;
}

}  // namespace std

#endif
