#ifndef COS_BOOT_INFO_HPP
#define COS_BOOT_INFO_HPP

#include <array.hpp>
#include <types.hpp>

#include "e820.hpp"

namespace cos {
struct boot_info {
	std::uint8_t memory_region_count;
	std::array<cos::e820_entry, 32> memory_regions;

	std::uint64_t page_bitmap_start;  // Where is the page bitmap located?
	std::uint64_t page_bitmap_count;  // How many pages does the bitmap store?
};
}  // namespace cos

#endif
