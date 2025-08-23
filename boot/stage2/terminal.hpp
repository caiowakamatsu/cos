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
  }

  void write(const char *str) {
    cycle_up();
    auto current_x = 0;
    while (current_x < width && str[current_x] != 0) {
      data[index(current_x, height - 1)] = str[current_x];
      current_x += 1;
    }
  }

private:
  [[nodiscard]] int index(int x, int y) { return (x + y * width) * 2; }

  void cycle_up() {
    for (int x = 0; x < width; x++) {
      for (int y = 0; y < height; y++) {
        data[index(x, y - 1)] = data[index(x, y)];
        data[index(x, y)] = ' ';
      }
    }
  }

  volatile char *data = nullptr;
};
} // namespace cos

#endif
