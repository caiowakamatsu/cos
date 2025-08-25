// This code is 32 bit mode, i do not care for it's quality, I just want to get
// to 64 bit mode as soon as possible The above comment absolves me from any and
// all programming war crimes I commit in pursuit of my goal
#include "boot.hpp"
#include "disk.hpp"
#include "page_table_entry.hpp"
#include "physical_page.hpp"
#include "terminal.hpp"
#include "types.hpp"
#include "x820.hpp"

void print_memory_regions(cos::terminal &terminal);
cos::allocated_physical_pages
initialize_physical_page_data(cos::terminal &terminal);

extern "C" void stage2_main() {
  auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

  terminal << "Preparing to go to 64 bit mode\n";
  print_memory_regions(terminal);

  auto pages = initialize_physical_page_data(terminal);
  auto allocator = cos::page_allocator(&pages);
  // Time to start the boot struct
  auto boot_struct = reinterpret_cast<cos::boot_info *>(
      allocator.allocate_memory(sizeof(cos::boot_info)));
  boot_struct->pages = pages;

  // Find a place to put the kernel
  const auto kernel_sector_count = 256;
  const auto kernel_address =
      allocator.allocate_memory(kernel_sector_count * 512);
  terminal << "Loading kernel at "
           << cos::hex(reinterpret_cast<cos::uint32_t>(kernel_address)) << "\n";
  if (const auto status =
          cos::read_from_disk(0, 1, kernel_address, false, false);
      status != 0) {
    terminal << "FAILED TO READ KERNEL ERR: "
             << cos::decimal(static_cast<cos::uint32_t>(status));
    while (1)
      ;
  }
  terminal << "Successfully read kernel\n";
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
      const auto aligned_start =
          cos::align_up(entry.base_address, 4096); // Align to the page
      const auto aligned_space_in_entry =
          (entry.base_address + entry.length) - aligned_start;
      if (aligned_space_in_entry >= space_required &&
          aligned_start >=
              0x10'0000) { // only load the bitmap at or higher than 1mb
        return reinterpret_cast<cos::byte *>(
            static_cast<cos::uint32_t>(aligned_start));
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
      const auto first_page = entry.base_address / 4096;
      // We round up here, in case we have some weird boundary stuff from E820
      const auto entry_page_count = (entry.length + 4095) / 4096;
      for (int j = 0; j < entry_page_count; j++) {
        pages[first_page + j] = cos::physical_page_status::used;
      }
    }
  }

  // Second, let's make sure we mark the bitmap itself
  const auto bitmap_page_size = pages.bitmap_size() / 4096;
  const auto base_page =
      reinterpret_cast<cos::uint32_t>(pages.base_physical_address()) / 4096;
  for (int i = 0; i < bitmap_page_size; i++) {
    pages[base_page + i] = cos::physical_page_status::used;
  }
}

cos::allocated_physical_pages
initialize_physical_page_data(cos::terminal &terminal) {
  const auto page_count = calculate_required_page_count();
  terminal << "Will prepare " << cos::decimal(page_count)
           << " pages, space required ";
  const auto storage_required =
      cos::allocated_physical_pages::required_memory_space_for_n_pages(
          page_count);
  terminal << cos::memory_size(storage_required) << "\n";
  const auto address_for_data =
      find_spot_for_physical_page_bitmap(storage_required);
  if (address_for_data == nullptr) {
    terminal << "failed to find place for bitmap\n";
    while (true)
      ;
  }
  terminal << "Found spot for bitmap at "
           << cos::hex(reinterpret_cast<cos::uint32_t>(address_for_data))
           << "\n";

  // Time to initialize the bitmap
  auto bitmap = cos::allocated_physical_pages(address_for_data, page_count);

  // Everything is technically free here, this is not good, let's fix it :)
  initialize_used_pages(bitmap);

  return bitmap;
}

void print_memory_regions(cos::terminal &terminal) {
  const auto memory_entry_count = reinterpret_cast<unsigned short *>(0x1200);
  terminal << "loaded " << cos::decimal(*memory_entry_count)
           << " memory regions\n";

  const auto x820_entries = reinterpret_cast<cos::x820_entry *>(0x1210);
  for (int i = 0; i < *memory_entry_count; i++) {
    const auto entry = x820_entries[i];

    terminal << "A " << cos::hex(entry.base_address) << " L "
             << cos::memory_size(entry.length);

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
