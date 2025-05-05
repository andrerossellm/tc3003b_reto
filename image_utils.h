#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

typedef struct {
    unsigned char red, green, blue;
} Pixel;

typedef struct {
    unsigned char *header;
    unsigned char *data;
    int width;
    int height;
    int row_padded;
    int data_size;
} Image;

int load_bmp(const char *filename, Image *img);
void free_image(Image *img);

void convertir_a_grises(const Image *image, const char *outputPath);
void crear_espejo_horizontal(const Image *image, const char *outputPath);
void crear_espejo_vertical(const Image *image, const char *outputPath);
void convertir_grises_y_espejo_horizontal(const Image *image, const char *outputPath);
void convertir_grises_y_espejo_vertical(const Image *image, const char *outputPath);
void blur(const Image *image, int kernelSize, const char *outputPath);

#endif
