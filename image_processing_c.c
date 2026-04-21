#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <omp.h>

#include "ppm.h"

// Image from:
// http://7-themes.com/6971875-funny-flowers-pictures.html

typedef struct {
  int x, y;
  float *red;
  float *green;
  float *blue;
} AccurateImage;

// Convert ppm to high precision format.
AccurateImage *convertToAccurateImage(PPMImage *image) {
  // Make a copy
  AccurateImage *imageAccurate;
  imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));

  int img_size = image->x * image->y;
  imageAccurate->red = malloc(img_size * sizeof(float));
  imageAccurate->green = malloc(img_size * sizeof(float));
  imageAccurate->blue = malloc(img_size * sizeof(float));

#pragma omp parallel for
  for (int i = 0; i < image->x * image->y; i++) {
    imageAccurate->red[i] = image->data[i].red;
    imageAccurate->green[i] = image->data[i].green;
    imageAccurate->blue[i] = image->data[i].blue;
  }
  imageAccurate->x = image->x;
  imageAccurate->y = image->y;

  return imageAccurate;
}

void blurHorizontal(float *out, float *in, int w, int h, int r) {
#pragma omp parallel for
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {

      float sum = 0;
      int count = 0;

      for (int dx = -r; dx <= r; dx++) {
        int xx = x + dx;
        if (xx < 0 || xx >= w)
          continue;

        sum += in[y * w + xx];
        count++;
      }

      out[y * w + x] = sum / count;
    }
  }
}

void blurVertical(float *out, float *in, int w, int h, int r) {
#pragma omp parallel for
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {

      float sum = 0;
      int count = 0;

      for (int dy = -r; dy <= r; dy++) {
        int yy = y + dy;
        if (yy < 0 || yy >= h)
          continue;

        sum += in[yy * w + x];
        count++;
      }

      out[y * w + x] = sum / count;
    }
  }
}
// blur one color channel
void blurIteration(float *out, float *in, int w, int h, int r) {
  float *temp = malloc(w * h * sizeof(float));

  blurHorizontal(temp, in, w, h, r);
  blurVertical(out, temp, w, h, r);

  free(temp);
}
void runBlur(AccurateImage *in, AccurateImage *out, int size, int iterations) {
  int w = in->x;
  int h = in->y;

  for (int iter = 0; iter < iterations; iter++) {

    blurIteration(out->red, in->red, w, h, size);
    blurIteration(out->green, in->green, w, h, size);
    blurIteration(out->blue, in->blue, w, h, size);

    // swap pointers
    float *tmp;
    tmp = in->red;
    in->red = out->red;
    out->red = tmp;
    tmp = in->green;
    in->green = out->green;
    out->green = tmp;
    tmp = in->blue;
    in->blue = out->blue;
    out->blue = tmp;
  }
}

PPMImage *imageDifference(AccurateImage *a, AccurateImage *b) {
  int n = a->x * a->y;

  PPMImage *out = malloc(sizeof(PPMImage));
  out->x = a->x;
  out->y = a->y;
  out->data = malloc(n * sizeof(PPMPixel));

#pragma omp parallel for
  for (int i = 0; i < n; i++) {
    double vals[3] = {b->red[i] - a->red[i], b->green[i] - a->green[i],
                      b->blue[i] - a->blue[i]};

    unsigned char *channels[3] = {&out->data[i].red, &out->data[i].green,
                                  &out->data[i].blue};

    for (int c = 0; c < 3; c++) {
      double value = vals[c];

      if (value > 255)
        *channels[c] = 255;
      else if (value < -1.0) {
        value = 257.0 + value;
        *channels[c] = (value > 255) ? 255 : (unsigned char)floor(value);
      } else if (value < 0.0) {
        *channels[c] = 0;
      } else {
        *channels[c] = (unsigned char)floor(value);
      }
    }
  }

  return out;
}

int main(int argc, char **argv) {
  // read image
  PPMImage *image;
  // select where to read the image from
  if (argc > 1) {
    // from file for debugging (with argument)
    image = readPPM("flower.ppm");
  } else {
    // from stdin for cmb
    image = readStreamPPM(stdin);
  }

  omp_set_num_threads(4);
  // Tiny
  AccurateImage *tiny1 = convertToAccurateImage(image);
  AccurateImage *tiny2 = convertToAccurateImage(image);
  runBlur(tiny1, tiny2, 2, 5);

  // Small
  AccurateImage *small1 = convertToAccurateImage(image);
  AccurateImage *small2 = convertToAccurateImage(image);
  runBlur(small1, small2, 3, 5);

  // Medium
  AccurateImage *medium1 = convertToAccurateImage(image);
  AccurateImage *medium2 = convertToAccurateImage(image);
  runBlur(medium1, medium2, 5, 5);

  // Large
  AccurateImage *large1 = convertToAccurateImage(image);
  AccurateImage *large2 = convertToAccurateImage(image);
  runBlur(large1, large2, 8, 5);

  // calculate difference
  // Note: result is in first buffer (odd iterations)
  PPMImage *final_tiny = imageDifference(tiny1, small1);
  PPMImage *final_small = imageDifference(small1, medium1);
  PPMImage *final_medium = imageDifference(medium1, large1);

  // Save the images.
  if (argc > 1) {
    writePPM("flower_tiny.ppm", final_tiny);
    writePPM("flower_small.ppm", final_small);
    writePPM("flower_medium.ppm", final_medium);
  } else {
    writeStreamPPM(stdout, final_tiny);
    writeStreamPPM(stdout, final_small);
    writeStreamPPM(stdout, final_medium);
  }
}
