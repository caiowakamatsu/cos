#include "physical_page.hpp"
#include "types.hpp"
#include "x820.hpp"

namespace cos {

physical_page_status
allocated_physical_page_proxy::operator=(physical_page_status status) noexcept {
  if (status == physical_page_status::free) {
    (*source) &= ~(cos::byte(1) << index);
  } else {
    (*source) |= cos::byte(1) << index;
  }

  return status;
}

page_allocator::page_allocator(allocated_physical_pages *pages)
    : pages(pages) {}

cos::byte *page_allocator::allocate_pages(cos::uint64_t page_count) {
  if (page_count == 0) {
    return nullptr; // ????? why???
  }

  // Yes, I'm aware this can be made faster by looking through bits directly
  // I will not be doing this, I care more about this being readable than fast
  for (cos::uint64_t i = 0; i < pages->page_count(); i++) {
    auto large_enough = true;

    for (cos::uint64_t j = 0; j < page_count; j++) {
      if ((*pages)[i + j] != physical_page_status::free) {
        large_enough = false;
        break;
      }
    }

    if (large_enough) {
      for (cos::uint64_t j = 0; j < page_count; j++) {
        (*pages)[i + j] = physical_page_status::used;
      }
      return pages->base_physical_address() + i * 4096;
    }
  }

  return nullptr;
}

cos::byte *page_allocator::allocate_memory(cos::uint64_t memory_size) {
  return allocate_pages((memory_size + 4095) / 4096);
}

} // namespace cos
