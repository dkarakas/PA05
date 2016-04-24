#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"

/* check whether a header is valid
 * assume that header has been read from fptr
 * the position of the indicator of fptr is not certain
 * could be at the beginning of the file, end of the file or 
 * anywhere in the file
 * note that the check is only for this exercise/assignment
 * in general, the format is more complicated
 */
void print_char_in_bits(char number)
{
   int i;
   int result;
   unsigned char mask = 1 << (8 - 1);
   for (i = 0; i < 8; i++) {
    mask = 1;
    mask <<= (8 - i - 1);
    result = mask & number;
    if (result == 0) {
     fprintf(stderr,"0");
    } else {
     fprintf(stderr,"1");
    }
  mask >>= 1;
  }
}

int Is_BMP_Header_Valid(BMP_Header* header, FILE *fptr) {
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
     return FALSE;
  }
  // skip the two unused reserved fields

  // check the offset from beginning of file to image data
  // essentially the size of the BMP header
  // BMP_HEADER_SIZE for this exercise/assignment
  if (header->offset != BMP_HEADER_SIZE) {
     return FALSE;
  }
      
  // check the DIB header size == DIB_HEADER_SIZE
  // For this exercise/assignment
  if (header->DIB_header_size != DIB_HEADER_SIZE) {
     return FALSE;
  }

  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
    return FALSE;
  }

  // skip the test for xresolution, yresolution

  // ncolours and importantcolours should be 0
  if (header->ncolours != 0) {
    return FALSE;
  }
  if (header->importantcolours != 0) {
    return FALSE;
  }
  
  // Make sure we are getting 24 bits per pixel
  // or 16 bits per pixel
  // only for this assignment
  if (header->bits != 24 && header->bits != 16) {
    return FALSE;
  }

  // fill in extra to check for file size, image size
  // based on bits, width, and height

  //set at the beggining
  fseek(fptr,0,SEEK_END);
  unsigned long int sizeFile = ftell(fptr);

  if(sizeFile != header->size){
    fprintf(stderr,"Not good header size\n");
    return FALSE;
  }
  if((sizeFile-54) != header->imagesize){
    fprintf(stderr,"Not good header image size\n");
    return FALSE;
  }
  

  //find need padding
  int bytePerPixel = header->bits/8;
  int padNeed = (header->width*bytePerPixel)%4;
  
  if(padNeed != 0){
    padNeed = 4 - padNeed;
  }

  if(header->imagesize != (header->height*((header->width*bytePerPixel)+padNeed))){
    fprintf(stderr,"Not good header image size//\n");
    return FALSE;
  }

  return TRUE;
}

/* The input argument is the source file pointer. 
 * The function returns an address to a dynamically allocated BMP_Image only 
 * if the file * contains a valid image file 
 * Otherwise, return NULL
 * If the function cannot get the necessary memory to store the image, also 
 * return NULL
 * Any error messages should be printed to stderr
 */
