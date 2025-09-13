// This code is 32 bit mode, i do not care for it's quality, I just want to get
// to 64 bit mode as soon as possible The above comment absolves me from any and
// all programming war crimes I commit in pursuit of my goal
#include <memory.hpp>
#include <string.hpp>

#include "boot.hpp"
#include "disk.hpp"
#include "page_table_entry.hpp"
#include "physical_page.hpp"
#include "terminal.hpp"
#include "types.hpp"
#include "x820.hpp"

void print_memory_regions(cos::terminal &terminal);
cos::allocated_physical_pages initialize_physical_page_data(cos::terminal &terminal);
struct filesystem_result {
	int sector_start = -1;
	int sector_count = 0;
};
filesystem_result read_filesystem_data(const char *identifier);

// Magical GDT
struct [[gnu::packed]] gdt_entry {
	cos::uint16_t limit_low;
	cos::uint16_t base_low;
	cos::uint8_t base_middle;
	cos::uint8_t access;
	cos::uint8_t granularity;
	cos::uint8_t base_high;
};

void setup_gdt(gdt_entry &gdt, cos::uint32_t base, cos::uint32_t limit, cos::uint8_t access, cos::uint8_t granularity) {
	gdt.limit_low = limit & 0xFFFF;
	gdt.base_low = base & 0xFFFF;
	gdt.base_middle = (base >> 16) & 0xFF;
	gdt.access = access;
	gdt.granularity = ((limit >> 16) & 0x0F) | (granularity & 0xF0);
	gdt.base_high = (base >> 24) & 0xFF;
}
gdt_entry gdt_entries[3];

struct [[gnu::packed]] gdt_ptr {
	cos::uint16_t limit;
	cos::uint32_t base;
};

gdt_ptr gdt_descriptor;

cos::byte *load_trampoline(cos::terminal &terminal, cos::page_allocator &allocator);

extern "C" void stage2_main() {
	auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

	print_memory_regions(terminal);

	auto pages = initialize_physical_page_data(terminal);
	auto allocator = cos::page_allocator(&pages);

	// Time to start the boot struct
	auto boot_struct = reinterpret_cast<cos::boot_info *>(allocator.allocate_memory(sizeof(cos::boot_info)));
	boot_struct->pages = pages;

	const auto kernel_disk_location = read_filesystem_data("kernel.bin");
	if (kernel_disk_location.sector_start == -1) {
		terminal << "failed to find kernel.bin in filesystem\n";
		while (1);
	} else {
		terminal << "found kernel.bin at sector " << cos::hex(kernel_disk_location.sector_start) << " with length "
				 << cos::decimal(kernel_disk_location.sector_count) << "\n";
	}

	// Find a place to put the kernel
	const auto kernel_sector_count = kernel_disk_location.sector_count;
	const auto kernel_address = allocator.allocate_memory(kernel_sector_count * 512);
	terminal << "Loading kernel at " << cos::hex(reinterpret_cast<cos::uint32_t>(kernel_address)) << "\n";
	if (const auto status = cos::read_from_disk(kernel_disk_location.sector_start + 1,
												kernel_disk_location.sector_count, kernel_address, false, false);
		status != 0) {
		terminal << "FAILED TO READ KERNEL ERR: " << cos::decimal(static_cast<cos::uint32_t>(status));
		while (1);
	}

	constexpr auto stack_size = 0x40'0000ull * 1;

	const auto kernel_stack_address = allocator.allocate_memory(stack_size);
	terminal << "Kernel stack allocated at: " << cos::hex(reinterpret_cast<cos::uint32_t>(kernel_stack_address))
			 << "\n";
	for (int i = 0; i < stack_size; i++) {
		*(kernel_stack_address + i) = cos::byte(0x69);
	}

	auto page_start_data = cos::page_map_start_data{
		.code_data = kernel_address,
		.code_size = static_cast<cos::uint32_t>(kernel_sector_count) * 512,
		.stack_data = kernel_stack_address,
		.stack_size = stack_size,
		.text_data = reinterpret_cast<cos::byte *>(0xB8000),
		.text_size = 25 * 80 * 2,  // 25 width, 80 height, 2 bytes per character
	};

	const auto cr3_first = cos::initialize_page_tables(page_start_data, terminal, allocator);
	while (true);

	setup_gdt(gdt_entries[0], 0, 0, 0, 0);
	setup_gdt(gdt_entries[1], 0, 0xFFFFF, 0x9A, 0x20 | 0xA0);
	setup_gdt(gdt_entries[2], 0, 0xFFFFF, 0x92, 0xC0);

	gdt_descriptor.base = reinterpret_cast<cos::uint32_t>(&gdt_entries);
	gdt_descriptor.limit = sizeof(gdt_entries) - 1;

	const auto trampoline_address = load_trampoline(terminal, allocator);
	// Bye C++ (for now... ill be back in 64 bit mode)

	volatile const auto eax_val = reinterpret_cast<cos::uint32_t>(cr3_first);
	volatile const auto ebx_val = reinterpret_cast<cos::uint32_t>(&gdt_descriptor);
	volatile const auto ecx_val = static_cast<cos::uint32_t>('B');

	asm volatile(
		"mov %0, %%eax\n\t"
		"mov %1, %%ebx\n\t"
		"mov %2, %%ecx\n\t"
		:
		: "r"(eax_val), "r"(ebx_val), "r"(ecx_val)
		: "eax", "ebx", "ecx");

	asm volatile("jmp %0" : : "r"(reinterpret_cast<cos::uint32_t>(trampoline_address)));
}

