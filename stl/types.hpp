#ifndef COS_STL_TYPES_HPP
#define COS_STL_TYPES_HPP

namespace cos {

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

using int8_t = signed char;
using int16_t = signed short;
using int32_t = signed int;
using int64_t = signed long long;

enum class byte : unsigned char {};

template <typename IntegerType>
constexpr byte operator<<(byte b, IntegerType shift) noexcept {
  return byte(static_cast<unsigned int>(b) << shift);
}

template <typename IntegerType>
constexpr byte operator>>(byte b, IntegerType shift) noexcept {
  return byte(static_cast<unsigned int>(b) >> shift);
}

template <typename IntegerType>
constexpr byte &operator<<=(byte &b, IntegerType shift) noexcept {
  return b = b << shift;
}

template <typename IntegerType>
constexpr byte &operator>>=(byte &b, IntegerType shift) noexcept {
  return b = b >> shift;
}

constexpr byte operator|(byte l, byte r) noexcept {
  return byte(static_cast<unsigned int>(l) | static_cast<unsigned int>(r));
}

constexpr byte operator&(byte l, byte r) noexcept {
  return byte(static_cast<unsigned int>(l) & static_cast<unsigned int>(r));
}

constexpr byte operator^(byte l, byte r) noexcept {
  return byte(static_cast<unsigned int>(l) ^ static_cast<unsigned int>(r));
}

constexpr byte operator~(byte b) noexcept {
  return byte(~static_cast<unsigned int>(b));
}

constexpr byte &operator|=(byte &l, byte r) noexcept { return l = (l | r); }

constexpr byte &operator&=(byte &l, byte r) noexcept { return l = l & r; }

constexpr byte &operator^=(byte &l, byte r) noexcept { return l = l ^ r; }

} // namespace cos

#endif