BMP_Image *Read_BMP_Image(FILE* fptr) {

  // go to the beginning of the file
  fseek(fptr,0,SEEK_SET);

  BMP_Image *bmp_image = NULL;

  //Allocate memory for BMP_Image*;
  bmp_image = (BMP_Image*)malloc(sizeof(BMP_Image));
  bmp_image->data = NULL;
  if(bmp_image==NULL){
    fprintf(stderr,"error malloc");
    return NULL;
  }

  //Read the first 54 bytes of the source into the header
  int read = fread((&(bmp_image->header)),sizeof(BMP_Header),1,fptr);
  // if read successful, check validity of header
  if(read == 1){
    if(!Is_BMP_Header_Valid(&bmp_image->header,fptr)){
      fprintf(stderr,"Can't read the image from file1\n");
      Free_BMP_Image(bmp_image);
      return NULL;
    }
  }else{
    return NULL;
  }
  // Allocate memory for image data
  bmp_image->data = (unsigned char *)malloc(bmp_image->header.imagesize);
  if(bmp_image->data==NULL){
    fprintf(stderr,"error malloc bmp_image->data");
    return NULL;
  }

  // read in the image data
  fseek(fptr,sizeof(BMP_Header),SEEK_SET);
  read = fread(bmp_image->data,bmp_image->header.imagesize,1,fptr);
  if(read != 1){
     fprintf(stderr,"Can't read the image from file2\n");
     Free_BMP_Image(bmp_image);
     return NULL;
  }

   //find need padding
   int bytePerPixel = bmp_image->header.bits/8;
   int padNeed = (bmp_image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 

  char (*arrayOriginalPicture)[bmp_image->header.width*bytePerPixel+padNeed]= (char (*)[bmp_image->header.width*bytePerPixel+padNeed])(bmp_image->data); 
  int i,j;

  for(i=0; i<bmp_image->header.height;i++)
    for(j=0; j<(bmp_image->header.width*bytePerPixel+padNeed);j++)
        if(   bmp_image->header.width*bytePerPixel <= j  ){
          arrayOriginalPicture[i][j] = 0;
        }
  return bmp_image;
}

/* The input arguments are the destination file pointer, BMP_Image *image.
 * The function write the header and image data into the destination file.
 * return TRUE if write is successful
 * FALSE otherwise
 */
int Write_BMP_Image(FILE* fptr, BMP_Image* image) 
{
   // go to the beginning of the file
   fseek(fptr,0,SEEK_SET);

   // write header
   int write = fwrite(&(image->header),sizeof(BMP_Header),1,fptr);
   if(write != 1){
     fprintf(stderr,"error printing file"); 
     return FALSE;
   }


   // write image data
   write = fwrite(image->data,image->header.imagesize,1,fptr);
   if(write != 1){
     fprintf(stderr,"error printing file"); 
     return FALSE;
   }

   return TRUE;
}

/* The input argument is the BMP_Image pointer. The function frees memory of 
 * the BMP_Image.
 */
void Free_BMP_Image(BMP_Image* image) {
  if(image!=NULL)
    if(image->data!=NULL)
      free(image->data);
  if(image!=NULL)
    free(image);
  return;
}

// Given a BMP_Image, create a new image that is a reflection 
// of the given image
// It could be a horizontal reflection (with the vertical mirror 
// being placed at the center of the image) 
// It could be a vertical reflection (with the horizontal mirror
// being placed at the center of the image)
// It could be a horizontal reflection followed by a vertical
// reflection (or equivalently, a vertical reflection followed by
// horizontal reflection).
// hrefl == 1 implies that a horizontal reflection should take place
// hrefl == 0 implies that a horizontal reflection should not take place
// vrefl == 1 implies that a vertical reflection should take place
// vrefl == 0 implies that a vertical reflection should not take place

BMP_Image *Reflect_BMP_Image(BMP_Image *image, int hrefl, int vrefl)
{
   //CREATING A NEW STRUCTURE FOR THE IMAGE AND COPYING THE HEADER.
   BMP_Image *t_image = NULL;
   if(image != NULL){
     //ALLOC SPACE FOR THE NEW STRUCTURE
     t_image = (BMP_Image *)malloc(sizeof(BMP_Image));
     if(t_image == NULL){
       return NULL;
     }
     t_image->data = NULL;

     t_image->data = (unsigned char *)malloc(image->header.imagesize);
     if(t_image->data==NULL){
       free(t_image);
       fprintf(stderr,"error malloc bmp_image->data");
       return NULL;
     }

     //COPYING THE HEADER INTO THE NEW STRUCTURE
     t_image->header = image->header;
   } 

   //find need padding
   int bytePerPixel = image->header.bits/8;
   int padNeed = (image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 

   char (*arrayPic)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(t_image->data);
   char (*arrayOriginalPicture)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(image->data); 

  int i,j;
  for(i=0; i<t_image->header.height;i++)
    for(j=0; j<(t_image->header.width*bytePerPixel+padNeed);j++)
        if(   t_image->header.width*bytePerPixel <= j  ){
          arrayPic[i][j] = 0;
        }


  int count;
  int move;
   if(hrefl == 1){
     for(i=0; i<image->header.height;i++){
      count = 1;
      move = 1;
       for(j=0; j < (image->header.width*bytePerPixel+padNeed);j++)
           if(   image->header.width*bytePerPixel > j  ){
                arrayPic[i][bytePerPixel*count-move] =  arrayOriginalPicture[i][image->header.width*bytePerPixel-1-j];
                move++;
                if(move == (bytePerPixel+1)){
                  count++;
                  move = 1;
                }
           }else{break;}
      }
     Free_BMP_Image(image);
     return t_image;
   }

   if(vrefl == 1){
     for(i=0; i<image->header.height;i++)
       for(j=0; j < image->header.width*bytePerPixel;j++)
                arrayPic[i][j] =  arrayOriginalPicture[image->header.height-i-1][j];
   Free_BMP_Image(image);
   return t_image;

   }
   return image;
}

BMP_Image *Convert_24_to_16_BMP_Image(BMP_Image *image){

   //find need padding
   int bytePerPixel = image->header.bits/8;
   int padNeed = (image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 
  
   //Calculate the number of bit in 16 bit picture
   int bytesPerRow = image->header.width*2;
   int padNeed_16bit = bytesPerRow % 4;
   if(padNeed_16bit != 0){
     padNeed_16bit = 4 - padNeed_16bit;
   }

   fprintf(stderr,"padd bits %d\n",padNeed_16bit);
   // creating a new structure to store 16 bit
   BMP_Image *t_image = NULL;
   if(image != NULL){
     //ALLOC SPACE FOR THE NEW STRUCTURE
     t_image = (BMP_Image *)malloc(sizeof(BMP_Image));
     if(t_image == NULL){
       return NULL;
     }
     t_image->data = NULL;

     t_image->data = (unsigned char *)malloc( image->header.height*(bytesPerRow+padNeed_16bit));
     if(t_image->data==NULL){
       free(t_image);
       fprintf(stderr,"error malloc bmp_image->data");
       return NULL;
     }

     //COPYING THE HEADER INTO THE NEW STRUCTURE
     t_image->header = image->header;
     t_image->header.bits = 2*8;
     t_image->header.imagesize = image->header.height*(bytesPerRow+padNeed_16bit);
     t_image->header.size = t_image->header.imagesize + BMP_HEADER_SIZE;
   } 

  //casting the original data arrays into two dimesional row(bytes) and height
  char (*arrayPic16bit)[bytesPerRow+padNeed_16bit] = (char (*)[bytesPerRow+padNeed_16bit])(t_image->data);
  char (*originalPicture)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(image->data);
  //variables storing the 24 bit pixels
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned int valueFor16Bit=0;
  int count = 0; //value which will track the positions in the 16 bit image;
  //used to cast value of the unsgined int valueFor16 bit
  char *insertArray;

  int i,j;
  for(i=0; i<t_image->header.height;i++)
    for(j=t_image->header.width*2-1; j<(t_image->header.width*2+padNeed_16bit);j++)
          arrayPic16bit[i][j] = 0;

  for(i=0; i < t_image->header.height; i++){
    count=0;
    for(j=0; j < image->header.width*bytePerPixel;j = j + 3){
  //     fprintf(stderr,"\n row %d j %d\n",image->header.width*bytePerPixel+padNeed,j);
       red = originalPicture[i][j+2];
       green = originalPicture[i][j+1];
       blue = originalPicture[i][j];
       //adjusting the colors for 16 bit;
       red = red >> 3;
       green = green >> 3;
       blue = blue >> 3;
       //store the in a new variable
       valueFor16Bit = (red << RED_BIT) | (green << GREEN_BIT) | (blue << 0);
/*
       print_char_in_bits(red);
       fprintf(stderr," ");
       print_char_in_bits(green);
       fprintf(stderr," ");
       print_char_in_bits(blue);
       fprintf(stderr," ");
*/
       insertArray = (char *)&valueFor16Bit;      
       arrayPic16bit[i][count] = insertArray[0];
       arrayPic16bit[i][count+1] = insertArray[1];
       count = count + 2;
       valueFor16Bit = 0;
    }
  }
  Free_BMP_Image(image);
  return t_image;
}

BMP_Image *Convert_16_to_24_BMP_Image(BMP_Image *image){

   //find need padding
   int bytePerPixel = image->header.bits/8;
   int padNeed = (image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 
  
   //Calculate the number of bit in 16 bit picture
   int bytesPerRow = image->header.width*3;
   int padNeed_24bit = bytesPerRow % 4;
   if(padNeed_24bit != 0){
     padNeed_24bit = 4 - padNeed_24bit;
   }

   fprintf(stderr,"padd bits %d\n",padNeed_24bit);
   // creating a new structure to store 24 bit
   BMP_Image *t_image = NULL;
   if(image != NULL){
     //ALLOC SPACE FOR THE NEW STRUCTURE
     t_image = (BMP_Image *)malloc(sizeof(BMP_Image));
     if(t_image == NULL){
       return NULL;
     }
     t_image->data = NULL;

     t_image->data = (unsigned char *)malloc( image->header.height*(bytesPerRow+padNeed_24bit));
     if(t_image->data==NULL){
       free(t_image);
       fprintf(stderr,"error malloc bmp_image->data");
       return NULL;
     }

     //COPYING THE HEADER INTO THE NEW STRUCTURE
     t_image->header = image->header;
     t_image->header.bits = 3*8;
     t_image->header.imagesize = image->header.height*(bytesPerRow+padNeed_24bit);
     t_image->header.size = t_image->header.imagesize + BMP_HEADER_SIZE;
   } 

  //casting the original data arrays into two dimesional row(bytes) and height
  char (*arrayPic24bit)[bytesPerRow+padNeed_24bit] = (char (*)[bytesPerRow+padNeed_24bit])(t_image->data);
  char (*originalPicture)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(image->data);
  //variables storing the 24 bit pixels
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned int valueFor24Bit=0;
  int count = 0; //value which will track the positions in the 24 bit image;
  //used to cast value of the unsgined int valueFor24 bit
  char *insertArray;

  int i,j;
  for(i=0; i<t_image->header.height;i++)
    for(j=t_image->header.width*3-1; j<(t_image->header.width*3+padNeed_24bit);j++)
          arrayPic24bit[i][j] = 0;

  for(i=0; i < t_image->header.height; i++){
    count=0;
    for(j=0; j < image->header.width*bytePerPixel;j = j + 2){
   //     fprintf(stderr,"\n row %d j %d\n",image->header.width*bytePerPixel+padNeed,j);
       //taking the middle 5 bits
       red = originalPicture[i][j+1]>>2;
       //taking the last 2 bits of the first byte and then taking the first 3 bits of the next byte
       green = ((originalPicture[i][j+1] & 0x03) << 3) | (((unsigned char)originalPicture[i][j])>>5);
       //taking the last 5 bits of the last byte
       blue = originalPicture[i][j]&0x1F;
       //adjusting the colors for 24 bit;
       red = ((red)*255)/31;
       green = ((green)*255)/31;
       blue = ((blue)*255)/31;
       //store the in a new variable
       valueFor24Bit = (red << 16) | (green << 8) | (blue << 0);
       insertArray = (char *)&valueFor24Bit;      
       arrayPic24bit[i][count] = insertArray[0];
       arrayPic24bit[i][count+1] = insertArray[1];
       arrayPic24bit[i][count+2] = insertArray[2];
       count = count + 3;
       valueFor24Bit = 0;
    }
  }
  Free_BMP_Image(image);
  return t_image;
}

BMP_Image *Convert_24_to_16_BMP_Image_with_Dithering(BMP_Image *image){
   //find need padding
   int bytePerPixel = image->header.bits/8;
   int padNeed = (image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 
  
   //Calculate the number of bit in 16 bit picture
   int bytesPerRow = image->header.width*2;
   int padNeed_16bit = bytesPerRow % 4;
   if(padNeed_16bit != 0){
     padNeed_16bit = 4 - padNeed_16bit;
   }

   fprintf(stderr,"padd bits %d\n",padNeed_16bit);
   // creating a new structure to store 16 bit
   BMP_Image *t_image = NULL;
   if(image != NULL){
     //ALLOC SPACE FOR THE NEW STRUCTURE
     t_image = (BMP_Image *)malloc(sizeof(BMP_Image));
     if(t_image == NULL){
       return NULL;
     }
     t_image->data = NULL;

     t_image->data = (unsigned char *)malloc( image->header.height*(bytesPerRow+padNeed_16bit));
     if(t_image->data==NULL){
       free(t_image);
       fprintf(stderr,"error malloc bmp_image->data");
       return NULL;
     }

     //COPYING THE HEADER INTO THE NEW STRUCTURE
     t_image->header = image->header;
     t_image->header.bits = 2*8;
     t_image->header.imagesize = image->header.height*(bytesPerRow+padNeed_16bit);
     t_image->header.size = t_image->header.imagesize + BMP_HEADER_SIZE;
   } 

  //casting the original data arrays into two dimesional row(bytes) and height
  char (*arrayPic16bit)[bytesPerRow+padNeed_16bit] = (char (*)[bytesPerRow+padNeed_16bit])(t_image->data);
  char (*originalPicture)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(image->data);
  //variables storing the 24 bit pixels
  unsigned char red;
  unsigned char green;
  unsigned char blue;
  unsigned int valueFor16Bit=0;
  unsigned int valueFor24Bit=0;
  int count = 0; //value which will track the positions in the 16 bit image;
  //used to cast value of the unsgined int valueFor16 bit
  char *insertArray;
  //char *insertArray2;
  int newPixel;
  int oldPixel;
  int error;

  int i,j;
  for(i=0; i<t_image->header.height;i++)
    for(j=t_image->header.width*2-1; j<(t_image->header.width*2+padNeed_16bit);j++)
          arrayPic16bit[i][j] = 0;

   for(i=0; i < t_image->header.height; i++){
    count=0;
    for(j=0; j < image->header.width*bytePerPixel;j = j + 3){
       //oldPixel = (int)(originalPicture[i][j]& 0x00FFFFFF);
       oldPixel = ((*((int *)&originalPicture[i][j]))&0x00FFFFFF);

       red = originalPicture[i][j+2];
       green = originalPicture[i][j+1];
       blue = originalPicture[i][j];
       //adjusting the colors for 16 bit;
       red = red >> 3;
       green = green >> 3;
       blue = blue >> 3;
       //store the in a new variable
       valueFor16Bit = (red << RED_BIT) | (green << GREEN_BIT) | (blue << 0);//new pixel
       insertArray = (char *)&valueFor16Bit;
       arrayPic16bit[i][count] = insertArray[0];
       arrayPic16bit[i][count+1] = insertArray[1];
       valueFor16Bit = 0;

       red = arrayPic16bit[i][count+1]>>2;
       //taking the last 2 bits of the first byte and then taking the first 3 bits of the next byte
       green = ((arrayPic16bit[i][count+1] & 0x03) << 3) | (((unsigned char)arrayPic16bit[i][count])>>5);
       //taking the last 5 bits of the last byte
       blue = arrayPic16bit[i][count]&0x1F;
       //adjusting the colors for 24 bit;
       red = ((red)*255)/31;
       green = ((green)*255)/31;
       blue = ((blue)*255)/31;
       //store the in a new variable
       valueFor24Bit = (red << 16) | (green << 8) | (blue << 0);
       insertArray = (char *)&valueFor24Bit;      
       originalPicture[i][j] = insertArray[0];
       originalPicture[i][j+1] = insertArray[1];
       originalPicture[i][j+2] = insertArray[2];
       count = count + 2;
       valueFor24Bit = 0;
       valueFor16Bit = 0;
       //Here newPixel represents the value that was calculated from down scaling and the up scaling
       //newPixel = (int)(originalPicture[i][j]& 0x00FFFFFF);
       newPixel = ((*((int *)&originalPicture[i][j]))&0x00FFFFFF);
       error = oldPixel - newPixel;
       //fprintf(stderr,"oldPixel %d newPixel %d error %d\n",oldPixel, newPixel, error);
       
       //here newPixel represents the value to be inserted
       //Pixel to the right diffusion
       if((j+3)<image->header.width*bytePerPixel){
         newPixel = ((int)(originalPicture[i][j+3]&0x00FFFFFF)+error*7/16);
         if(newPixel<=255 && newPixel>=0){
           originalPicture[i][j+3] = newPixel;
         }else if(newPixel>255){
           originalPicture[i][j+3] = 255;
         }else if(newPixel<0){
           originalPicture[i][j+3] = 0;
         }
       }

       //Pixel to the left and height +1
       if((i+1)<t_image->header.height){
         if((j-3)>0){
           newPixel = ((((int)originalPicture[i+1][j-3])&(0x00FFFFFF))+error*3/16);
           if(newPixel<=255 && newPixel>=0){
             originalPicture[i+1][j-3] = newPixel;
           }else if(newPixel>255){
             originalPicture[i+1][j-3] = 255;
           }else if(newPixel<0){
             originalPicture[i+1][j-3] = 0;
           }
         }

         //Pixel to the below and height +1
         newPixel = ((((int)originalPicture[i+1][j])&0x00FFFFFF)+error*5/16);
         if(newPixel<=255 && newPixel>=0){
           originalPicture[i+1][j] = newPixel;
         }else if(newPixel>255){
           originalPicture[i+1][j] = 255;
         }else if(newPixel<0){
           originalPicture[i+1][j] = 0;
         }

         //Pixel to the right and height +1
         if((j+3)<image->header.width*bytePerPixel){
           newPixel = ((((int)originalPicture[i+1][j+3])&0x00FFFFFF)+error*1/16);
           if(newPixel<=255 && newPixel>=0){
             originalPicture[i+1][j+3] = newPixel;
           }else if(newPixel>255){
             originalPicture[i+1][j+3] = 255;
           }else if(newPixel<0){
             originalPicture[i+1][j+3] = 0;
           }
         }
       }
    }
  }
  
  Free_BMP_Image(image);
  return t_image;
}
