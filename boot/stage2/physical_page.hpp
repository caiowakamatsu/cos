#ifndef COS_PHYSICAL_PAGE_HPP
#define COS_PHYSICAL_PAGE_HPP

#include <types.hpp>

namespace cos {

struct physical_page {
	std::uint64_t number;

	constexpr static auto size = std::uint32_t(4096);
};

template <typename T>
T align_up(T value, std::uint64_t alignment) {
	// This implementation works when alignment is a power of 2
	return (value + alignment - 1) & ~(alignment - 1);
}

enum class physical_page_status {
	free,
	used,
};

class page_bitmap {
public:
private:
	std::byte *data;
};

}  // namespace cos

#endif
