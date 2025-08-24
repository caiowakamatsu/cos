#ifndef COS_BOOT_HPP
#define COS_BOOT_HPP

#include "physical_page.hpp"

namespace cos {

struct boot_info {
  cos::allocated_physical_pages pages;
};

}; // namespace cos

#endif
