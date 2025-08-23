#ifndef COS_TERMINAL_HPP
#define COS_TERMINAL_HPP

namespace cos {
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

  terminal &operator<<(unsigned long long number) {
    (*this) << "0x";
    if (number == 0) {
      this->write('0');
      return *this;
    }

    char buffer[16];
    int i = 0;

    while (number != 0) {
      int digit = number & 0xF;
      buffer[i++] = digit < 10 ? '0' + digit : 'A' + (digit - 10);
      number >>= 4;
    }

    for (int j = i - 1; j >= 0; j--) {
      write(buffer[j]);
    }

    return *this;
  }

private:
  char cx = 0;
  char cy = 0;

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
