#ifndef __COS_STL_MEMORY_HPP
#define __COS_STL_MEMORY_HPP

#include <types.hpp>

namespace cos {
static inline void memcpy(cos::byte *dst, cos::byte *src, cos::uint64_t len) {
  for (cos::uint64_t i = 0; i < len; i++) {
    dst[i] = src[i];
  }
}

} // namespace cos

#endif
