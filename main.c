#include "image_utils.h"
#include <stdio.h>
#include <omp.h>

#define NUM_THREADS 8

int main() {
    Image image;
    if (!load_bmp("images/sample1.bmp", &image)) {
        fprintf(stderr, "Error loading image\n");
        return 1;
    }

    const double ST = omp_get_wtime();
    omp_set_num_threads(NUM_THREADS);

    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
                convertir_a_grises(&image, "img_gray.bmp");

            #pragma omp section
                crear_espejo_horizontal(&image, "img_espejo_h.bmp");

            #pragma omp section
                crear_espejo_vertical(&image, "img_espejo_v.bmp");

            #pragma omp section
                blur(&image, 21, "img_blur.bmp");

            #pragma omp section
                blur(&image, 21, "img_blur.bmp");
        }
    }
    // blur(&image, 21, "img_blur.bmp");
    // blur(&image, 21, "img_blur.bmp");
    const double STOP = omp_get_wtime();
    printf("Tiempo = %lf \n", (STOP - ST));

    free_image(&image);
    return 0;
}
