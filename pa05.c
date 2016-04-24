#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bmp.h"

int main(int argc,char **argv){
  //counter for number of operations;
  int write = 0;//used to assure successful writing
  FILE* fptr_in = NULL;
  FILE* fptr_out = NULL;
  BMP_Image *image = NULL;
  if(argc != 3){
    fprintf(stderr,"Unexpected number of argumets\n");
    return EXIT_FAILURE;
  }

  fptr_in = fopen(argv[1],"r");
  if(fptr_in == NULL){
    fprintf(stderr,"can't open file. free linked list.\n");
    return EXIT_FAILURE;
  }
  fptr_out = fopen(argv[2],"w");
  if(fptr_out == NULL){
    fprintf(stderr,"can't open file. free linked list and input file.\n");
    fclose(fptr_in);
    return EXIT_FAILURE;
  }


  image = Read_BMP_Image(fptr_in);
  if(image == NULL){
    fclose(fptr_in);
    fclose(fptr_out);
    return EXIT_FAILURE;
  }

  if(image->header.bits == 24){
    image = Convert_24_to_16_BMP_Image_with_Dithering(image);
  }else if(image->header.bits == 16){
    image = Convert_16_to_24_BMP_Image(image);
  }else{
    fprintf(stderr,"unknown conversion\n"); 
    Free_BMP_Image(image);
    fclose(fptr_in);
    fclose(fptr_out);
    return EXIT_FAILURE;
  }

  write = Write_BMP_Image(fptr_out,image);
  Free_BMP_Image(image);

  fclose(fptr_in);
  fclose(fptr_out);
  return EXIT_SUCCESS;
} 

