#ifndef COS_FILESYSTEM_HPP
#define COS_FILESYSTEM_HPP

#include <types.hpp>

#include "terminal.hpp"

namespace cos::filesystem {

struct entry {
	std::uint32_t sector_start = 0;
	std::uint32_t sector_count = 0;
};
[[nodiscard]] entry find(const char *identifier);

};	// namespace cos::filesystem

#endif
