#include <stdio.h>
#include <stdlib.h>

FILE *fptr;

int main()
{

	// This pointer will hold the
	// base address of the block created
	int* ptr;
	int* sal;
    fptr=fopen("Log_archive.txt","w");

	//Definition of variables
	int n, i, j, m, k, mask, count, col, lin, probe, eval, data, data1, new_element, elements_per_line;


	// Get the number of elements for the array
	n = 1000;
	//Define the number of elements in the matrix
	elements_per_line = 20;
	//The number of elements in a line will allways take the entire
	//values "n". This is n % elements_per_line = 0
	//For other else value, the algotim will have an error
	if (n % elements_per_line != 0){
		printf("Define new value for --elements_per_line-- variable, the program will finish");
		return 1;
	}
	//Ensure the sucess of algoritm
	printf("Enter number of elements: %d\n", n);
	printf("Select size of kernel-mask- 3, 5, 7 or 9\n");
	scanf("%d", &mask);

	// Dynamically allocate memory using malloc()
	ptr = (int*)malloc(n * sizeof(int));
	sal = (int*)malloc(n * sizeof(int));

	// Check if the memory has been successfully
	// allocated by malloc or not
	if (ptr == NULL) {
		printf("Memory not allocated.\n");
		exit(0);
	}
	else {

		// Memory has been successfully allocated
		printf("Memory successfully allocated using malloc.\n");
        fprintf(fptr, "The number of elements in the matrix are %d.\n", n);
        fprintf(fptr, "Number of columns is  %d.\n", elements_per_line);
        fprintf(fptr, "Mask dimension MxN is  %d.\n", mask);

		// Get the elements of the array
		for (i = 0; i < n; ++i) {
			//ptr[i] = i + 11;
			ptr[i] = 1;
		}

		// Print the elements of the array
		k = 0;
		for (i = 0; i < n; ++i) {
			++k;
			if (k != elements_per_line){
                fprintf(fptr, "%d , ", ptr[i]);
			}
			if (k == elements_per_line){
                fprintf(fptr, "%d\n", ptr[i]);
				k = 0;
			}
		}
        fprintf(fptr, "Dark elemets to incorporate to matrix in 0:\n");
		k = 0;
		mask = mask - 1;
		mask = mask / 2;
		for (j = 0; j < mask; j++){
			for (i = 0; i < elements_per_line + 2 * mask; ++i){
				if (i == elements_per_line + 2 * mask -1){
                    fprintf(fptr, "0\n");
				}
				else{
                    fprintf(fptr, "0 , ");
				}
			}
		}
		for (i = 0; i < n; ++i){
			++k;
			if (k == 1){
				for (j = 0; j < mask; j++){
                    fprintf(fptr, "0 , ");
				}
			}
			if (k != elements_per_line){
                fprintf(fptr, "%d , ", ptr[i]);
			}
			if (k == elements_per_line){
                fprintf(fptr, "%d , ", ptr[i]);
				k = 0;
				for (j = 0; j < mask - 1; j++){
                    fprintf(fptr, "0 , ");
				}
                fprintf(fptr, "0\n");
			}
		}
		for (j = 0; j < mask; j++){
			for (i = 0; i < elements_per_line + 2 * mask; ++i){
				if (i == elements_per_line + 2 * mask - 1){
                    fprintf(fptr, "0\n");
				}
				else{
                    fprintf(fptr, "0 , ");
				}
			}
		}

	}

	// Store new elements of the converted matrix
	//First element of the memory, special kernel conditions;
	for(count = 0; count < n; count++){
		new_element = 0;
		eval = mask;
		probe = 0;
		if (count % elements_per_line == 0){
			probe = 1;
		}
		if (count % elements_per_line == (elements_per_line - 1)){
			probe = 4;
		}
		if (count % elements_per_line !=0 && 
		(count - (count/elements_per_line)*elements_per_line - mask < 0)){
			eval = -(count - (count/elements_per_line)*elements_per_line);
			data = eval;
			probe = 2;
		}
		if (count % elements_per_line != (elements_per_line - 1) &&
		(count - (count/elements_per_line)*elements_per_line + mask > (elements_per_line - 1))){
			eval = ((1 + count/elements_per_line)*elements_per_line) - count;
			data = eval;
			probe = 3;
		}
		for (i = 0; i < (2 * mask) + 1; i++){
			lin = ( (count/elements_per_line) + i - mask);
			if (lin > n / elements_per_line){
				lin = -1;
			}
			for (j = 0; j < (2 * mask) + 1 ; j++){
				col = (count + j - mask);
			 	if (probe == 1 && col < count){
			 		col = - eval;
			 		eval --;
					if (eval == 0){
						eval = mask;
					}
			 	}
			 	if (probe == 2 && col < count + data){
			 		col = eval;
			 		eval ++;
					if (eval == 0){
						eval = data;
					}
			 	}
			 	if (probe == 3 && col >= count + data){
			 		col = - eval;
			 		eval ++;
					if (eval == 0){
						eval = data;
					}
			 	}
			 	if (probe == 4 && col > count){
			 		col = - eval;
			 		eval --;
					if (eval == 0){
						eval = mask;
					}
			 	}
				if(col < 0 || lin < 0){
					data1 = 0;
				}
				else{
					data1 = (col % elements_per_line) + (lin * elements_per_line);
					
					data1 = ptr[data1];
					new_element += data1;
				}
			}
		}
		sal[count] = new_element;
	}

	// Print the elements new array of the array
    fprintf(fptr, "The elements of the new array are:\n");
	k = 0;
	for (i = 0; i < n; ++i) {
		++k;
		if (k != elements_per_line){
            fprintf(fptr, "%d, ", sal[i]);
		}
		if (k == elements_per_line){
            fprintf(fptr, "%d\n", sal[i]);
			k = 0;
		}
	}

    printf("Output file completed--Log_archive.txt\n");

	free(ptr);
	free(sal);
    fclose(fptr);

	return 0;
}
