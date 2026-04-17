#include <iostream>
#include <fstream>
#include <stdlib.h>


using namespace std;


#include<stdio.h>
#include<stdlib.h>


#define CREATOR "RPFELGUEIRAS"
#define RGB_COMPONENT_COLOR 255

typedef struct {
     unsigned char red,green,blue;
} PPMPixel;

typedef struct {
     int x, y;
     PPMPixel *data;
} PPMImage;


PPMImage *readStreamPPM(FILE *fp) {
	char buff[16];
	PPMImage *image;
	int c, rgb_comp_color;
	//open PPM file for reading
	if (!fp) {
		//fprintf(stderr, "Unable to open file '%s'\n", filename);
		exit(1);
	}

	//read image format
	if (!fgets(buff, sizeof(buff), fp)) {
		//perror(filename);
		exit(1);
	}
	 
	//check the image format
	if (buff[0] != 'P' || buff[1] != '6') {
		fprintf(stderr, "Invalid image format (must be 'P6')\n");
		exit(1);
	}

	//alloc memory form image
	image = (PPMImage *)malloc(sizeof(PPMImage));
	if (!image) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}

	//check for comments
	c = getc(fp);
	while (c == '#') {
	while (getc(fp) != '\n') ;
		 c = getc(fp);
	}

	ungetc(c, fp);
	//read image size information
	if (fscanf(fp, "%d %d", &image->x, &image->y) != 2) {
		fprintf(stderr, "Invalid image size (error loading )\n");
		exit(1);
	}

	//read rgb component
	if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
		fprintf(stderr, "Invalid rgb component (error loading )\n");
		exit(1);
	}

	//check rgb component depth
	if (rgb_comp_color!= 255) {
		fprintf(stderr, " does not have 8-bits components\n");
		exit(1);
	}

	while (fgetc(fp) != '\n') ;
	//memory allocation for pixel data
	image->data = (PPMPixel*)malloc(image->x * image->y * sizeof(PPMPixel));

	if (!image) {
		fprintf(stderr, "Unable to allocate memory\n");
		exit(1);
	}

	//read pixel data from file
	if (fread(image->data, 3 * image->x, image->y, fp) != image->y) {
		fprintf(stderr, "Error loading image\n");
		exit(1);
	}
	return image;
}


PPMImage *readPPM(const char *filename)
{
         char buff[16];
         PPMImage *img;
         FILE *fp;
         int c, rgb_comp_color;
         //open PPM file for reading
         fp = fopen(filename, "rb");
         if (!fp) {
              fprintf(stderr, "Unable to open file '%s'\n", filename);
              exit(1);
         }

         //read image format
         if (!fgets(buff, sizeof(buff), fp)) {
              perror(filename);
              exit(1);
         }

    //check the image format
    if (buff[0] != 'P' || buff[1] != '6') {
         fprintf(stderr, "Invalid image format (must be 'P6')\n");
         exit(1);
    }

    //alloc memory form image
    img = (PPMImage *)malloc(sizeof(PPMImage));
    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //check for comments
    c = getc(fp);
    while (c == '#') {
    while (getc(fp) != '\n') ;
         c = getc(fp);
    }

    ungetc(c, fp);
    //read image size information
    if (fscanf(fp, "%d %d", &img->x, &img->y) != 2) {
         fprintf(stderr, "Invalid image size (error loading '%s')\n", filename);
         exit(1);
    }

    //read rgb component
    if (fscanf(fp, "%d", &rgb_comp_color) != 1) {
         fprintf(stderr, "Invalid rgb component (error loading '%s')\n", filename);
         exit(1);
    }

    //check rgb component depth
    if (rgb_comp_color!= RGB_COMPONENT_COLOR) {
         fprintf(stderr, "'%s' does not have 8-bits components\n", filename);
         exit(1);
    }

    while (fgetc(fp) != '\n') ;
    //memory allocation for pixel data
    img->data = (PPMPixel*)malloc(img->x * img->y * sizeof(PPMPixel));

    if (!img) {
         fprintf(stderr, "Unable to allocate memory\n");
         exit(1);
    }

    //read pixel data from file
    if (fread(img->data, 3 * img->x, img->y, fp) != img->y) {
         fprintf(stderr, "Error loading image '%s'\n", filename);
         exit(1);
    }

    fclose(fp);
    return img;
}


