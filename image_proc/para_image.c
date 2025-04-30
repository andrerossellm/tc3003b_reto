#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "selec_proc.h"

int main(){
    FILE *fptr;
    char data[80] = "arc1.txt";
    fptr = fopen(data, "w");
    if (fptr == NULL){
        printf("Error\n");
        exit(1);
    }
    fprintf(fptr, "Ejemplo escribir\n");
    fprintf(fptr, "Emmanuel Torres Rios");
    fclose(fptr);
    /*fam.bmp, prueba1.bmp, sample.bmp, sample1.bmp */
    inv_img("inv_1" ,"./img/sample1.bmp");
    inv_img_color("inv_color_1" ,"./img/sample1.bmp");
    return 0;
}