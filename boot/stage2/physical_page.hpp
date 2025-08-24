#ifndef COS_PHYSICAL_PAGE_HPP
#define COS_PHYSICAL_PAGE_HPP

#include <types.hpp>

namespace cos {

template <typename T> T align_up(T value, cos::uint64_t alignment) {
  // This implementation works when alignment is a power of 2
  return (value + alignment - 1) & ~(alignment - 1);
}

enum class physical_page_status {
  free,
  used,
};

struct allocated_physical_page_proxy {
public:
  allocated_physical_page_proxy(cos::uint8_t index, cos::byte *source);

  operator physical_page_status() const noexcept;

  physical_page_status operator=(physical_page_status status) noexcept;

private:
  cos::uint8_t index;
  cos::byte *source;
};

struct allocated_physical_pages {
public:
  allocated_physical_pages(cos::byte *page_bit_start, cos::uint64_t page_count)
      : byte_count((page_count + 7) / 8), page_bit_start(page_bit_start),
        actual_page_count(page_count) {
    for (cos::uint64_t i = 0; i < byte_count; i++) {
      page_bit_start[i] = cos::byte(0);
    }
  }

  [[nodiscard]] allocated_physical_page_proxy
  operator[](cos::uint64_t page_number) noexcept {
    // Calculate the index of the page in the byte array
    const auto byte_lookup_index = page_number / 8;
    const auto byte_lookup_shift = page_number % 8;
    return allocated_physical_page_proxy(byte_lookup_shift,
                                         page_bit_start + byte_lookup_index);
  }

  [[nodiscard]] static cos::uint64_t
  required_memory_space_for_n_pages(cos::uint64_t page_count) {
    return (page_count + 7) / 8;
  }

  [[nodiscard]] cos::uint64_t bitmap_size() const noexcept {
    return byte_count;
  }

  [[nodiscard]] cos::byte *base_physical_address() const noexcept {
    return page_bit_start;
  }

  [[nodiscard]] cos::uint64_t page_count() const noexcept;

private:
  cos::uint64_t byte_count = 0;
  cos::uint64_t actual_page_count = 0;

  // 0 bit = free
  // 1 bit = not free :(
  cos::byte *page_bit_start = nullptr;
};

// This class is a SIMPLE page allocator, it is not smart
// We use this just to prepare the boot struct, so no performance requirement
struct page_allocator {
public:
  explicit page_allocator(allocated_physical_pages *pages);

  [[nodiscard]] cos::byte *allocate_pages(cos::uint64_t page_count);

  [[nodiscard]] cos::byte *allocate_memory(cos::uint64_t memory_size);

private:
  allocated_physical_pages *pages = nullptr;
};

} // namespace cos

#endif
