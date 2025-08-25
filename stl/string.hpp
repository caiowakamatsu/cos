#ifndef __COS_STL_STRING_HPP
#define __COS_STL_STRING_HPP

namespace cos {
[[nodiscard]] static inline bool streq(const char *a, const char *b) {
  while (*a == *b) {
    if (*a == 0) {
      return true;
    }
    a += 1;
    b += 1;
  }

  return false;
}
} // namespace cos

#endif
