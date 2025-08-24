#ifndef COS_DISK_HPP
#define COS_DISK_HPP

#include "types.hpp"
namespace cos {

enum class disk_read_status {
  ok,
  err,
  df,
  timeout,
  sec_count,
  sec_source,
};
[[nodiscard]] cos::uint8_t
read_from_disk(cos::uint32_t sector_source, cos::uint8_t sector_count,
               cos::byte *target, bool secondary = false, bool slave = false);

} // namespace cos

#endif
