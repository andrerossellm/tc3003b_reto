#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "image_utils.h"

// --------- ESCALA DE GRISES ----------
void convertir_a_grises(const char *inputPath, const char *outputPath) {
    FILE *in = fopen(inputPath, "rb");
    if (!in) {
        perror("Error abriendo imagen de entrada");
        return;
    }

    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        fclose(in);
        return;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, in);
    fwrite(header, sizeof(unsigned char), 54, out);

    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int size = *(int *)&header[34];

    unsigned char *data = (unsigned char *)malloc(size);
    fread(data, sizeof(unsigned char), size, in);

    int rowSize = (width * 3 + 3) & (~3);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width * 3; j += 3) {
            int idx = i * rowSize + j;
            unsigned char blue = data[idx];
            unsigned char green = data[idx + 1];
            unsigned char red = data[idx + 2];
            unsigned char gray = (red + green + blue) / 3;
            data[idx] = data[idx + 1] = data[idx + 2] = gray;
        }
    }

    fwrite(data, sizeof(unsigned char), size, out);
    free(data);
    fclose(in);
    fclose(out);
}

// --------- ESPEJO VERTICAL ----------
void crear_espejo_vertical(const char *inputPath, const char *outputPath) {
    int width, height, header_size;
    unsigned char *header = NULL;

    Pixel **pixels = read_bmp(inputPath, &width, &height, &header, &header_size);
    if (!pixels) {
        fprintf(stderr, "Error al leer imagen para espejo vertical\n");
        return;
    }

    flip_vertical_pixels(pixels, height);
    write_bmp(outputPath, pixels, width, height, header);

    free(header);
    free_pixels(pixels, height);
}

// --------- ESPEJO HORIZONTAL ----------
void crear_espejo_horizontal(const char *inputPath, const char *outputPath) {
    int width, height, header_size;
    unsigned char *header = NULL;

    Pixel **pixels = read_bmp(inputPath, &width, &height, &header, &header_size);
    if (!pixels) {
        fprintf(stderr, "Error al leer imagen para espejo horizontal\n");
        return;
    }

    flip_hrizntal_pixels(pixels, height, width);
    write_bmp(outputPath, pixels, width, height, header);

    free(header);
    free_pixels(pixels, height);
}

// --------- BLUR ----------
void blur(unsigned short kernelSize, struct imageMetadata imageData) {
    char inputFilename[1028];
    sprintf(inputFilename, "%s/%s", imageData.directory, imageData.name);

    FILE *originalImage = fopen(inputFilename, "rb");
    if (!originalImage) {
        fprintf(stderr, "No se pudo abrir la imagen original: %s\n", inputFilename);
        return;
    }

    char sufix[64];
    sprintf(sufix, "_blurred_%d.bmp", kernelSize);
    char outputFilename[1028];
    sprintf(outputFilename, "%s/%s%s", imageData.directory, imageData.name, sufix);

    FILE *outputImage = fopen(outputFilename, "wb");

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, originalImage);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    unsigned char *pixelData = (unsigned char *)malloc(imageData.imageSize);
    fread(pixelData, imageData.imageSize, 1, originalImage);

    int kernelRadius = (kernelSize - 1) / 2;
    int widthWithPadding = (imageData.width * 3 + 3) & -4;

    for (int y = 0; y < imageData.height; y++) {
        for (int x = 0; x < imageData.width; x++) {
            unsigned int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            for (int ky = -kernelRadius; ky <= kernelRadius; ky++) {
                for (int kx = -kernelRadius; kx <= kernelRadius; kx++) {
                    int i = y + ky;
                    int j = x + kx;

                    if (i < 0 || i >= imageData.height || j < 0 || j >= imageData.width)
                        continue;

                    int index = (i * widthWithPadding) + (j * 3);
                    bSum += pixelData[index];
                    gSum += pixelData[index + 1];
                    rSum += pixelData[index + 2];
                    count++;
                }
            }

            int index = (y * widthWithPadding) + (x * 3);
            pixelData[index]     = bSum / count;
            pixelData[index + 1] = gSum / count;
            pixelData[index + 2] = rSum / count;
        }
    }

    fwrite(pixelData, imageData.imageSize, 1, outputImage);
    fclose(outputImage);
    fclose(originalImage);
    free(pixelData);
}

// --------- FLIP HORIZONTAL (PIXELS) ----------
void flip_hrizntal_pixels(Pixel **P, int height, int width) {
    Pixel *R = (Pixel *)malloc(width * sizeof(Pixel));
    for (int i = 0; i < height; i++) {
        Pixel *I0 = P[i];
        for (int j = 0; j < width; j++) {
            R[j] = I0[width - j - 1];
        }
        memcpy(I0, R, width * sizeof(Pixel));
    }
    free(R);
}

// --------- FLIP VERTICAL (PIXELS) ----------
void flip_vertical_pixels(Pixel **P, int height) {
    for (int i = 0; i < height / 2; i++) {
        Pixel *temp = P[i];
        P[i] = P[height - i - 1];
        P[height - i - 1] = temp;
    }
}

// --------- LECTURA BMP ----------
Pixel **read_bmp(const char *filename, int *width, int *height, unsigned char **header, int *header_size) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Error opening BMP file");
        return NULL;
    }

    unsigned char file_header[54];
    fread(file_header, sizeof(unsigned char), 54, f);
    *header = malloc(54);
    memcpy(*header, file_header, 54);
    *header_size = 54;

    *width = *(int *)&file_header[18];
    *height = *(int *)&file_header[22];

    int row_padded = (*width * 3 + 3) & (~3);
    unsigned char *data = (unsigned char *)malloc(row_padded * (*height));
    fread(data, sizeof(unsigned char), row_padded * (*height), f);
    fclose(f);

    Pixel **pixels = (Pixel **)malloc(*height * sizeof(Pixel *));
    for (int i = 0; i < *height; i++) {
        pixels[i] = (Pixel *)malloc(*width * sizeof(Pixel));
        for (int j = 0; j < *width; j++) {
            int idx = i * row_padded + j * 3;
            pixels[i][j].blue = data[idx];
            pixels[i][j].green = data[idx + 1];
            pixels[i][j].red = data[idx + 2];
        }
    }
    free(data);
    return pixels;
}

// --------- ESCRITURA BMP ----------
void write_bmp(const char *filename, Pixel **pixels, int width, int height, unsigned char *header) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Error writing BMP file");
        return;
    }

    int row_padded = (width * 3 + 3) & (~3);
    unsigned char *row = (unsigned char *)calloc(row_padded, sizeof(unsigned char));

    fwrite(header, sizeof(unsigned char), 54, f);

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            row[j * 3] = pixels[i][j].blue;
            row[j * 3 + 1] = pixels[i][j].green;
            row[j * 3 + 2] = pixels[i][j].red;
        }
        fwrite(row, sizeof(unsigned char), row_padded, f);
    }

    free(row);
    fclose(f);
}

// --------- LIBERAR PIXELS ----------
void free_pixels(Pixel **pixels, int height) {
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}
