#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <omp.h>

#define NUM_THREADS 8
#define IMAGE_DIR "images"

int main() {
    DIR* FD;
    struct dirent* in_file;
    char* filenames[100];
    int file_count = 0;
    int written_file_count = 0;

    // Open the directory
    FD = opendir(IMAGE_DIR);
    if (FD == NULL) {
        fprintf(stderr, "Error: Failed to open directory '%s' - %s\n", IMAGE_DIR, strerror(errno));
        return 1;
    }

    // Collect all BMP file names
    while ((in_file = readdir(FD))) {
        if (!strcmp(in_file->d_name, ".") || !strcmp(in_file->d_name, ".."))
            continue;

        // Store the filename
        if (strstr(in_file->d_name, ".bmp") != NULL) {
            filenames[file_count] = strdup(in_file->d_name); 
            file_count++;
        }
    }
    closedir(FD);

    if (file_count == 0) {
        printf("No BMP files found in directory '%s'\n", IMAGE_DIR);
        return 0;
    }

    const double ST = omp_get_wtime();
    omp_set_num_threads(NUM_THREADS);

    // Parallel processing of image files
    #pragma omp parallel for
    for (int i = 0; i < file_count; i++) {
        char input_path[512];
        snprintf(input_path, sizeof(input_path), "%s/%s", IMAGE_DIR, filenames[i]);
    
        Image image;
        if (!load_bmp(input_path, &image)) {
            fprintf(stderr, "Thread %d: Failed to load %s\n", omp_get_thread_num(), input_path);
            continue;
        }
    
        // Remove .bmp extension to get base name
        char base_name[256];
        strncpy(base_name, filenames[i], sizeof(base_name));
        base_name[sizeof(base_name) - 1] = '\0';
        char *ext = strrchr(base_name, '.');
        if (ext != NULL) {
            *ext = '\0'; 
        }

        // Create output path
        char gray_out[512], hmirror_out[512], vmirror_out[512], blur_out[512];
        snprintf(gray_out, sizeof(gray_out), "output/%s_gray.bmp", base_name);
        snprintf(hmirror_out, sizeof(hmirror_out), "output/%s_hmirror.bmp", base_name);
        snprintf(vmirror_out, sizeof(vmirror_out), "output/%s_vmirror.bmp", base_name);
        snprintf(blur_out, sizeof(blur_out), "output/%s_blur.bmp", base_name);
   
        // Call function to transform the image
        convertir_a_grises(&image, gray_out);
        crear_espejo_horizontal(&image, hmirror_out);
        crear_espejo_vertical(&image, vmirror_out);
        blur(&image, 21, blur_out);

        written_file_count += 6;
    
        free_image(&image);
        printf("Thread %d: Processed %s\n", omp_get_thread_num(), filenames[i]);
        free(filenames[i]);
    }

    char *outputTxt = "totalLocalitiesScanned.txt";
    FILE *fp = fopen(outputTxt, "w");
    if (fp == NULL) {
        printf("Error opening the file %s\n", outputTxt);
        return -1;
    }

    fprintf(fp, "Total read localities: %d\n", file_count);
    fprintf(fp, "Total written localities: %d\n", written_file_count);

    fclose(fp); 

    const double STOP = omp_get_wtime();
    printf("Total time = %lf seconds\n", (STOP - ST));

    return 0;
}
