#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main()

{
FILE		*fpt;
unsigned char	*image;
unsigned char	*smoothed;
char		header[320];
int		ROWS,COLS,BYTES;
int		r,c,r2,c2,sum;
struct timespec	tp1,tp2;

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

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp1);
printf("Start Time: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);

	/* smooth image, not including the border points */
for (r=3; r<ROWS-3; r++) {
  for (c=3; c<COLS-3; c++)
    {
    sum=0;
    for (r2=-3; r2<=3; r2++) {
      for (c2=-3; c2<=3; c2++) {
        sum+=image[(r+r2)*COLS+(c+c2)];
        }
    }
    smoothed[r*COLS+c]=sum/49;
    }
}

	/* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
printf("End Time: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);

	/* report how long it took to smooth */
printf("Time Taken: %ld\n",tp2.tv_nsec-tp1.tv_nsec);

	/* write out smoothed image to see result */
fpt=fopen("smoothed_basic.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed,COLS*ROWS,1,fpt);
fclose(fpt);
return 0;
}

