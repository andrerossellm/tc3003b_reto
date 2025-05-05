#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int load_bmp(const char *filename, Image *img) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("Error abriendo archivo BMP");
        return 0;
    }

    img->header = malloc(54);
    fread(img->header, sizeof(unsigned char), 54, f);

    img->width = *(int *)&img->header[18];
    img->height = *(int *)&img->header[22];
    img->row_padded = (img->width * 3 + 3) & (~3);
    img->data_size = img->row_padded * img->height;

    img->data = malloc(img->data_size);
    fread(img->data, sizeof(unsigned char), img->data_size, f);

    fclose(f);
    return 1;
}

void free_image(Image *img) {
    free(img->header);
    free(img->data);
}

// ---------- GRAYSCALE ----------
void convertir_a_grises(const Image *image, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *grayData = malloc(image->data_size);
    memcpy(grayData, image->data, image->data_size);

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width * 3; j += 3) {
            int idx = i * image->row_padded + j;
            unsigned char blue = grayData[idx];
            unsigned char green = grayData[idx + 1];
            unsigned char red = grayData[idx + 2];
            unsigned char gray = (red + green + blue) / 3;
            grayData[idx] = grayData[idx + 1] = grayData[idx + 2] = gray;
        }
    }

    fwrite(grayData, sizeof(unsigned char), image->data_size, out);
    free(grayData);
    fclose(out);
}

// ---------- HORIZONTAL MIRROR ----------
void crear_espejo_horizontal(const Image *image, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *mirrorData = malloc(image->data_size);
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int idx = i * image->row_padded + j * 3;
            int midx = i * image->row_padded + (image->width - j - 1) * 3;
            mirrorData[midx] = image->data[idx];
            mirrorData[midx + 1] = image->data[idx + 1];
            mirrorData[midx + 2] = image->data[idx + 2];
        }
    }

    fwrite(mirrorData, sizeof(unsigned char), image->data_size, out);
    free(mirrorData);
    fclose(out);
}

// ---------- VERTICAL MIRROR ----------
void crear_espejo_vertical(const Image *image, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *mirrorData = malloc(image->data_size);
    for (int i = 0; i < image->height; i++) {
        int dst_row = image->height - 1 - i;
        memcpy(&mirrorData[dst_row * image->row_padded], &image->data[i * image->row_padded], image->row_padded);
    }

    fwrite(mirrorData, sizeof(unsigned char), image->data_size, out);
    free(mirrorData);
    fclose(out);
}

// ---------- GRAYSCALE HORIZONTAL MIRROR ----------
void convertir_grises_y_espejo_horizontal(const Image *image, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *processedData = malloc(image->data_size);

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int idx = i * image->row_padded + j * 3;
            unsigned char blue = image->data[idx];
            unsigned char green = image->data[idx + 1];
            unsigned char red = image->data[idx + 2];
            unsigned char gray = (red + green + blue) / 3;
            processedData[idx] = processedData[idx + 1] = processedData[idx + 2] = gray;
        }
    }

    unsigned char *finalData = malloc(image->data_size);
    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int src_idx = i * image->row_padded + j * 3;
            int dst_idx = i * image->row_padded + (image->width - j - 1) * 3;
            finalData[dst_idx] = processedData[src_idx];
            finalData[dst_idx + 1] = processedData[src_idx + 1];
            finalData[dst_idx + 2] = processedData[src_idx + 2];
        }
    }

    fwrite(finalData, sizeof(unsigned char), image->data_size, out);
    free(processedData);
    free(finalData);
    fclose(out);
}

// ---------- GRAYSCALE VERTICAL MIRROR ----------
void convertir_grises_y_espejo_vertical(const Image *image, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *processedData = malloc(image->data_size);

    for (int i = 0; i < image->height; i++) {
        for (int j = 0; j < image->width; j++) {
            int idx = i * image->row_padded + j * 3;
            unsigned char blue = image->data[idx];
            unsigned char green = image->data[idx + 1];
            unsigned char red = image->data[idx + 2];
            unsigned char gray = (red + green + blue) / 3;
            processedData[idx] = processedData[idx + 1] = processedData[idx + 2] = gray;
        }
    }

    unsigned char *finalData = malloc(image->data_size);
    for (int i = 0; i < image->height; i++) {
        int dst_row = image->height - 1 - i;
        memcpy(&finalData[dst_row * image->row_padded], &processedData[i * image->row_padded], image->row_padded);
    }

    fwrite(finalData, sizeof(unsigned char), image->data_size, out);
    free(processedData);
    free(finalData);
    fclose(out);
}


// ---------- BLUR ----------
void blur(const Image *image, int kernelSize, const char *outputPath) {
    FILE *out = fopen(outputPath, "wb");
    if (!out) {
        perror("Error creando imagen de salida");
        return;
    }

    fwrite(image->header, sizeof(unsigned char), 54, out);

    unsigned char *blurred = malloc(image->data_size);
    memcpy(blurred, image->data, image->data_size);

    int radius = kernelSize / 2;

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int rSum = 0, gSum = 0, bSum = 0, count = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                for (int kx = -radius; kx <= radius; kx++) {
                    int i = y + ky, j = x + kx;
                    if (i < 0 || i >= image->height || j < 0 || j >= image->width) continue;
                    int idx = i * image->row_padded + j * 3;
                    bSum += image->data[idx];
                    gSum += image->data[idx + 1];
                    rSum += image->data[idx + 2];
                    count++;
                }
            }

            int idx = y * image->row_padded + x * 3;
            blurred[idx] = bSum / count;
            blurred[idx + 1] = gSum / count;
            blurred[idx + 2] = rSum / count;
        }
    }

    fwrite(blurred, sizeof(unsigned char), image->data_size, out);
    free(blurred);
    fclose(out);
}
