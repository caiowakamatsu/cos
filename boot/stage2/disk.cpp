#include "disk.hpp"

namespace port {
// port io bullshittery
void write_byte(std::uint16_t port, std::uint8_t value) { asm volatile("outb %0, %1" ::"a"(value), "Nd"(port)); }

[[nodiscard]] std::uint8_t read_byte(std::uint16_t port) {
	std::uint8_t ret = 0;
	asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

void write_short(std::uint16_t port, std::uint16_t value) { asm volatile("outw %0, %1" ::"a"(value), "Nd"(port)); }

[[nodiscard]] std::uint16_t read_short(std::uint16_t port) {
	std::uint16_t ret = 0;
	asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
	return ret;
}

constexpr std::uint16_t ATA_PRIMARY_IO = 0x1F0;
constexpr std::uint16_t ATA_PRIMARY_CTL = 0x3F6;
constexpr std::uint16_t ATA_SECONDARY_IO = 0x170;
constexpr std::uint16_t ATA_SECONDARY_CTL = 0x376;

constexpr std::uint8_t ATA_REG_DATA = 0;
constexpr std::uint8_t ATA_REG_ERROR_FEAT = 1;
constexpr std::uint8_t ATA_REG_SECCNT = 2;
constexpr std::uint8_t ATA_REG_LBA0 = 3;
constexpr std::uint8_t ATA_REG_LBA1 = 4;
constexpr std::uint8_t ATA_REG_LBA2 = 5;
constexpr std::uint8_t ATA_REG_DRIVE_HEAD = 6;
constexpr std::uint8_t ATA_REG_STATUS_CMD = 7;

constexpr std::uint8_t ATA_SR_BSY = 0x80;
constexpr std::uint8_t ATA_SR_DRDY = 0x40;
constexpr std::uint8_t ATA_SR_DF = 0x20;
constexpr std::uint8_t ATA_SR_DRQ = 0x08;
constexpr std::uint8_t ATA_SR_ERR = 0x01;

constexpr std::uint8_t ATA_CMD_READ_SECTORS = 0x20;

}  // namespace port

namespace {
void ata_io_wait(std::uint16_t ctrl) {
	[[maybe_unused]] auto _ = port::read_byte(ctrl);
	_ = port::read_byte(ctrl);
	_ = port::read_byte(ctrl);
	_ = port::read_byte(ctrl);
}

std::uint8_t ata_poll_ready(std::uint16_t io, std::uint16_t ctrl, bool need_drq, std::uint32_t timeout) {
	[[maybe_unused]] auto first_read = port::read_byte(io + port::ATA_REG_STATUS_CMD);

	auto attempts = 0;
	while (attempts++ < timeout) {
		const auto st = port::read_byte(io + port::ATA_REG_STATUS_CMD);
		if (st & port::ATA_SR_ERR) {
			return port::read_byte(io + port::ATA_REG_ERROR_FEAT);
		}

		if (st & port::ATA_SR_DF) {
			return port::ATA_SR_DF;
		}

		if (st & port::ATA_SR_BSY) {
			continue;
		}

		if (need_drq) {
			if (st & port::ATA_SR_DRQ) {
				return 0;
			}
		} else {
			return 0;
		}
	}

	return 3;  // temporary
}
}  // namespace

namespace cos {
std::uint8_t read_from_disk(std::uint32_t sector_source, std::uint8_t sector_count, std::byte *target, bool secondary,
							bool slave) {
	if (sector_count == 0) {
		return 1;
	}
	if (sector_source & 0xF000'0000u) {
		return 2;
	}

	const auto addr_int = reinterpret_cast<std::uint32_t>(target);

	const auto io = secondary ? port::ATA_SECONDARY_IO : port::ATA_PRIMARY_IO;
	const auto ctrl = secondary ? port::ATA_SECONDARY_CTL : port::ATA_PRIMARY_CTL;

	// Select the drive
	const auto drive = (slave ? 0xF0 : 0xE0) | ((sector_source >> 24 & 0x0F));
	port::write_byte(io + port::ATA_REG_DRIVE_HEAD, drive);
	ata_io_wait(ctrl);

	port::write_byte(ctrl, 0x02);  // disable IRQ

	// Program sector count + LBA stuff
	port::write_byte(io + port::ATA_REG_SECCNT, sector_count);
	port::write_byte(io + port::ATA_REG_LBA0, static_cast<std::uint32_t>((sector_source >> 0) & 0xFF));
	port::write_byte(io + port::ATA_REG_LBA1, static_cast<std::uint32_t>((sector_source >> 8) & 0xFF));
	port::write_byte(io + port::ATA_REG_LBA2, static_cast<std::uint32_t>((sector_source >> 16) & 0xFF));

	// isssue read command
	port::write_byte(io + port::ATA_REG_STATUS_CMD, port::ATA_CMD_READ_SECTORS);

	auto data = reinterpret_cast<std::uint16_t *>(target);
	for (std::uint16_t sector = 0; sector < sector_count; sector++) {
		if (const auto poll_result = ata_poll_ready(io, ctrl, true, 1000); poll_result != 0) {
			return poll_result;
		}

		for (int i = 0; i < 256; i++) {
			data[i] = port::read_short(io + port::ATA_REG_DATA);
		}
		data += 256;

		// wait a bit between sector reads
		ata_io_wait(ctrl);
	}

	// enable IRQ again
	port::write_byte(ctrl, 0x00);

	return 0;
}

}  // namespace cos
