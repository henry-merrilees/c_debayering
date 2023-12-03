#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define VERTICAL_RESOLUTION 30
#define HORIZONTAL_RESOLUTION 40

typedef struct {
  uint8_t r, g, b, a;
} rgba_t;

// NOTE: ROW MAJOR
typedef uint8_t pixel_buf_Type[VERTICAL_RESOLUTION][HORIZONTAL_RESOLUTION];
typedef rgba_t color_pixel_buf_Type[VERTICAL_RESOLUTION][HORIZONTAL_RESOLUTION];

typedef enum { Red, Blue, GreenBesideRed, GreenBesideBlue } color_t;
typedef enum { Left, Center, Right } lateral_t;
typedef enum { Top, Middle, Bottom } vertical_t;


pixel_buf_Type pixel_buf;

// hanlde out of bounds cases
void edge_handler(uint8_t pixels_in[3][3], lateral_t lateral,
                  vertical_t vertical, uint8_t pixels_out[3][3]) {
  // Copy the input to output as a baseline
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      pixels_out[i][j] = pixels_in[i][j];
    }
  }

  // Handle lateral cases
  switch (lateral) {
  case Left:
    pixels_out[0][0] = pixels_in[0][2];
    pixels_out[1][0] = pixels_in[1][2];
    pixels_out[2][0] = pixels_in[2][2];
    break;
  case Right:
    pixels_out[0][2] = pixels_in[0][0];
    pixels_out[1][2] = pixels_in[1][0];
    pixels_out[2][2] = pixels_in[2][0];
    break;
  case Center:
    // No changes for the center
    break;
  }

  // Handle vertical cases
  switch (vertical) {
  case Top:
    pixels_out[0][0] = pixels_in[0][0];
    pixels_out[0][1] = pixels_in[0][1];
    pixels_out[0][2] = pixels_in[0][2];
    break;
  case Bottom:
    pixels_out[2][0] = pixels_in[0][0];
    pixels_out[2][1] = pixels_in[0][1];
    pixels_out[2][2] = pixels_in[0][2];
    break;
  case Middle:
    // No changes for the middle
    break;
  }
}

// The filter3x3 function
rgba_t filter3x3(uint8_t pixels_in[3][3], lateral_t lateral,
                 vertical_t vertical, color_t color) {
  uint8_t pixels[3][3];
  rgba_t pixel_out;

  edge_handler(pixels_in, lateral, vertical, pixels);

  pixel_out.r = pixel_out.g = pixel_out.b = 0;
  pixel_out.a = 0xff; // opaque

  // Demosaic logic
  switch (color) {
  case Red:
    pixel_out.r = pixels[1][1];
    pixel_out.g =
        (pixels[1][2] + pixels[2][1] + pixels[0][1] + pixels[1][0]) / 4;
    pixel_out.b =
        (pixels[2][2] + pixels[2][0] + pixels[0][2] + pixels[0][0]) / 4;
    break;
  case Blue:
    pixel_out.r =
        (pixels[2][2] + pixels[2][0] + pixels[0][2] + pixels[0][0]) / 4;
    pixel_out.g =
        (pixels[1][2] + pixels[2][1] + pixels[0][1] + pixels[1][0]) / 4;
    pixel_out.b = pixels[1][1];
    break;
  case GreenBesideRed:
    pixel_out.r = (pixels[1][2] + pixels[1][0]) / 2;
    pixel_out.g = pixels[1][1];
    pixel_out.b = (pixels[2][1] + pixels[0][1]) / 2;
    break;
  case GreenBesideBlue:
    pixel_out.r = (pixels[2][1] + pixels[0][1]) / 2;
    pixel_out.g = pixels[1][1];
    pixel_out.b = (pixels[1][2] + pixels[1][0]) / 2;
    break;
  }
  return pixel_out;
}

void debayer(pixel_buf_Type *input, color_pixel_buf_Type *output) {
  rgba_t pixel_out;
  uint8_t pixels_in[3][3];
  uint8_t pixels_out[3][3];

  // current output pixel coordinates
  for (int j = 0; j < VERTICAL_RESOLUTION; ++j) {
    for (int i = 0; i < HORIZONTAL_RESOLUTION; ++i) {

      // Copy the filter input to the 3x3 array
      for (int k = 0; k < 3; ++k) {
        for (int l = 0; l < 3; ++l) {
          int x = i + k - 1;
          int y = j + l - 1;

          pixels_in[l][k] =
              (*input)[y % VERTICAL_RESOLUTION][x % HORIZONTAL_RESOLUTION];
        }
      }

      vertical_t vertical;
      lateral_t lateral;

      switch (i) {
      case (0):
        lateral = Left;
        break;
      case (HORIZONTAL_RESOLUTION - 1):
        lateral = Right;
        break;
      default:
        lateral = Center;
        break;
      }

      switch (j) {
      case (0):
        vertical = Top;
        break;
      case (VERTICAL_RESOLUTION - 1):
        vertical = Bottom;
        break;
      default:
        vertical = Middle;
        break;
      }

      color_t color;

      int x_even = i % 2 == 0;
      int y_even = j % 2 == 0;

      if (x_even && y_even) {
        color = Red;
      } else if (!x_even && y_even) {
        color = GreenBesideRed;
      } else if (x_even && !y_even) {
        color = GreenBesideBlue;
      } else {
        color = Blue;
      }

      // Call the filter
      pixel_out = filter3x3(pixels_in, lateral, vertical, color);
      (*output)[j][i] = pixel_out;
    }
  }
}

int main() {
  pixel_buf_Type input_image;
  color_pixel_buf_Type output_image;

  char *line = NULL;
  for (int j = 0; j < VERTICAL_RESOLUTION; ++j) {
    for (int i = 0; i < HORIZONTAL_RESOLUTION; ++i) {

      size_t len = 0;
      ssize_t read = getline(&line, &len, stdin);
      if (read == -1)
        return 1;

      input_image[j][i] = (uint8_t)strtol(line, NULL, 16);
    }
  }

  debayer(&input_image, &output_image);

  // write out the image to std out one line at a time, as hex encoded bytes....
  for (int j = 0; j < VERTICAL_RESOLUTION; ++j) {
    for (int i = 0; i < HORIZONTAL_RESOLUTION; ++i) {
      rgba_t px = output_image[j][i];
      printf("%02hhx%02hhx%02hhx%02hhx\n", px.r, px.g, px.b, px.a);
    }
  }

  return 0;
}
