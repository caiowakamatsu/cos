// This code is 32 bit mode, i do not care for it's quality, I just want to get
// to 64 bit mode as soon as possible The above comment absolves me from any and
// all programming war crimes I commit in pursuit of my goal
#include "page_table_entry.hpp"
#include "terminal.hpp"
#include "x820.hpp"

void print_memory_regions(cos::terminal &terminal);

extern "C" void stage2_main() {
  auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

  terminal << "Preparing to go to 64 bit mode\n";
  print_memory_regions(terminal);
}

void print_memory_regions(cos::terminal &terminal) {
  const auto memory_entry_count = reinterpret_cast<unsigned short *>(0x1000);
  terminal << "loaded " << *memory_entry_count << " memory regions\n";

  const auto x820_entries = reinterpret_cast<cos::x820_entry *>(0x1010);
  for (int i = 0; i < *memory_entry_count; i++) {
    const auto entry = x820_entries[i];

    terminal << "Address: " << entry.base_address << "\n"
             << "Length: " << entry.length << "\n";

    terminal << "type: ";
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
    terminal << "\n";
  }
}
