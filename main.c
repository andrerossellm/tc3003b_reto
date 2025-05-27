#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <mpi.h>
#include <omp.h>
#include <sys/stat.h> 

#define MAX_FILES 100
#define TAG_FILENAME 1
#define TAG_DONE 2
#define NUM_THREADS 10 

long local_read_pixels = 0;
long local_written_images = 0;

void process_image(const char *input_path) {
    omp_set_num_threads(NUM_THREADS);

    Image image;
    if (!load_bmp(input_path, &image)) {
        fprintf(stderr, "Failed to load %s\n", input_path);
        return;
    }

    long image_pixels = image.width * image.height;

    #pragma omp atomic
    local_read_pixels += image_pixels;

    char base_name[256];
    const char *last_slash = strrchr(input_path, '/');
    #ifdef _WIN32
        const char *last_backslash = strrchr(input_path, '\\');
        if (last_backslash && (!last_slash || last_backslash > last_slash)) {
            last_slash = last_backslash;
        }
    #endif

    if (last_slash) {
        strncpy(base_name, last_slash + 1, sizeof(base_name) - 1);
    } else {
        strncpy(base_name, input_path, sizeof(base_name) - 1);
    }
    base_name[sizeof(base_name) - 1] = '\0';

    char *ext = strrchr(base_name, '.');
    if (ext) *ext = '\0';

    char gray_out[512], hmirror_out[512], vmirror_out[512], blur_out[512], gray_hmirror_out[512], gray_vmirror_out[512];
    snprintf(gray_out, sizeof(gray_out), "output/%s_gray.bmp", base_name);
    snprintf(hmirror_out, sizeof(hmirror_out), "output/%s_hmirror.bmp", base_name);
    snprintf(vmirror_out, sizeof(vmirror_out), "output/%s_vmirror.bmp", base_name);
    snprintf(gray_vmirror_out, sizeof(gray_vmirror_out), "output/%s_gray_vmirror.bmp", base_name);
    snprintf(gray_hmirror_out, sizeof(gray_hmirror_out), "output/%s_gray_hmirror.bmp", base_name);
    snprintf(blur_out, sizeof(blur_out), "output/%s_blur.bmp", base_name);

    #pragma omp parallel sections
    {
        #pragma omp section
        convertir_a_grises(&image, gray_out);

        #pragma omp section
        crear_espejo_horizontal(&image, hmirror_out);

        #pragma omp section
        crear_espejo_vertical(&image, vmirror_out);

        #pragma omp section
        convertir_grises_y_espejo_vertical(&image, gray_vmirror_out);

        #pragma omp section
        convertir_grises_y_espejo_horizontal(&image, gray_hmirror_out);

        #pragma omp section
        blur(&image, 21, blur_out);
    }

    #pragma omp atomic
    local_written_images += 6;

    free_image(&image);
}

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <image_directory>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    const char *image_directory = argv[1];

    if (rank == 0) {
        #ifdef _WIN32
            _mkdir("output");
        #else
            mkdir("output", 0777);
        #endif
    }
    MPI_Barrier(MPI_COMM_WORLD);

    double start_time = MPI_Wtime();

    int total_images = 0;
    long total_read_pixels = 0;
    long total_written_images = 0;

    if (rank == 0) {
        DIR *FD;
        struct dirent *in_file;
        char *filenames[MAX_FILES];
        int file_count = 0;

        FD = opendir(image_directory);
        if (FD == NULL) {
            fprintf(stderr, "Error: Failed to open directory '%s' - %s\n", image_directory, strerror(errno));
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        while ((in_file = readdir(FD))) {
            if (!strcmp(in_file->d_name, ".") || !strcmp(in_file->d_name, ".."))
                continue;

            if (strstr(in_file->d_name, ".bmp") != NULL) {
                filenames[file_count] = strdup(in_file->d_name);
                file_count++;
            }
        }
        closedir(FD);

        total_images = file_count;
        int next_file = 0;
        int processed_count = 0; 

        for (int i = 1; i < size && next_file < file_count; i++) {
            MPI_Send(filenames[next_file], strlen(filenames[next_file]) + 1, MPI_CHAR, i, TAG_FILENAME, MPI_COMM_WORLD);
            next_file++;
        }

        while (processed_count < total_images) {
            char done_signal[1];
            MPI_Status status;
            MPI_Recv(done_signal, 1, MPI_CHAR, MPI_ANY_SOURCE, TAG_DONE, MPI_COMM_WORLD, &status);
            int slave = status.MPI_SOURCE;
            processed_count++;
            printf("PROGRESS: %d/%d\n", processed_count, total_images);
            fflush(stdout); 

            if (next_file < file_count) {
                MPI_Send(filenames[next_file], strlen(filenames[next_file]) + 1, MPI_CHAR, slave, TAG_FILENAME, MPI_COMM_WORLD);
                next_file++;
            } else {
                MPI_Send(NULL, 0, MPI_CHAR, slave, TAG_FILENAME, MPI_COMM_WORLD);
            }
        }
        
        for (int i = 1; i < size; i++) {
            MPI_Send(NULL, 0, MPI_CHAR, i, TAG_FILENAME, MPI_COMM_WORLD);
        }


        // Free filenames
        for (int i = 0; i < file_count; i++) {
            free(filenames[i]);
        }

        // Collect stats from slaves
        for (int i = 1; i < size; i++) {
            long slave_pixels, slave_outputs;
            MPI_Recv(&slave_pixels, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Recv(&slave_outputs, 1, MPI_LONG, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_read_pixels += slave_pixels;
            total_written_images += slave_outputs;
        }

        double stop_time = MPI_Wtime();
        double elapsed = stop_time - start_time;
        long written_pixels = total_read_pixels * 6;
        long total_data_bytes = (total_read_pixels * 3) + (written_pixels * 3);


        FILE *fp = fopen("totalLocalitiesScanned.txt", "w");
        if (!fp) {
            fprintf(stderr, "Error: Could not open totalLocalitiesScanned.txt for writing.\n");
        } else {
            fprintf(fp, "Total time: %f seconds\n", elapsed);
            fprintf(fp, "Total read images: %d\n", total_images);
            fprintf(fp, "Total written images: %ld\n", total_written_images);
            fprintf(fp, "Total read pixels: %ld\n", total_read_pixels);
            fprintf(fp, "Total written pixels: %ld\n", written_pixels);
            fprintf(fp, "Pixels processed per second: %.2e\n", (double)(total_read_pixels + written_pixels) / elapsed);
            fprintf(fp, "Data processed per second (bytes/sec): %.2e\n", (double)total_data_bytes / elapsed);
            fprintf(fp, "Estimated Total MIPS: %.2e\n", ((double)total_data_bytes * 20) / (elapsed * 1e6));
            fclose(fp);
        }

    } else {
        // SLAVE PROCESS
        while (1) {
            char filename[256];
            MPI_Status status;
            MPI_Recv(filename, 256, MPI_CHAR, 0, TAG_FILENAME, MPI_COMM_WORLD, &status);

            int count;
            MPI_Get_count(&status, MPI_CHAR, &count);
            if (count == 0) break;

            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", image_directory, filename);
            process_image(filepath);

            char done_signal = 'D';
            MPI_Send(&done_signal, 1, MPI_CHAR, 0, TAG_DONE, MPI_COMM_WORLD);
        }

        MPI_Send(&local_read_pixels, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
        MPI_Send(&local_written_images, 1, MPI_LONG, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}