void writeStreamPPM(FILE *fp, PPMImage *img) {
	if (!fp) {
		fprintf(stderr, "Unable to open file\n");
		exit(1);
	}

	//write the header file
	//image format
	fprintf(fp, "P6\n");

	//comments
	fprintf(fp, "# Created by %s\n",CREATOR);

	//image size
	fprintf(fp, "%d %d\n",img->x,img->y);

	// rgb component depth
	fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

	// pixel data
	fwrite(img->data, 3 * img->x, img->y, fp);
}

void writePPM(const char *filename, PPMImage *img)
{
    FILE *fp;
    //open file for output
    fp = fopen(filename, "wb");
    if (!fp) {
         fprintf(stderr, "Unable to open file '%s'\n", filename);
         exit(1);
    }

    //write the header file
    //image format
    fprintf(fp, "P6\n");

    //comments
    fprintf(fp, "# Created by %s\n",CREATOR);

    //image size
    fprintf(fp, "%d %d\n",img->x,img->y);

    // rgb component depth
    fprintf(fp, "%d\n",RGB_COMPONENT_COLOR);

    // pixel data
    fwrite(img->data, 3 * img->x, img->y, fp);
    fclose(fp);
}

void changeColorPPM(PPMImage *img)
{
    int i;
    if(img){

         for(i=0;i<img->x*img->y;i++){
              img->data[i].red=RGB_COMPONENT_COLOR-img->data[i].red;
              img->data[i].green=RGB_COMPONENT_COLOR-img->data[i].green;
              img->data[i].blue=RGB_COMPONENT_COLOR-img->data[i].blue;
         }
    }
}

// Compute the final image
int testImage(PPMImage *compare, PPMImage *correct) {
	
	int counDiffA[1024];
	for(int i = 0; i < 1024; i++) {
		counDiffA[i] = 0;
	}
	int *counDiff = &counDiffA[512];
	
	if(correct->x * correct->y != compare->x * compare->y) {
		exit(1);
	}
	
	for(int i = 0; i < correct->x * correct->y; i++) {
		counDiff[(int)(compare->data[i].red - correct->data[i].red)]++;
		counDiff[(int)(compare->data[i].green - correct->data[i].green)]++;
		counDiff[(int)(compare->data[i].blue - correct->data[i].blue)]++;
	}
	
	int requiredCorrect = correct->x * correct->y * 3;
	int correctPixels = counDiff[0];
	//printf("Correct pixles: %d\n", correctPixels);

	int singleErrors = counDiffA[512-1] + counDiffA[512+1];
	//printf("Pixles, single errors: %d\n", singleErrors);
	if(singleErrors > 2000) {
		exit(1);
	}
	
	int multiErrors = requiredCorrect - (correctPixels + singleErrors);
	//printf("Pixles, multiple errors: %d\n", multiErrors);
	if(multiErrors > 201) {
		exit(1);
	}
	return 0;
}


int main(int argc , char ** argv){
	
	PPMImage *imageTinyCorrect;
	PPMImage *imageSmallCorrect;
	PPMImage *imageMediumCorrect;
	
	PPMImage *imageTiny;
	PPMImage *imageSmall;
	PPMImage *imageMedium;
	
	if(argc != 4) {
		/*
		imageTinyCorrect = readPPM("flower_tiny_correct.ppm");
		imageSmallCorrect = readPPM("flower_small_correct.ppm");
		imageMediumCorrect = readPPM("flower_medium_correct.ppm");
		imageTiny = readPPM("flower_tiny.ppm");
		imageSmall = readPPM("flower_small.ppm");
		imageMedium = readPPM("flower_medium.ppm");
		*/
		printf("Wrong number of arguments to checker.cpp\n");
	} else {
		FILE *fp;// = stdin;
		//printf("argv[2]: %s\n", argv[2]);
		fp = fopen(argv[2], "rb");
		if (!fp) {
			fprintf(stderr, "Unable to open file '%s'\n", argv[2]);
			exit(1);
		}
		
		imageTiny = readStreamPPM(fp);
		imageSmall = readStreamPPM(fp);
		imageMedium = readStreamPPM(fp);
		fclose(fp);
		
		//printf("argv[3]: %s\n", argv[3]);
		fp = fopen(argv[3], "rb");
		if (!fp) {
			fprintf(stderr, "Unable to open file '%s'\n", argv[3]);
			exit(1);
		}
		
		imageTinyCorrect = readStreamPPM(fp);
		imageSmallCorrect = readStreamPPM(fp);
		imageMediumCorrect = readStreamPPM(fp);
		fclose(fp);
	}
	
	
	
	int pass = testImage(imageTiny, imageTinyCorrect);
	pass = testImage(imageSmall, imageSmallCorrect);
	pass = testImage(imageMedium, imageMediumCorrect);
	
	return 0;
}





