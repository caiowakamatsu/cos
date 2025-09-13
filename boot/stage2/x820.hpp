#ifndef X820_HPP
#define X820_HPP

namespace cos {
enum class x820_entry_type : unsigned int { usable = 0x1, reserved, acpi_reclaimable, acpi_nvs, bad_memory };

struct x820_entry {
	x820_entry() = default;

	unsigned long long base_address = 0;
	unsigned long long length = 0;
	x820_entry_type entry_type = x820_entry_type::bad_memory;
	unsigned int extra_meta = 0;
};
}  // namespace cos

#endif
