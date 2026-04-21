#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <omp.h>

#include "ppm.h"

// Image from:
// http://7-themes.com/6971875-funny-flowers-pictures.html

typedef struct {
  int x, y;
  double *red;
  double *green;
  double *blue;
} AccurateImage;

// Convert ppm to high precision format.
AccurateImage *convertToAccurateImage(PPMImage *image) {
  // Make a copy
  AccurateImage *imageAccurate;
  imageAccurate = (AccurateImage *)malloc(sizeof(AccurateImage));

  int img_size = image->x * image->y;
  imageAccurate->red = (double *)malloc(img_size * sizeof(double));
  imageAccurate->green = (double *)malloc(img_size * sizeof(double));
  imageAccurate->blue = (double *)malloc(img_size * sizeof(double));

  for (int i = 0; i < image->x * image->y; i++) {
    imageAccurate->red[i] = (double)image->data[i].red;
    imageAccurate->green[i] = (double)image->data[i].green;
    imageAccurate->blue[i] = (double)image->data[i].blue;
  }
  imageAccurate->x = image->x;
  imageAccurate->y = image->y;

  return imageAccurate;
}

// blur one color channel
void blurIteration(double *colourOut, double *colourIn, int width, int height,
                   int size) {

  int numberOfValuesInEachRow = width;

  // Iterate over each pixel
  for (int senterY = 0; senterY < height; senterY++) {
    for (int senterX = 0; senterX < width; senterX++) {

      // For each pixel we compute the magic number
      double sum = 0;
      int countIncluded = 0;
      for (int x = -size; x <= size; x++) {

        for (int y = -size; y <= size; y++) {
          int currentX = senterX + x;
          int currentY = senterY + y;

          if (currentX < 0)
            continue;
          if (currentX >= width)
            continue;
          if (currentY < 0)
            continue;
          if (currentY >= height)
            continue;

          // Now we can begin
          int offsetOfThePixel =
              (numberOfValuesInEachRow * currentY + currentX);
          sum += colourIn[offsetOfThePixel];

          // Keep track of how many values we have included
          countIncluded++;
        }
      }

      // Now we compute the final value
      double value = sum / countIncluded;

      // Update the output image
      int offsetOfThePixel = (numberOfValuesInEachRow * senterY + senterX);
      colourOut[offsetOfThePixel] = value;
    }
  }
}

void runBlur(AccurateImage *in, AccurateImage *out, int size, int iterations) {
  int w = in->x;
  int h = in->y;

  for (int iter = 0; iter < iterations; iter++) {

    blurIteration(out->red, in->red, w, h, size);
    blurIteration(out->green, in->green, w, h, size);
    blurIteration(out->blue, in->blue, w, h, size);

    // swap pointers
    double *tmp;
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

// TODO: pass in colour array instead
// Perform the final step, and return it as ppm.
PPMImage *imageDifference(AccurateImage *imageInSmall,
                          AccurateImage *imageInLarge) {
  PPMImage *imageOut;
  imageOut = (PPMImage *)malloc(sizeof(PPMImage));
  imageOut->data =
      (PPMPixel *)malloc(imageInSmall->x * imageInSmall->y * sizeof(PPMPixel));

  imageOut->x = imageInSmall->x;
  imageOut->y = imageInSmall->y;

  for (int i = 0; i < imageInSmall->x * imageInSmall->y; i++) {
    double value = (imageInLarge->red[i] - imageInSmall->red[i]);
    if (value > 255)
      imageOut->data[i].red = 255;
    else if (value < -1.0) {
      value = 257.0 + value;
      if (value > 255)
        imageOut->data[i].red = 255;
      else
        imageOut->data[i].red = floor(value);
    } else if (value > -1.0 && value < 0.0) {
      imageOut->data[i].red = 0;
    } else {
      imageOut->data[i].red = floor(value);
    }

    value = (imageInLarge->green[i] - imageInSmall->green[i]);
    if (value > 255)
      imageOut->data[i].green = 255;
    else if (value < -1.0) {
      value = 257.0 + value;
      if (value > 255)
        imageOut->data[i].green = 255;
      else
        imageOut->data[i].green = floor(value);
    } else if (value > -1.0 && value < 0.0) {
      imageOut->data[i].green = 0;
    } else {
      imageOut->data[i].green = floor(value);
    }

    value = (imageInLarge->blue[i] - imageInSmall->blue[i]);
    if (value > 255)
      imageOut->data[i].blue = 255;
    else if (value < -1.0) {
      value = 257.0 + value;
      if (value > 255)
        imageOut->data[i].blue = 255;
      else
        imageOut->data[i].blue = floor(value);
    } else if (value > -1.0 && value < 0.0) {
      imageOut->data[i].blue = 0;
    } else {
      imageOut->data[i].blue = floor(value);
    }
  }
  return imageOut;
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
