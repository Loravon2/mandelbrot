#include <array>
#include <cstdint>
#include <cmath>
#include <iostream>

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
  255, 255, 255
};

// Position = 0.0     Color = (  0,   7, 100)
// Position = 0.16    Color = ( 32, 107, 203)
// Position = 0.42    Color = (237, 255, 255)
// Position = 0.6425  Color = (255, 170,   0)
// Position = 0.8575  Color = (  0,   2,   0)

std::array<double, 3 * KEY_POINTS> interpolate() {
  std::array<double, 3 * KEY_POINTS> m;
  
  for (int channel = 0; channel < 3; channel ++) {
    std::array<double, KEY_POINTS - 1> delta;
    for (int i = 0; i < KEY_POINTS - 1; i++) {
      delta[i] = ((double) (keys[3 * (i+1) + channel] - keys[3 * i + channel]) / (x_key[i+1] - x_key[i]));
    }

    for (int i = 1; i < KEY_POINTS - 1; i++) {
      if(delta[i-1] * delta[i] < 0) {
        m[3 * i + channel] = 0;
      }
      else {
        m[3 * i + channel] = (delta[i - 1] + delta[i]) / 2;
      }
    }

    m[0 + channel] = delta[0];
    m[3 * (KEY_POINTS - 1) + channel] = delta[KEY_POINTS - 2];

    for (int i = 0; i < KEY_POINTS - 1; i++) {
      if (std::abs(delta[i]) < 0.000001) {
        m[3 * i + channel] = 0;
        m[3 * (i+1) + channel] = 0;
        continue;
      }

      double alpha = m[3 * i + channel] / delta[i];
      double beta = m[3 * i+1 + channel] / delta[i];

      if (alpha < 0) {
        m[3 * i + channel] = 0;
        continue;
      }

      if (beta < 0) {
        m[3 * (i+1) + channel] = 0;
        continue;
      }

      if ((alpha * alpha + beta * beta) > 9) {
        double theta = 3 / sqrtf(alpha * alpha + beta * beta);

        m[3 * i + channel] = theta * alpha * delta[i];
        m[3 * (i+1) + channel] = theta * beta * delta[i];
      }
    }
  }

  return m;
}

std::array<uint8_t, 3> interpolatedFunc(std::array<double, 3 * KEY_POINTS> m, double x) {
  int keyval = 0;
  for (int i = 0; i < KEY_POINTS - 1; i++) {
    if (x < x_key[i+1]) {
      keyval = i;
      break;
    }
  }

  double delta = x_key[keyval + 1] - x_key[keyval];
  double t = (x - x_key[keyval]) / delta;

  std::array<uint8_t, 3> result;

  for (int i = 0; i < 3; i++) {
    double res = keys[3 * keyval + i] * (1 + 2*t) * (1 - t) * (1 - t) + delta * m[3 * keyval + i] * t * (1 - t) * (1 - t) + keys[3 * (keyval + 1) + i] * t * t * (3 - 2 * t) + delta * m[3 * (keyval + 1) + i] * t * t * (t - 1);
    result[i] = (uint8_t) res;
  }

  return result;
}

int main() {
  std::array<double, 3 * KEY_POINTS> m = interpolate();

  std::cout << "m-values to export" << std::endl;
  for (int j = 0; j < KEY_POINTS; j++) {
    for (int i = 0; i < 3; i++) {
      std::cout << m[3 * j + i] << ", ";
    }
    std::cout << "\n";
  }
  std::cout << std::endl;

  std::array<uint8_t, 3> rgb = interpolatedFunc(m, 0.8);

  std::cout << "Example function call:" << std::endl;
  std::cout << int(rgb[0]) << " " << int(rgb[1]) << " " << int(rgb[2]) << std::endl;

  return 0;
}