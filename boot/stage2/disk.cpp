#include "disk.hpp"
#include "types.hpp"

extern "C" {
void read_lba();
}

namespace cos {

bool read_from_disk(cos::uint32_t sector_source, cos::uint8_t sector_count,
                    cos::byte *target) {
  const auto addr_int = reinterpret_cast<cos::uint32_t>(target);
  if (addr_int > 0x10FFEF) {
    return false; // no, you are not loading that high
  }

  return true;
}

} // namespace cos
