#include "image_utils.h"
#include <string.h>

int main() {

    //convertir_a_grises("images/sample1.bmp", "img_gris.bmp");
    struct imageMetadata imgData;
    strcpy(imgData.directory, "images");
    strcpy(imgData.name, "sample1.bmp");
    imgData.width = 3840;
    imgData.height = 2160;
    imgData.imageSize = imgData.width * imgData.height * 3;

    blur(21, imgData); 

    // crear_espejo_horizontal("images/sample1.bmp", "img_espejo_h.bmp");
    // crear_espejo_vertical("images/sample1.bmp", "img_espejo_v.bmp");


    return 0;

}