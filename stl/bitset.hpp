#ifndef STL_BITSET_HPP
#define STL_BITSET_HPP

#include <types.hpp>

#include "array.hpp"

namespace std {

template <std::size_t BitCount>
class bitset {
private:
	constexpr static std::size_t StorageCount = (BitCount + 31) / 32;

	array<uint32_t, BitCount> bit_storage;

public:
	bitset() {
		for (std::size_t i = 0; i < StorageCount; i++) {
			bit_storage[i] = 0;
		}
	}

	[[nodiscard]] bool test(std::size_t index) const noexcept {
		const auto storage = bit_storage[index / 32];

		const auto mask = ((index % 32) << 1u);

		return storage & mask;
	}

	void set(std::size_t index, bool value = true) noexcept {
		const auto mask = ((index % 32) << 1u);

		if (value) {
			bit_storage[index / 32] |= mask;
		} else {
			bit_storage[index / 32] &= ~mask;
		}
	}
};

}  // namespace std

#endif
