// This code is 32 bit mode, i do not care for it's quality, I just want to get
// to 64 bit mode as soon as possible The above comment absolves me from any and
// all programming war crimes I commit in pursuit of my goal
#include "page_table_entry.hpp"
#include "terminal.hpp"

extern "C" void stage2_main() {
  auto terminal = cos::terminal(reinterpret_cast<char *>(0xB8000));
  terminal.write("Successfully in 32bit mode");
  terminal.write("Preparing to go to 64bit mode");
}
