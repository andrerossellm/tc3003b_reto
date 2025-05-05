#ifndef IMAGE_UTILS_H
#define IMAGE_UTILS_H

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

struct imageMetadata {
    char name[256];
    char directory[256];
    int width;
    int height;
    int imageSize;
};

void convertir_a_grises(const char *inputPath, const char *outputPath);
void crear_espejo_vertical(const char *inputPath, const char *outputPath);
void crear_espejo_horizontal(const char *inputPath, const char *outputPath);
void blur(unsigned short kernelSize, struct imageMetadata imageData);

Pixel **read_bmp(const char *filename, int *width, int *height, unsigned char **header, int *header_size);
void write_bmp(const char *filename, Pixel **pixels, int width, int height, unsigned char *header);
void free_pixels(Pixel **pixels, int height);

// Funciones internas
void flip_vertical_pixels(Pixel **P, int height);
void flip_hrizntal_pixels(Pixel **P, int height, int width);

#endif
