#ifndef E820_HPP
#define E820_HPP

#include <types.hpp>

namespace cos {

enum class e820_entry_type : std::uint32_t {
	usable = 0x1,
	reserved,
	acpi_reclaimable,
	acpi_nvs,
	bad_memory,
};

struct e820_entry {
	e820_entry() = default;

	std::uint64_t base_address = 0;
	std::uint64_t length = 0;
	e820_entry_type type = e820_entry_type::bad_memory;
	std::uint32_t extra_metadata = 0;
};

}  // namespace cos

#endif
