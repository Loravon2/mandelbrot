#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <array>
#include <iostream>
#include <array>

#define EPSILON 0.000000001
//#define MAX_IT  5000
#define KEY_POINTS 5


const std::array<double, KEY_POINTS> x_key = {
  0.0,
  0.16,
  0.42,
  0.6425,
  1.01
};

const std::array<uint8_t, 3 * KEY_POINTS> keys = {
  0, 7, 100,
  32, 107, 203,
  237, 255, 255,
  255, 170, 0,
  255, 255, 255,
};

// pre-calculated color function
const std::array<double, 3 * KEY_POINTS> m = {
  200, 625, 643.75, 
  494.231, 597.115, 421.875, 
  242.697, 0, 0, 
  0, 0, 0, 
  0, 231.293, 693.878, 
};

std::array<uint8_t, 4> calcCol(double hue) {
  int keyval = 0;
  for (int i = 0; i < KEY_POINTS - 1; i++) {
    if (hue < x_key[i+1]) {
      keyval = i;
      break;
    }
  }

  double delta = x_key[keyval + 1] - x_key[keyval];
  double t = (hue - x_key[keyval]) / delta;

  std::array<uint8_t, 4> result;

  for (int i = 0; i < 3; i++) {
    double res = keys[3 * keyval + i] * (1 + 2*t) * (1 - t) * (1 - t) + delta * m[3 * keyval + i] * t * (1 - t) * (1 - t) + keys[3 * (keyval + 1) + i] * t * t * (3 - 2 * t) + delta * m[3 * (keyval + 1) + i] * t * t * (t - 1);
    result[i] = (uint8_t) res;
  }

  result[3] = 255;

  return result;
}


int findIter(int px, int py, size_t width, size_t height, double x_offset, double y_offset, double x_maxx, double y_maxx, size_t max_it) {
  double sx = (double) (px + 0.5) * ((x_maxx - x_offset) / (double) width) + x_offset;
  double sy = (double) (py + 0.5) * ((y_maxx - y_offset) / (double) height) + y_offset;

  double q = (sx - 0.25) * (sx - 0.25) + sy * sy;
  if (q * (q + (sx - 0.25)) <= 0.25 * sy * sy or (sx + 1) * (sx + 1) + sy * sy <= 1.0 / 16.0) {
    return max_it;
  }

  double x = 0;
  double y = 0;
  double x2 = 0;
  double y2 = 0;
  double w = 0;
  int it = 0;

  double xold = 0;
  double yold = 0;
  int period = 0;

  while (x2 + y2 <= (1 << 16) && it < max_it) {

    x = x2 - y2 + sx;
    y = w - x2 - y2 + sy;
    x2 = x * x;
    y2 = y * y;
    w = (x + y) * (x + y);

    it++;

    if (std::abs(xold - x) < EPSILON && std::abs(yold - y) < EPSILON) {
      it = max_it;
      break;
    }

    period++;
    if (period > 20) {
      period = 0;
      xold = x;
      yold = y;
    }
  }

  return it;
}

void turnRight(std::array<int, 2>& dir) {
  int buf = dir[0];
  dir[0] = -dir[1];
  dir[1] = buf;
}

void turnLeft(std::array<int, 2>& dir) {
  int buf = dir[0];
  dir[0] = dir[1];
  dir[1] = - buf;
}

void turnAround(std::array<int, 2>& dir) {
  dir[0] = -dir[0];
  dir[1] = -dir[1];
}

void trace(int px, int py, size_t width, size_t height, double x_offset, double y_offset, double x_maxx, double y_maxx, uint16_t* iterCount, size_t max_it) {
  int col = findIter(px, py, width, height, x_offset, y_offset, x_maxx, y_maxx, max_it);

  std::array<int, 2> dir = {1, 0};
  int x = px;
  int y = py;

  bool firstIt = true;
  bool fill = false;

  while(true) {
    if (not firstIt) {
      if (x == px && y == py) {
        if (fill) {
          return;
        }
        fill = true;
        dir = {1, 0};
      }
    }

    firstIt = false;

    if(not fill && iterCount[x + width * y] == 0) {
      iterCount[x + width * y] = col;
    }

    // fill line to the right when going up
    if(fill && dir[0] == 0 && dir[1] == -1) {
      for (int xx = x; xx < width; xx++) {
        if (iterCount[xx + width * y] == 0) {
          iterCount[xx + width * y] = findIter(xx, y, width, height, x_offset, y_offset, x_maxx, y_maxx, max_it);
          continue;
        }
        if (iterCount[xx + width * y] != col) {
          break;
        }
      }
    }

    // check left
    if (0 <= x + dir[1] && x + dir[1] < width && 0 <= y - dir[0] && y - dir[0] < height) {
      int w = iterCount[x + dir[1] + width * (y - dir[0])];
      
      if (w == 0) {
        w = findIter(x + dir[1], y - dir[0], width, height, x_offset, y_offset, x_maxx, y_maxx, max_it);
        iterCount[x + dir[1] + width * (y - dir[0])] = w;
      }
      if (col == w) {
        turnLeft(dir);
        x += dir[0];
        y += dir[1];
        continue;
      }
    }

    // check ahead
    if (0 <= x + dir[0] && x + dir[0] < width && 0 <= y + dir[1] && y + dir[1] < height) {
      int w = iterCount[x + dir[0] + width * (y + dir[1])];

      if (w == 0) {
        w = findIter(x + dir[0], y + dir[1], width, height, x_offset, y_offset, x_maxx, y_maxx, max_it);
        iterCount[x + dir[0] + width * (y + dir[1])] = w;
      }
      if (col == w) {
        x += dir[0];
        y += dir[1];
        continue;
      }
    }

    // check right
    if (0 <= x - dir[1] && x - dir[1] < width && 0 <= y + dir[0] && y + dir[0] < height) {
      int w = iterCount[x - dir[1] + width * (y + dir[0])];
      
      if (w == 0) {
        w = findIter(x - dir[1], y + dir[0], width, height, x_offset, y_offset, x_maxx, y_maxx, max_it);
        iterCount[x - dir[1] + width * (y + dir[0])] = w;
      }
      if (col == w) {
        turnRight(dir);
        x += dir[0];
        y += dir[1];
        continue;
      }
    }

    if (x == px && y == py && dir[0] == 1 && dir[1] == 0) {
      return;
    }

    // turn around
    turnAround(dir);
    x += dir[0];
    y += dir[1];
  }
}

