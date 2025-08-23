// This code is 32 bit mode, i do not care for it's quality, I just want to get
// to 64 bit mode as soon as possible The above comment absolves me from any and
// all programming war crimes I commit in pursuit of my goal
#include "page_table_entry.hpp"
#include "terminal.hpp"

void print_memory_regions(cos::terminal &terminal);

extern "C" void stage2_main() {
  auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));

  terminal << "Preparing to go to 64 bit mode\n";
  print_memory_regions(terminal);

  /*
terminal.write("Successfully in 32bit mode");
terminal.write("Preparing to go to 64bit mode");

volatile unsigned short *memory_entry_count =
reinterpret_cast<unsigned short *>(0x1000);
char o[2];
o[0] = '0' + *memory_entry_count;
o[1] = 0;
terminal.write(o);*/
}

void print_memory_regions(cos::terminal &terminal) {
  volatile unsigned short *memory_entry_count =
      reinterpret_cast<unsigned short *>(0x1000);
  terminal << "loaded " << *memory_entry_count << " memory regions\n";
}
