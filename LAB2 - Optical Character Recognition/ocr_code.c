/* LAB 2 -- OPTICAL CHARACTER RECOGNITION
    NIRALI BANDARU
    INTRO TO COMPUTER VISION
    PROF. ADAM HOOVER
    FALL 2020
    CLEMSON UNIVERSITY
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int main(int argc, char *argv[]) {

//Declare variables-----------------------------------------------------------------------------------------

FILE    *fpt, *fpt1, *fpt2;
unsigned char   *temp_image, *input_image, *msf_image;
char    header[80], header1[80], letter, *zero_mean_template;
int     ROWS, COLS, BYTES, r, c, BYTES1;
int     dr, dc, tr, tc, i, j;
int     average, sum, min_v, max_v;
double  result_num;
long int    *myarray, sum_msf, min_value, max_value;
int     TP, FP, T, TN, FN, letter_c, letter_r, det, iterations, num_det;
char    gt_letter[10];

//Read input image and template image, and ground truth file-------------------------------------------------

printf("Looking for %c\n", letter);

if (argc != 2) {
    printf("Usage: OCR [letter]\n");
    exit(0);
}

letter = argv[1][0];

fpt = fopen("parenthood.ppm", "rb");
fpt1 = fopen("parenthood_e_template.ppm", "rb");
fpt2 = fopen("groundtruth.txt", "r");

if(fpt == NULL){
    printf("Unable to open %s for reading\n", "parenthood.ppm");
    exit(0);
}
if(fpt1 == NULL){
    printf("Unable to open %s for reading\n", "parenthood_e_template.ppm");
    exit(0);
}
if(fpt2 == NULL){
    printf("Unable to open %s for reading\n", "groundtruth.txt");
    exit(0);
}

printf("Reading input files\n");
i = fscanf(fpt, "%s %d %d %d ", header, &COLS, &ROWS, &BYTES);
if (i != 4 || strcmp(header, "P5") != 0 || BYTES != 255) {
    printf("%s is not an 8-bit PPM greyscale (P5) image\n", "parenthood.ppm");
    fclose(fpt);
    exit(0);
}

j = fscanf(fpt1, "%s %d %d %d ", header1, &c, &r, &BYTES1);
if (j != 4 || strcmp(header, "P5") != 0 || BYTES != 255) {
    printf("%s is not an 8-bit PPM greyscale (P5) image\n", "parenthood_e_template.ppm");
    fclose(fpt1);
    exit(0);
}

// Allocate memory for images and output image-------------------------------------------------------------------

printf("Allocating memory\n");
input_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
if (input_image == NULL) {
    printf("Unable to allocate %d x %d memory\n", COLS, ROWS);
    exit(0);
}
header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(input_image,1,COLS*ROWS,fpt);
fclose(fpt);

temp_image = (unsigned char *)calloc(r*c, sizeof(unsigned char));
if (temp_image == NULL) {
    printf("Unable to allocate %d x %d memory\n", c, r);
    exit(0);
}
fread(temp_image,1,c*r,fpt1);
fclose(fpt1);

zero_mean_template = (char *)calloc(r*c, sizeof(char));

msf_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));

myarray = (long int *)calloc(ROWS*COLS, sizeof(long int));

//Calculating the Zero-Mean Template---------------------------------------------------------------------------
printf("Calculating the zero mean template\n");
sum = 0;
for (tr = 0; tr < r; tr++) {
    for (tc = 0; tc < c; tc++) {
        sum += temp_image[tr*c + tc];
    }
}
average = (int)((double) sum/(double)(r*c));

for (tr = 0; tr < r; tr++) {
    for (tc=0; tc < c; tc++) {
        zero_mean_template[tr*c+tc] = temp_image[tr*c+tc] - average;
    }
}
//Performing Convolution--------------------------------------------------------------------------------------
printf("Calculating MSF Image\n");
min_value = 0x7fffffff;
max_value = -1;
for (dr = 7; dr < ROWS-7; dr++) {
    for (dc = 4; dc < COLS-4; dc++) {
        sum_msf = 0;
        for (tr = -7; tr <= 7; tr++) {
            for (tc = -4; tc <= 4; tc++) {
               sum_msf += (input_image[(dr+tr)*COLS+(dc+tc)]*zero_mean_template[(tr+7)*c+(tc+4)]);
            }
        }
        myarray[dr*COLS+dc] = sum_msf;
        if (sum_msf < min_value) {
            min_value = sum_msf;
        }
        if (sum_msf > max_value) {
            max_value = sum_msf;
        }
    }
}

//Normalize the MSF------------------------------------------------------------------------------------------

printf("Normalizing MSF image\n");
// Normalization formula ---> (I-Imin)*(newmax-newmin/oldmax-oldmin) - newmin
result_num = (double) 255/(max_value-min_value);

min_v = 10000000;
max_v = 0;
for (dr = 7; dr < ROWS-7; dr++) {
    for (dc = 4; dc < COLS-4; dc++) {
        myarray[dr*COLS+dc] = (int)((myarray[dr*COLS+dc] - min_value)*(double)(result_num));
        msf_image[dr*COLS+dc] = myarray[dr*COLS+dc];

        //Determine Normalized MSF minimum and maximum values to reconfirm scale 0-255
        if (msf_image[dr*COLS+dc] < min_v) {
            min_v = msf_image[dr*COLS+dc];
        }
        if (msf_image[dr*COLS+dc] > max_v) {
            max_v = msf_image[dr*COLS+dc];
        }
    }
}

fpt=fopen("msf_out.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(msf_image,COLS*ROWS,1,fpt);
fclose(fpt);

//Determining TP and FP at Threshold T-----------------------------------------------------------------------

T = 210;
printf("Analyzing at Threshold --> T = %d\n", T);
for (dr = 7; dr < ROWS-7; dr++) {
    for (dc = 4; dc < COLS-4; dc++) {
        if (msf_image[dr*COLS+dc] > T) {
            msf_image[dr*COLS+dc] = 255;
        }
        else {
            msf_image[dr*COLS+dc] = 0;
        }
    }
}

//Writing the binary image output
printf("Converting MSF image into binary\n");
fpt=fopen("binary_out.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(msf_image,COLS*ROWS,1,fpt);
fclose(fpt);

//Looping through the ground truth text file

num_det = TP = FP = 0;
fpt=fopen("groundtruth.txt","r");
while (1)
  {
  iterations += 1;
  i=fscanf(fpt,"%s %d %d",gt_letter,&letter_c,&letter_r);
  if (i != 3) {
    break;
  }
det = 0;
  for (dr=letter_r-7; dr<=letter_r+7; dr++) {
    for (dc=letter_c-4; dc<=letter_c+4; dc++) {
        if (msf_image[dr*COLS+dc] > T) {
            det = 1;
            num_det += 1;
            if (gt_letter[0] == letter) {
                TP += 1;
            }
            else{
                FP += 1;
            }
            break;
        }
        if (det == 1) {
            break;
        }
        else{
            continue;
        }
    }
    if (det == 1) {
        break;
    }
    else {
        continue;
    }
  }
}

//Calculating FP, TP, TPR and FPR at T

FN = 151-TP;
TN = 1111-FP;
printf("Retrieved Outputs from Analysis: \n\n");
printf("Detected = %d\n", num_det);
printf("TP = %d       ", TP);
printf("FP = %d\n", FP);
printf("FN = %d         ", FN);
printf("TN = %d\n", TN);


printf("End of analysis.");

return 0;
}