void firstPass(size_t width, size_t height, double x_offset, double y_offset, double x_maxx, double y_maxx, uint16_t* iterCount, size_t max_it) {
  for (int py = 0; py < height; py++) {
    for (int px = 0; px < width; px++) {
      if (iterCount[px + width * py] == 0) {
        trace(px, py, width, height, x_offset, y_offset, x_maxx, y_maxx, iterCount, max_it);
        
        
        //iterCount[px + width * py] = findIter(px, py, width, height, x_offset, y_offset, x_maxx, y_maxx);
      }
    }
  }
}

void secondPass(size_t width, size_t height, const uint16_t* iterCount, uint16_t* iterPerPixel, const size_t max_it) {
  for (int py = 0; py < height; py++) {
    for (int px = 0; px < width; px++) {
      if (iterCount[px + width * py] < max_it) {
        int i = iterCount[px + width * py];
        iterPerPixel[i]++;
      }
    }
  }
}

int thirdPass(const uint16_t* iterPerPixel, const size_t max_it) {
  int total = 0;
  for (int i = 0; i < max_it; i++) {
    total += iterPerPixel[i];
  }

  return total;
}

void fourthPass(size_t width, size_t height, const uint16_t* iterCount, const uint16_t* iterPerPixel, const int total, double* hue) {
  for (int py = 0; py < height; py++) {
    for (int px = 0; px < width; px++) {
      hue[px + width * py] = 0;
      int it = iterCount[px + width * py];

      for (int i = 0; i < it; i++) {
        hue[px + width * py] += (float) iterPerPixel[i] / (float) total;
      }
    }
  }
}

extern "C" {
  void renderHunByHun(size_t width, size_t height, double x_offset, double y_offset, double x_maxx, double y_maxx, size_t max_it, uint8_t* output) {
    uint16_t* iterCount = (uint16_t *) malloc(sizeof(uint16_t) * width * height);
    uint16_t* iterPerPixel = (uint16_t *) malloc(sizeof(uint16_t) * max_it);
    double* hue = (double *) malloc(sizeof(double) * width * height);

    if (iterCount == nullptr or iterPerPixel == nullptr or hue == nullptr) {
      std::cout << "ABORT -- Out of memory" << std::endl;
      return;
    }

    for (int i = 0; i < width * height; i++) {
      iterCount[i] = 0;
    }

    for (int i = 0; i < max_it; i++) {
      iterPerPixel[i] = 0;
    }

    firstPass(width, height, x_offset, y_offset, x_maxx, y_maxx, iterCount, max_it);
    secondPass(width, height, iterCount, iterPerPixel, max_it);
    int total = thirdPass(iterPerPixel, max_it);
    fourthPass(width, height, iterCount, iterPerPixel, total, hue);

    for (int py = 0; py < height; py++) {
      for (int px = 0; px < width; px++) {
        if (iterCount[px + width * py] == max_it) {
          for (int i = 0; i < 3; i++) {
            output[4 * px + 4 * width * py + i] = 0;
          }
          output[4 * px + 4 * width * py + 3] = 255;
          continue;
        }

        std::array<uint8_t, 4> rgb = calcCol(hue[px + width * py]);

        for (int i = 0; i < 4; i++) {
          output[4 * px + 4 * width * py + i] = rgb[i];
        }
      }
    }

    free(iterCount);
    free(iterPerPixel);
    free(hue);
  }
}

int main() {
  // uint8_t * col_buf = (uint8_t*) malloc(sizeof(uint8_t) * 1500 * 1000 * 4);

  // renderHunByHun(1500, 1000, -3, -1.5, 1.5, 1.5, col_buf);

  // free(col_buf);
  return 0;
}
