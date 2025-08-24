#ifndef COS_DISK_HPP
#define COS_DISK_HPP

#include "types.hpp"
namespace cos {

bool read_from_disk(cos::uint32_t sector_source, cos::uint8_t sector_count,
                    cos::byte *target);

}

#endif