cos::byte *load_trampoline(cos::terminal &terminal, cos::page_allocator &allocator) {
	const auto trampoline_disk_location = read_filesystem_data("stage2.bin");
	if (trampoline_disk_location.sector_start == -1) {
		terminal << "failed to find trampoline.bin\n";
		while (1);
	} else {
		terminal << "found trampoline.bin " << cos::decimal(trampoline_disk_location.sector_count)
				 << " sectors starting at " << cos::decimal(trampoline_disk_location.sector_start) << "\n";
	}

	const auto trampoline_address = allocator.allocate_pages(trampoline_disk_location.sector_count * 512);
	if (cos::read_from_disk(trampoline_disk_location.sector_start, trampoline_disk_location.sector_count,
							trampoline_address) != 0) {
		terminal << "failed to read trampoline from disk\n";
		while (1);
	} else {
		terminal << "trampoline loaded at " << cos::hex(reinterpret_cast<cos::uint64_t>(trampoline_address)) << "\n";
	}

	return trampoline_address;
}

filesystem_result read_filesystem_data(const char *identifier) {
	// Parse table
	char file_data[512];

	if (const auto status = cos::read_from_disk(1, 1, reinterpret_cast<cos::byte *>(file_data)); status != 0) {
		return {};
	}

	auto file_count = cos::uint16_t();
	auto total_sectors = cos::uint16_t();
	cos::memcpy(reinterpret_cast<cos::byte *>(&file_count), reinterpret_cast<cos::byte *>(file_data), 2);
	cos::memcpy(reinterpret_cast<cos::byte *>(&total_sectors), reinterpret_cast<cos::byte *>(file_data + 2), 2);

	for (cos::uint16_t i = 0; i < file_count; i++) {
		const auto file_entry_sector = i / 8 + 2;  // 8 entries per sector, starting at sector 2
		const auto file_entry_read_shift = 64 * (i % 8);

		if (const auto status = cos::read_from_disk(file_entry_sector, 1, reinterpret_cast<cos::byte *>(file_data));
			status != 0) {
			return {};
		}

		if (cos::streq(identifier, file_data + file_entry_read_shift)) {
			auto result = filesystem_result();
			cos::memcpy(reinterpret_cast<cos::byte *>(&result.sector_start),
						reinterpret_cast<cos::byte *>(file_data + file_entry_read_shift + 32), 4);
			cos::memcpy(reinterpret_cast<cos::byte *>(&result.sector_count),
						reinterpret_cast<cos::byte *>(file_data + file_entry_read_shift + 36), 4);
			return result;
		}
	}

	return {};
}

cos::uint64_t calculate_required_page_count() {
	const auto memory_entry_count = *reinterpret_cast<cos::uint16_t *>(0x1200);
	const auto e820_entries = reinterpret_cast<cos::x820_entry *>(0x1210);

	// The amount of pages we keep track of start at 0
	// Then go up to the highest USABLE bit of physical memory
	// So this function really only finds the highest usable physical memory
	auto highest_memory_address = cos::uint64_t();

	for (int i = 0; i < memory_entry_count; i++) {
		const auto entry = e820_entries[i];
		if (entry.entry_type == cos::x820_entry_type::usable) {
			const auto end_address = entry.base_address + entry.length;
			if (end_address > highest_memory_address) {
				highest_memory_address = end_address;
			}
		}
	}

	constexpr auto page_size = 4096;
	const auto page_count = (highest_memory_address + page_size - 1) / page_size;

	return page_count;
}

