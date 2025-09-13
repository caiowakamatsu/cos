#ifndef COS_DISK_HPP
#define COS_DISK_HPP

#include <types.hpp>

namespace cos {

enum class disk_read_status {
	ok,
	err,
	df,
	timeout,
	sec_count,
	sec_source,
};
[[nodiscard]] std::uint8_t read_from_disk(std::uint32_t sector_source, std::uint8_t sector_count, std::byte *target,
										  bool secondary = false, bool slave = false);

}  // namespace cos

#endif
