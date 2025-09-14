#include "filesystem.hpp"

#include <memory.hpp>
#include <string.hpp>
#include <types.hpp>

#include "disk.hpp"

namespace cos::filesystem {

entry find(const char *identifier) {
	char file_data[512];

	if (const auto status = cos::read_from_disk(1, 1, reinterpret_cast<std::byte *>(file_data)); status != 0) {
		return {};
	}

	auto file_count = std::uint16_t();
	auto total_sectors = std::uint16_t();

	std::memcpy(reinterpret_cast<std::byte *>(&file_count), reinterpret_cast<std::byte *>(file_data), 2);
	std::memcpy(reinterpret_cast<std::byte *>(&total_sectors), reinterpret_cast<std::byte *>(file_data + 2), 2);

	for (std::uint16_t i = 0; i < file_count; i++) {
		const auto file_entry_sector = i / 8 + 2;
		const auto file_entry_read_shift = 64 * (i % 8);

		if (const auto status = cos::read_from_disk(file_entry_sector, 1, reinterpret_cast<std::byte *>(file_data));
			status != 0) {
			return {
				.sector_start = 0,
				.sector_count = 0,
			};
		}

		if (std::streq(identifier, file_data + file_entry_read_shift)) {
			auto result = entry();

			std::memcpy(reinterpret_cast<std::byte *>(&result.sector_start),
						reinterpret_cast<std::byte *>(file_data + file_entry_read_shift + 32), 4);
			std::memcpy(reinterpret_cast<std::byte *>(&result.sector_count),
						reinterpret_cast<std::byte *>(file_data + file_entry_read_shift + 36), 4);

			return result;
		}
	}

	return {};
}

}  // namespace cos::filesystem
