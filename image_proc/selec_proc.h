//Emmanuel Torres
//Tecnologico de Monterrey
//Campus Puebla
//Octubre 2024
extern void itoa(int N, char *str) {
    int i = 0;
  
    // Save the copy of the number for sign
    int sign = N;

    // If the number is negative, make it positive
    if (N < 0)
        N = -N;

    // Extract digits from the number and add them to the
    // string
    while (N > 0) {
      
        // Convert integer digit to character and store
      	// it in the str
        str[i++] = N % 10 + '0';
      	N /= 10;
    } 

    // If the number was negative, add a minus sign to the
    // string
    if (sign < 0) {
        str[i++] = '-';
    }

    // Null-terminate the string
    str[i] = '\0';

    // Reverse the string to get the correct order
    for (int j = 0, k = i - 1; j < k; j++, k--) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
    }
}

extern void inv_img(char mask[10], char path[80]){
    FILE *image, *outputImage, *lecturas, *fptr;    
    // char PATH_0[120] = "/Users/emmanueltorres/Documents/image_proces_c/c_openmp/img/";
    char add_char[80] = "./img/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");
    printf("%s\n", add_char);
    image = fopen(path,"rb");          //Original Image
    outputImage = fopen(add_char,"wb");

	//Definition of variables
	int i, j, tam1;
    long ancho, tam, bpp;
    long alto;
    unsigned char r, g, b, pixel;               //Pixel

    unsigned char xx[54];
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    printf("tamano archivo %li\n", tam);
    tam1 = tam;
    printf("bits por pixel %li\n", bpp);
    printf("largo img %li\n",alto);
    printf("ancho img %li\n",ancho);

    unsigned char* arr_in = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
   
    j = 0;

    while(!feof(image)){
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);
        pixel = 0.21 * r + 0.72 * g + 0.07 * b;
        arr_in[j] = pixel;
        j++;
    }

    printf("lectura de datos: %d\n", j * 3);
    printf("elementos faltantes: %d\n", tam1 - (j * 3));
    
    for(int i = 0; i < ancho*alto; i++){
        
        // para imagen sample agregar 140
        //fputc(arr_in [(ancho * alto) - 140 - i], outputImage);
        fputc(arr_in [(ancho * alto) - i], outputImage);
        fputc(arr_in [(ancho * alto) - i], outputImage);
        fputc(arr_in [(ancho * alto) - i], outputImage);
        
    }
    free(arr_in);
    fclose(image);
    fclose(outputImage);

}

extern void inv_img_color(char mask[10], char path[80]){
    FILE *image, *outputImage, *lecturas, *fptr;    
    // char PATH_0[120] = "/Users/emmanueltorres/Documents/image_proces_c/c_openmp/img/";
    char add_char[80] = "./img/";
    strcat(add_char, mask);
    strcat(add_char, ".bmp");
    printf("%s\n", add_char);
    image = fopen(path,"rb");          //Original Image
    outputImage = fopen(add_char,"wb");

	//Definition of variables
	int i, j, tam1;
    long ancho, tam, bpp;
    long alto;
    unsigned char r, g, b, pixel;               //Pixel

    unsigned char xx[54];
    for(int i=0; i<54; i++) {
      xx[i] = fgetc(image);
      fputc(xx[i], outputImage);   //Copia cabecera a nueva imagen
    }
    tam = (long)xx[4]*65536+(long)xx[3]*256+(long)xx[2];
    bpp = (long)xx[29]*256+(long)xx[28];
    ancho = (long)xx[20]*65536+(long)xx[19]*256+(long)xx[18];
    alto = (long)xx[24]*65536+(long)xx[23]*256+(long)xx[22];
    printf("tamano archivo %li\n", tam);
    tam1 = tam;
    printf("bits por pixel %li\n", bpp);
    printf("largo img %li\n",alto);
    printf("ancho img %li\n",ancho);

    unsigned char* arr_in_b = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_g = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
    unsigned char* arr_in_r = (unsigned char*)malloc(ancho*alto*sizeof(unsigned char));
   
    j = 0;

    while(!feof(image)){
        b = fgetc(image);
        g = fgetc(image);
        r = fgetc(image);
        
        arr_in_b[j] = b;
        arr_in_g[j] = g;
        arr_in_r[j] = r;
        j++;
    }

    printf("lectura de datos: %d\n", j * 3);
    printf("elementos faltantes: %d\n", tam1 - (j * 3));
    
    for(int i = 0; i < ancho*alto; i++){
        
        fputc(arr_in_b [(ancho * alto) - i], outputImage);
        fputc(arr_in_g [(ancho * alto) - i], outputImage);
        fputc(arr_in_r [(ancho * alto) - i], outputImage);
        
    }
    free(arr_in_b);
    free(arr_in_g);
    free(arr_in_r);
    fclose(image);
    fclose(outputImage);

}