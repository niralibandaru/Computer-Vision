#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main()

{
FILE		*fpt;
unsigned char	*image;
unsigned char	*smoothed, *smoothednew;
char		header[320];
int		ROWS,COLS,BYTES;
int		r,c,r2,c2,sum, prevsum=0;
struct timespec	tp1,tp2;
int *myarray;

	/* read image */
if ((fpt=fopen("bridge.ppm","rb")) == NULL)
  {
  printf("Unable to open bridge.ppm for reading\n");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d",header,&COLS,&ROWS,&BYTES);

if (strcmp(header,"P5") != 0  ||  BYTES != 255)
  {
  printf("Not a greyscale 8-bit PPM image\n");
  exit(0);
  }
image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(image,1,COLS*ROWS,fpt);
fclose(fpt);

	/* allocate memory for smoothed version of image */
smoothed=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
smoothednew=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
myarray=(int *)calloc(ROWS*COLS,sizeof(int));


	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp1);
printf("Start Time: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

/* smooth image, not including the border points */
for (r=0; r<ROWS; r++) {
  for (c=3; c<COLS-3; c++)
    {
    sum=0;
    if(c==3) {
        for (c2=-3; c2<=3; c2++) {
            sum += image[(r)*COLS+(c+c2)];
        }
        prevsum = sum;
        myarray[r*COLS+c] = sum;
    }
    else {
        sum = prevsum - image[(r)*COLS+(c-4)] + image[(r)*COLS+(c+3)];
        prevsum = sum;
        myarray[r*COLS+c] = sum;

    }
    }
}
for (c=0; c<COLS; c++) {
    for (r=3; r<ROWS-3; r++) {
    sum=0;
    if(r==3) {
        for (r2=-3; r2<=3; r2++) {
            sum += myarray[(r+r2)*COLS+(c)];
        }
        prevsum = sum;

    }
    else{
        sum = prevsum - myarray[(r-4)*COLS+(c)] + myarray[(r+3)*COLS+(c)];
        prevsum = sum;
    }
    smoothednew[r*COLS+c]= sum/49;
    }
}

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
printf("End Time: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
printf("Time Taken: %ld\n",tp2.tv_nsec-tp1.tv_nsec);

	/* write out smoothed image to see result */
fpt=fopen("smoothednew_sepwindow.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothednew,COLS*ROWS,1,fpt);
fclose(fpt);
return 0;
}


