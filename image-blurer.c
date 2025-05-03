#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct imageMetadata
{
    int width;
    int height;
    int imageSize;
    char name[512];
    char directory[512];  // Aquí guardamos la ruta del directorio de la imagen
};

void blur(unsigned short kernelSize, struct imageMetadata imageData)
{
    // Usamos la ruta completa de la imagen original (con su directorio y nombre de archivo)
    char inputFilename[1028];
    strcpy(inputFilename, imageData.directory);
    strcat(inputFilename, "/");
    strcat(inputFilename, imageData.name);
    strcat(inputFilename, "");
    
    FILE *originalImage = fopen(inputFilename, "rb");
    if (!originalImage)
    {
        fprintf(stderr, "No se pudo abrir la imagen original: %s\n", inputFilename);
        return;
    }

    // Creamos el nombre del archivo de salida (en el mismo directorio de la imagen original)
    char sufix[64];
    sprintf(sufix, "_blurred_%d.bmp", kernelSize);
    char outputFilename[1028];
    strcpy(outputFilename, imageData.directory);
    strcat(outputFilename, "/");
    strcat(outputFilename, imageData.name);
    strcat(outputFilename, sufix);

    FILE *outputImage = fopen(outputFilename, "wb");

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, originalImage);
    fwrite(header, sizeof(unsigned char), 54, outputImage);

    unsigned char *pixelData = (unsigned char *)malloc(imageData.imageSize);
    fread(pixelData, imageData.imageSize, 1, originalImage);

    int kernelRadius = (kernelSize - 1) / 2;
    int widthWithPadding = (imageData.width * 3 + 3) & -4;

    for (int y = 0; y < imageData.height; y++)
    {
        for (int x = 0; x < imageData.width; x++)
        {
            unsigned int rSum = 0, gSum = 0, bSum = 0;
            int count = 0;

            for (int ky = -kernelRadius; ky <= kernelRadius; ky++)
            {
                for (int kx = -kernelRadius; kx <= kernelRadius; kx++)
                {
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

int main(int argc, char *argv[])
{
    char *filename = NULL;
    int kernelSize = 55;
    struct imageMetadata imageData;

    // Procesar los argumentos
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <imagen.bmp>\n", argv[0]);
        return 1;
    }

    filename = argv[1];

    // Extraer el directorio y el nombre de la imagen
    char *tmp = strrchr(filename, '/');
    
    if (tmp)
    {
        strncpy(imageData.directory, filename, tmp - filename);
        imageData.directory[tmp - filename] = '\0';
        strcpy(imageData.name, tmp + 1); // Obtener solo el nombre de la imagen sin el directorio
    }
    else
    {
        strcpy(imageData.directory, ".");
        strcpy(imageData.name, filename); // Si no hay directorio, se asume que está en el directorio actual
    }

    // Abrir la imagen original
    FILE *originalImage = fopen(filename, "rb");
    if (!originalImage)
    {
        fprintf(stderr, "No se pudo abrir el archivo %s\n", filename);
        return 1;
    }

    unsigned char header[54];
    fread(header, sizeof(unsigned char), 54, originalImage);
    int width = *(int *)&header[18];
    int height = *(int *)&header[22];
    int imageSize = *(int *)&header[34];

    imageData.width = width;
    imageData.height = height;
    imageData.imageSize = imageSize;

    fclose(originalImage);

    blur(kernelSize, imageData);
    return 0;
}
