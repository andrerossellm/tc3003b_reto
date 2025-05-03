#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} Pixel;

static void flip_vertical_pixels(Pixel **P, int height) {
    for (int i = 0; i < height / 2; i++) {
        Pixel *temp = P[i];
        P[i] = P[height - i - 1];
        P[height - i - 1] = temp;
    }
}

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

    *width = *(int*)&file_header[18];
    *height = *(int*)&file_header[22];

    int row_padded = (*width * 3 + 3) & (~3);
    unsigned char *data = (unsigned char*)malloc(row_padded * (*height));
    fread(data, sizeof(unsigned char), row_padded * (*height), f);
    fclose(f);

    Pixel **pixels = (Pixel**)malloc(*height * sizeof(Pixel*));
    for (int i = 0; i < *height; i++) {
        pixels[i] = (Pixel*)malloc(*width * sizeof(Pixel));
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

void write_bmp(const char *filename, Pixel **pixels, int width, int height, unsigned char *header) {
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("Error writing BMP file");
        return;
    }

    int row_padded = (width * 3 + 3) & (~3);
    unsigned char *row = (unsigned char*)calloc(row_padded, sizeof(unsigned char));

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

void free_pixels(Pixel **pixels, int height) {
    for (int i = 0; i < height; i++) {
        free(pixels[i]);
    }
    free(pixels);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_image.bmp> <output_directory>\n", argv[0]);
        return 1;
    }

    const char *input_path = argv[1];
    const char *output_dir = argv[2];
    const char *output_name = "vertical.bmp";  // <-- Default name

    // Create output directory if it doesn't exist
    mkdir(output_dir, 0755);

    // Construct full output path: <output_dir>/flipped.bmp
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s", output_dir, output_name);

    int width, height, header_size;
    unsigned char *header = NULL;
    Pixel **image = read_bmp(input_path, &width, &height, &header, &header_size);

    if (!image) {
        fprintf(stderr, "Failed to load image\n");
        return 1;
    }

    flip_vertical_pixels(image, height);
    write_bmp(output_path, image, width, height, header);

    printf("Image flipped and saved to '%s'\n", output_path);

    free(header);
    free_pixels(image, height);

    return 0;
}


