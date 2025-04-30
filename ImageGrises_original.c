#include <stdio.h>
#include <stdlib.h>

int main()
{
    FILE *image, *outputImage, *lecturas;
    image = fopen("aber.bmp","rb");          //Imagen original a transformar
    outputImage = fopen("img_dd.bmp","wb");    //Imagen transformada

    unsigned char r, g, b;               //Pixel

    for(int i=0; i<54; i++) fputc(fgetc(image), outputImage);   //Copia cabecera a nueva imagen
    while(!feof(image)){                                        //Grises
       b = fgetc(image);
       g = fgetc(image);
       r = fgetc(image);
       
       if (r <= 80 && g <= 80 && b <= 80) {
         r = 255;
         g = 255;
         b = 255;
       }
       
       unsigned char pixel = 0.21*r+0.72*g+0.07*b;
       fputc(b, outputImage);
       fputc(g, outputImage);
       fputc(r, outputImage);
    }

    fclose(image);
    fclose(outputImage);
    return 0;
}
