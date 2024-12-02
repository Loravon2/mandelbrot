#include <cstdint>
#include <cstdlib>
#include <array>
#include <iostream>

const std::array<uint8_t, 5 * 4> COL_TABLE = {
  0, 0, 0, 255,
  40, 40, 40, 255,
  100, 100, 100, 255,
  160, 160, 160, 255,
  255, 255, 255, 255
};


std::uint8_t* renderHunnByHun() {
  uint8_t* col_buf = (uint8_t*) malloc(sizeof(uint8_t) * 4 * 100 * 100);

  for (int py = 0; py < 100; py++) {
    for (int px = 0; px < 100; px++) {
      double sx = (double) px / 33.3;
      double sy = (double) py / 33.3;

      double x = 0;
      double y = 0;
      int it = 0;
      int maxit = 1000;

      while (x * x + y * y <= 2 * 2 && it < maxit) {
        double xtemp = x * x - y * y + sx;
        y = 2 * x * y + sy;
        x = xtemp;

        it++;
      }

      for (int i = 0; i < 4; i++) {
        col_buf[i + 4 * px + 4 * 100 * py] = COL_TABLE[i + 4 * (it / 200)];
      }
    }
  }

  return col_buf;
}

int main() {
  std::uint8_t* col_buf = renderHunnByHun();

  for (int i = 0; i < 100 * 100; i++) {
    std::cout << (int) col_buf[i] << " " << (int) col_buf[i + 1] << " " << (int) col_buf[i + 2] << " " << (int) col_buf[i + 3] << "\n";
  }

  std::cout << std::endl;

  return 0;
}