cos::byte *find_spot_for_physical_page_bitmap(cos::uint64_t space_required) {
	const auto memory_entry_count = *reinterpret_cast<cos::uint16_t *>(0x1200);
	const auto e820_entries = reinterpret_cast<cos::x820_entry *>(0x1210);

	for (int i = 0; i < memory_entry_count; i++) {
		const auto entry = e820_entries[i];
		if (entry.entry_type == cos::x820_entry_type::usable) {
			const auto aligned_start = cos::align_up(entry.base_address, 4096);	 // Align to the page
			const auto aligned_space_in_entry = (entry.base_address + entry.length) - aligned_start;
			if (aligned_space_in_entry >= space_required &&
				aligned_start >= 0x10'0000) {  // only load the bitmap at or higher than 1mb
				return reinterpret_cast<cos::byte *>(static_cast<cos::uint32_t>(aligned_start));
			}
		}
	}

	// Failed to find anything :(
	return nullptr;
}

void initialize_used_pages(cos::allocated_physical_pages pages) {
	// First, let's clear from the e820 memory map
	const auto memory_entry_count = *reinterpret_cast<cos::uint16_t *>(0x1200);
	const auto e820_entries = reinterpret_cast<cos::x820_entry *>(0x1210);
	for (int i = 0; i < memory_entry_count; i++) {
		const auto entry = e820_entries[i];
		if (entry.entry_type != cos::x820_entry_type::usable) {
			// Technically I think some other types are reclaimable
			// But personally this falls under "not fucking around"
			const auto first_page = entry.base_address >> 12;
			// We round up here, in case we have some weird boundary stuff from E820
			const auto entry_page_count = (entry.length + 4095) / 4096;
			for (int j = 0; j < entry_page_count; j++) {
				pages[first_page + j] = cos::physical_page_status::used;
			}
		}
	}

	// Second, let's make sure we mark the bitmap itself
	const auto bitmap_page_size = (pages.bitmap_size() + 4095) / 4096;
	const auto base_page = reinterpret_cast<cos::uint32_t>(pages.base_physical_address()) >> 12;
	for (int i = 0; i < bitmap_page_size; i++) {
		pages[base_page + i] = cos::physical_page_status::used;
	}

	const auto stack_top = 0x200000;
	const auto stack_page_count = 16 + 1;
	const auto stack_base_page = (stack_top >> 12) - stack_page_count + 1;

	for (int i = 0; i < stack_page_count; i++) {
		pages[stack_base_page + i] = cos::physical_page_status::used;
	}
}

cos::allocated_physical_pages initialize_physical_page_data(cos::terminal &terminal) {
	const auto page_count = calculate_required_page_count();
	terminal << "Will prepare " << cos::decimal(page_count) << " pages, space required ";
	const auto storage_required = cos::allocated_physical_pages::required_memory_space_for_n_pages(page_count);
	terminal << cos::memory_size(storage_required) << "\n";
	const auto address_for_data = find_spot_for_physical_page_bitmap(storage_required);
	if (address_for_data == nullptr) {
		terminal << "failed to find place for bitmap\n";
		while (true);
	}
	terminal << "Found spot for bitmap at " << cos::hex(reinterpret_cast<cos::uint32_t>(address_for_data)) << "\n";

	// Time to initialize the bitmap
	auto bitmap = cos::allocated_physical_pages(address_for_data, page_count);

	// Everything is technically free here, this is not good, let's fix it :)
	initialize_used_pages(bitmap);
	terminal << "initializing used page\n";

	return bitmap;
}

void print_memory_regions(cos::terminal &terminal) {
	const auto memory_entry_count = reinterpret_cast<unsigned short *>(0x1200);
	terminal << "loaded " << cos::decimal(*memory_entry_count) << " memory regions\n";

	const auto x820_entries = reinterpret_cast<cos::x820_entry *>(0x1210);
	for (int i = 0; i < *memory_entry_count; i++) {
		const auto entry = x820_entries[i];

		terminal << "A " << cos::hex(entry.base_address) << " L " << cos::memory_size(entry.length);

		terminal << "(";
		switch (entry.entry_type) {
			case cos::x820_entry_type::usable:
				terminal << "usable";
				break;
			case cos::x820_entry_type::reserved:
				terminal << "reserved";
				break;
			case cos::x820_entry_type::acpi_reclaimable:
				terminal << "acpi_reclaimable";
				break;
			case cos::x820_entry_type::acpi_nvs:
				terminal << "acpi_nvs";
				break;
			case cos::x820_entry_type::bad_memory:
				terminal << "bad_memory";
				break;
		}
		terminal << ")\n";
	}
}
