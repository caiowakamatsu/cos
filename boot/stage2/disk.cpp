#include "disk.hpp"
#include "types.hpp"

namespace port {
// port io bullshittery
void write_byte(cos::uint16_t port, cos::uint8_t value) {
  asm volatile("outb %0, %1" ::"a"(value), "Nd"(port));
}

[[nodiscard]] cos::uint8_t read_byte(cos::uint16_t port) {
  cos::uint8_t ret = 0;
  asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

void write_short(cos::uint16_t port, cos::uint16_t value) {
  asm volatile("outw %0, %1" ::"a"(value), "Nd"(port));
}

[[nodiscard]] cos::uint16_t read_short(cos::uint16_t port) {
  cos::uint16_t ret = 0;
  asm volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
  return ret;
}

enum : cos::uint16_t {
  ATA_PRIMARY_IO = 0x1F0,
  ATA_PRIMARY_CTL = 0x3F6,
  ATA_SECONDARY_IO = 0x170,
  ATA_SECONDARY_CTL = 0x376,
};

enum : cos::uint8_t {
  ATA_REG_DATA = 0,
  ATA_REG_ERROR_FEAT = 1,
  ATA_REG_SECCNT = 2,
  ATA_REG_LBA0 = 3,
  ATA_REG_LBA1 = 4,
  ATA_REG_LBA2 = 5,
  ATA_REG_DRIVE_HEAD = 6,
  ATA_REG_STATUS_CMD = 7,
};

enum : cos::uint8_t {
  ATA_SR_BSY = 0x80,
  ATA_SR_DRDY = 0x40,
  ATA_SR_DF = 0x20,
  ATA_SR_DRQ = 0x08,
  ATA_SR_ERR = 0x01,
};

enum : cos::uint8_t {
  ATA_CMD_READ_SECTORS = 0x20,
};

} // namespace port

namespace {
void ata_io_wait(cos::uint16_t ctrl) {
  [[maybe_unused]] auto _ = port::read_byte(ctrl);
  _ = port::read_byte(ctrl);
  _ = port::read_byte(ctrl);
  _ = port::read_byte(ctrl);
}

cos::uint8_t ata_poll_ready(cos::uint16_t io, cos::uint16_t ctrl, bool need_drq,
                            cos::uint32_t timeout) {
  [[maybe_unused]] auto first_read =
      port::read_byte(io + port::ATA_REG_STATUS_CMD);

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

  return 3; // temporary
}
} // namespace

namespace cos {
cos::uint8_t read_from_disk(cos::uint32_t sector_source,
                            cos::uint8_t sector_count, cos::byte *target,
                            bool secondary, bool slave) {
  if (sector_count == 0) {
    return 1;
  }
  if (sector_source & 0xF000'0000u) {
    return 2;
  }

  const auto addr_int = reinterpret_cast<cos::uint32_t>(target);

  const auto io = secondary ? port::ATA_SECONDARY_IO : port::ATA_PRIMARY_IO;
  const auto ctrl = secondary ? port::ATA_SECONDARY_CTL : port::ATA_PRIMARY_CTL;

  // Select the drive
  const auto drive = (slave ? 0xF0 : 0xE0) | ((sector_source >> 24 & 0x0F));
  port::write_byte(io + port::ATA_REG_DRIVE_HEAD, drive);
  ata_io_wait(ctrl);

  port::write_byte(ctrl, 0x02); // disable IRQ

  // Program sector count + LBA stuff
  port::write_byte(io + port::ATA_REG_SECCNT, sector_count);
  port::write_byte(io + port::ATA_REG_LBA0,
                   static_cast<cos::uint32_t>((sector_source >> 0) & 0xFF));
  port::write_byte(io + port::ATA_REG_LBA1,
                   static_cast<cos::uint32_t>((sector_source >> 8) & 0xFF));
  port::write_byte(io + port::ATA_REG_LBA2,
                   static_cast<cos::uint32_t>((sector_source >> 16) & 0xFF));

  // isssue read command
  port::write_byte(io + port::ATA_REG_STATUS_CMD, port::ATA_CMD_READ_SECTORS);

  auto data = reinterpret_cast<cos::uint16_t *>(target);
  for (cos::uint16_t sector = 0; sector < sector_count; sector++) {
    if (const auto poll_result = ata_poll_ready(io, ctrl, true, 1000);
        poll_result != 0) {
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

} // namespace cos
