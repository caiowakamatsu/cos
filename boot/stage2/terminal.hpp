#ifndef COS_TERMINAL_HPP
#define COS_TERMINAL_HPP

namespace cos {

struct memory_size {
  explicit memory_size(unsigned long long value) : value(value) {}

  operator unsigned long long() { return value; }

  unsigned long long value = 0;
};

struct hex {
  explicit hex(unsigned long long value) : value(value) {}

  operator unsigned long long() { return value; }

  unsigned long long value = 0;
};

struct decimal {
  explicit decimal(unsigned long long value) : value(value) {}

  operator unsigned long long() { return value; }

  unsigned long long value = 0;
};

class terminal {
  constexpr static char width = 80;
  constexpr static char height = 25;

public:
  explicit terminal(char *data) : data(data) { clear(); }

  void clear() {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        data[index(x, y) + 0] = ' ';
        data[index(x, y) + 1] = 0x07;
      }
    }
    cx = 0;
    cy = 0;
  }

  void write(char c) {
    if (c == '\n') {
      // New line
      if (cy + 1 == height) {
        cycle_up();
      } else {
        cy += 1;
      }

      cx = 0;
      return;
    }

    if (cx + 1 == width) {
      // We need to go down, but what if we're already at the bottom?
      if (cy + 1 == height) {
        cycle_up(); // Move everything up one before writing
      } else {
        cy += 1;
      }
      cx = 0;
    }

    data[index(cx++, cy)] = c;
  }

  terminal &operator<<(const char *str) {
    auto current = 0;
    while (str[current] != 0) {
      this->write(str[current++]);
    }

    return *this;
  }

  terminal &operator<<(memory_size value) {
    print_memory_size(value);
    return *this;
  }

  terminal &operator<<(hex value) {
    print_hex(value);
    return *this;
  }

  terminal &operator<<(decimal value) {
    print_decimal(value);
    return *this;
  }

private:
  char cx = 0;
  char cy = 0;

  void print_memory_size(unsigned long long size) {
    const char *tables[6];
    tables[0] = "b";
    tables[1] = "kb";
    tables[2] = "mb";
    tables[3] = "gb";
    tables[4] = "tb";
    tables[5] = "pb";

    auto order = 0;
    while (size > 1024 && order < 6) {
      size /= 1024;
      order += 1;
    }

    (*this) << decimal(size) << tables[order];
  }

  void print_hex(unsigned long long value) {
    (*this) << "0x";
    if (value == 0) {
      this->write('0');
    }

    char buffer[16];
    int i = 0;

    while (value != 0) {
      int digit = value & 0xF;
      buffer[i++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
      value >>= 4;
    }

    for (int j = i - 1; j >= 0; j--) {
      write(buffer[j]);
    }
  }

  void print_decimal(unsigned long long value_big) {
    // we need to move to u32 for the div
    auto value = static_cast<unsigned int>(value_big);

    if (value == 0) {
      this->write('0');
      return;
    }

    char buffer[20]; // A 64-bit unsigned long long can have at most 20 digits.
    int i = 0;

    // Extract digits in reverse order
    while (value != 0) {
      buffer[i++] =
          (value % 10) + '0'; // Get the last digit and convert to char
      value /= 10;            // Remove the last digit
    }

    // Print digits in the correct order (from buffer)
    while (i > 0) {
      this->write(buffer[--i]);
    }
  }

  [[nodiscard]] int index(int x, int y) { return (x + y * width) * 2; }

  void cycle_up() {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        data[index(x, y - 1)] = data[index(x, y)];
        data[index(x, y)] = ' ';
      }
    }
    cx = 0;
  }

  volatile char *data = nullptr;
};
} // namespace cos

#endif
