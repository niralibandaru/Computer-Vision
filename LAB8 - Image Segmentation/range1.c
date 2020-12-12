/*range image challenge:

Modify the program abw-coord.c to calculate the x,y,z coordinates for the
given pixel cooordinate.  The conversion is:

x[i,j] = (j-255) * (range[i,j] / scal + offset) / |f|
y[i,j] = (255-i)/c * (range[i,j] / scal + offset) / |f|
z[i,j] = (255 - range[i,j]) / scal

where i is the row coordinate, j is the column coordinate, and
range[i,j] is range[i*COLS+j] in a 1D array storing the image;

and where the calibration constants scal, offset, c and f are given
in the code stub abw-coord.c.

See pg 11 of http://cecas.clemson.edu/~ahoover/ece431/refs/RangeCameras.pdf
for more details.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAXQUEUE 10000

void calculate_cross_product(double x0, double y0, double z0,
                             double x1, double y1, double z1,
                             double x2, double y2, double z2,
                             double *cx, double *cy, double *cz)
{
    double ax, ay, az;
    double bx, by, bz;
    //calculates cross product for surface normals
    ax = x1-x0;
    ay = y1-y0;
    az = z1-z0;

    bx = x2-x0;
    by = y2-y0;
    bz = z2-z0;

    *cx = ay*bz-az*by;
    *cy = az*bx-ax*bz;
    *cz = ax*by-ay*bx;
}

void calculate_angular_distance(double average_x, double average_y, double average_z,
                               double current_x, double current_y, double current_z, double *angulardistance)
{
    //calculate dot product
    double dot_product;
    double distance1, distance2;

    dot_product = (average_x*current_x) + (average_y*current_y) + (average_z*current_z);
    distance1 = sqrt(pow(average_x, 2) + pow(average_y, 2) + pow(average_z,2));
    distance2 = sqrt(pow(current_x, 2) + pow(current_y, 2) + pow(current_z,2));
    *angulardistance = (double) acos(dot_product/(distance1*distance2));
}

int main(int argc, char* argv[])
{
    //variable declaration

    //basics
    FILE		        *fpt;
    unsigned char	    *image;
    char		        header[80];
    int		            ROWS,COLS,MAX;

    //image variables
    int                 T; //threshold to mask out image

    //iterators, loop variables
    int                 r, c, i, j, k, index;

    //coordinate conversions
    double	            cp[7];
    double	            xangle,yangle,dist;
    double	            ScanDirectionFlag,SlantCorrection;
    double		        P[3][128*128];


    //images
    unsigned char       *segmented_image, *thresholded_image;

    //surface normal
    int                 pixel_dist;
    double              C[3][128*128];

    //region growing
    int                 queue[MAXQUEUE],qh,qt;
    int	                r2,c2;
    double              angularthreshold;
    double              sum[3] = {0,0,0};
    int                 count, new_label;
    double              averageX, averageY, averageZ;
    double              angulardistance;
    int                 region, seed;
    //error-handling

    if (argc != 2)
    {
        printf("Usage:  abw-coord [file]\n");
        exit(0);
    }
    fpt = fopen(argv[1],"rb");
    if (fpt == NULL)
    {
        printf("Unable to open %s for reading\n",argv[1]);
        exit(0);
    }
    fscanf(fpt,"%s %d %d %d ",header,&COLS,&ROWS,&MAX);

    //memory allocation
    image = (unsigned char *)calloc(ROWS*COLS,1);
    fread(image,ROWS*COLS,1,fpt);
    fclose(fpt);

    segmented_image = (unsigned char *)calloc(ROWS*COLS,1);
    thresholded_image = (unsigned char *)calloc(ROWS*COLS,1);


    //threshold image to only have chair and floor

    T = 143;
    for (r = 0; r < ROWS; r++)
    {
        for (c = 0; c < COLS; c++)
        {
            if (image[r*COLS+c] > T)
            {
                thresholded_image[r*COLS+c] = 255;
                //printf("Updating %d %d\n", r, c);
            }
            else
            {

                thresholded_image[r*COLS+c] = image[r*COLS+c];
            }
            //segmented_image[r*COLS+c] = thresholded_image[r*COLS+c];
        }
    }

    //calculate 3D coordinates for the image

    cp[0]=1220.7;		            /* horizontal mirror angular velocity in rpm */
    cp[1]=32.0;		                /* scan time per single pixel in microseconds */
    cp[2]=(COLS/2)-0.5;		        /* middle value of columns */
    cp[3]=1220.7/192.0;	            /* vertical mirror angular velocity in rpm */
    cp[4]=6.14;		                /* scan time (with retrace) per line in milliseconds */
    cp[5]=(ROWS/2)-0.5;		        /* middle value of rows */
    cp[6]=10.0;		                /* standoff distance in range units (3.66cm per r.u.) */

    cp[0]=cp[0]*3.1415927/30.0;	    /* convert rpm to rad/sec */
    cp[3]=cp[3]*3.1415927/30.0;	    /* convert rpm to rad/sec */
    cp[0]=2.0*cp[0];		        /* beam ang. vel. is twice mirror ang. vel. */
    cp[3]=2.0*cp[3];		        /* beam ang. vel. is twice mirror ang. vel. */
    cp[1]/=1000000.0;		        /* units are microseconds : 10^-6 */
    cp[4]/=1000.0;			        /* units are milliseconds : 10^-3 */

    ScanDirectionFlag = -1; //scan direction downward
    for (r=0; r<ROWS; r++)
    {
        for (c=0; c<COLS; c++)
        {

            SlantCorrection=cp[3]*cp[1]*((double)c-cp[2]);
            xangle=cp[0]*cp[1]*((double)c-cp[2]);
            yangle=(cp[3]*cp[4]*(cp[5]-(double)r))+	/* Standard Transform Part */
                   SlantCorrection*ScanDirectionFlag;	/*  + slant correction */
            dist=(double)image[r*COLS+c]+cp[6];
            P[2][r*COLS+c]=sqrt((dist*dist)/(1.0+(tan(xangle)*tan(xangle))
                                             +(tan(yangle)*tan(yangle))));
            P[0][r*COLS+c]=tan(xangle)*P[2][r*COLS+c];
            P[1][r*COLS+c]=tan(yangle)*P[2][r*COLS+c];
            /*if(r == 0 && c == 0) {
                printf("\t sc %f xa %f ya %f d %f p0 %f p1 %f p2 %f\n",
                       SlantCorrection, xangle, yangle, dist, P[0][r*COLS+c], P[1][r*COLS+c], P[2][r*COLS+c])
            }*/
        }
    }

    /*for(r = 0; r < 3; ++r) {
        for(c = 0; c < 3; ++c){
            printf("%f %f %f\t", P[0][r*COLS+c], P[1][r*COLS+c], P[2][r*COLS+c]);
        }
        printf("\n") ;
    }*/


    //calculate surface normal

    //calculate the cross products
    pixel_dist = 3;
    for (r = 0; r < ROWS-pixel_dist; r++)
    {
        for (c = 0; c < COLS-pixel_dist; c++)
        {
            calculate_cross_product(P[0][r*COLS+c], P[1][r*COLS+c], P[2][r*COLS+c],
                                    P[0][(r+pixel_dist)*COLS+c], P[1][(r+pixel_dist)*COLS+c], P[2][(r+pixel_dist)*COLS+c],
                                    P[0][r*COLS+(c+pixel_dist)], P[1][r*COLS+(c+pixel_dist)], P[2][r*COLS+(c+pixel_dist)],
                                    (&C[0][r*COLS+c]), (&C[1][r*COLS+c]), (&C[2][r*COLS+c]));
        }
    }


    //region growth

    qh = 1;	/* queue head */
    qt = 0;	/* queue tail */
    count = 0;
    angularthreshold = 0.79;
    new_label = 30;
    region = 0;

    for (r = pixel_dist-1; r < ROWS - pixel_dist; r++)
    {
        for (c = pixel_dist-1; c < COLS - pixel_dist; c++)
        {
            //Queue logic
            seed = 1;
            for (i = -2; i < 3; i++)
            {
                for (j = -2; j < 3; j++)
                {
                    index = ((i + r) * COLS) + (j + c);
                    if (thresholded_image[index] == 255 || segmented_image[index] != 0)
                        seed = 0;

                }
            }
            // printf("seed checkpoint!\n");
            if (seed == 1) {
                //initialize sums
                sum[0] = C[0][r*COLS+c];
                sum[1] = C[1][r*COLS+c];
                sum[2] = C[2][r*COLS+c];

                averageX = C[0][r*COLS+c];
                averageY = C[1][r*COLS+c];
                averageZ = C[2][r*COLS+c];

                //queue[0] = r*COLS+c;

                count = 0;
                qh = 1;
                qt = 0;
                queue[qt] = r * COLS + c;
                //printf("r %d c %d qh %d qt %d\n", r, c, qh, qt);
                while(qh != qt)
                {

                    //re-initialize variables
                    int wsz_lo = -1;
                    int wsz_hi = 1;
                    for (r2 = wsz_lo; r2 <= wsz_hi; r2++)
                    {
                        for (c2 = wsz_lo; c2 <= wsz_hi; c2++)
                        {
                            /*printf("r = %d c = %d r2 = %d c2 = %d qh = %d qt = %d q[qt] %d si[x] = %d\n", r, c, r2, c2, qh, qt, queue[qt],
                                   segmented_image[(queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2)]);
*/
                            //border conditions, skip the pixel in question
                            if (r2 == 0  &&  c2 == 0)
                                continue;
                            //printf("checkpoint 1\n");
                            if ((queue[qt]/COLS+r2) < 0  ||  (queue[qt]/COLS+r2) >= ROWS-pixel_dist  ||
                                    (queue[qt]%COLS+c2) < 0  ||  (queue[qt]%COLS+c2) >= COLS-pixel_dist)
                                continue;
                            //printf("checkpoint 2\n");

                            if (segmented_image[(queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2)] != 0)
                                continue;
                            //printf("checkpoint 3\n");

                            //test criteria to join region
                            //angular threshold
                            //int idx = (queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2 ;
                            //printf("asn x %f y %f z %f\n", C[0][idx], C[1][idx], C[2][idx]);
                            calculate_angular_distance(averageX, averageY, averageZ,
                                                               C[0][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2],
                                                               C[1][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2],
                                                               C[2][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2], (&angulardistance));
                            //printf("checkpoint 3.5\n");
                            if(angulardistance > angularthreshold)
                            {
                                //printf("%f > %f, skip!\n", angulardistance, angularthreshold);
                                continue;
                            }
                            else {
                                ;//printf("%f < %f, adding!\n", angulardistance, angularthreshold);
                            }


                            //printf("checkpoint 4\n");
                            //printf("angulardistance = %lf\n", angulardistance);

                            //if angular distance within threshold range, do the following:
                            count++;

                            sum[0] +=  C[0][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2]; //
                            sum[1] +=  C[1][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
                            sum[2] +=  C[2][(queue[qt]/COLS+r2)*COLS+queue[qt]%COLS+c2];
                            averageX = sum[0] / count;
                            averageY = sum[1] / count;
                            averageZ = sum[2] / count;
                            /*printf("count = %d adding %d %f %f %f\n", count, (queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2),
                                   averageX, averageY, averageZ);*/

                            //give new label to pixel
                            segmented_image[(queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2)] = new_label;
                            //labels[(queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2)] = 1;
                            queue[qh] = (queue[qt]/COLS+r2)*COLS+(queue[qt]%COLS+c2);
                            qh = (qh + 1) % MAXQUEUE;
                            if (qh == qt)
                            {
                                printf("Max queue size exceeded\n");
                                exit(0);
                            }
/*
                            if(count > 10)
                                exit(0);
*/

                        } //end inner nested for loop
                    } //end inner for loop

                    qt = (qt + 1 )% MAXQUEUE;
                } //end while loop
                printf("X: %lf, Y: %lf, Z: %lf\n", averageX, averageY, averageZ);

                if(count > 0) {
                    int ktmp = 0;
                    for (k = 0; k < (128*128); k++) {
                        if (segmented_image[k] == new_label) {
                            ktmp++;
                            if(ktmp > 5) break;
                            //printf("%d %d %d\n", k, k/COLS, k%COLS);
                            segmented_image[k] = 0;
                        }
                    }

                    /*if(new_label == 120) {
                        int ktmp = 0;
                        for (k = 0; k < (128*128); k++) {
                            if (segmented_image[k] == new_label) {
                            ktmp++;
                            if(ktmp % 3 == 0)
                                segmented_image[k] = 255;
                            }
                        }
                    }*/
                    if (count < 100)
                    {
                        printf("count = %d, resetting label %d!\n", count, new_label);
                        for (k = 0; k < (128*128); k++)
                            if (segmented_image[k] == new_label)
                                segmented_image[k] = 0;

                    }
                    else
                    {
                        region++;
                        new_label += 30;
                        printf("Region: %d, label %d Number of Pixels: %d\n", region, new_label, count);
                    }
                }
            } //if seed ends

        } //end outer nested for loop
    } //end outer for loop


    //outputs
/*
    for (i=0; i<10; i++)
    {
        printf("Region Number %d has ---%d---- Pixels\n", i, pixel_count[i]);
    }
*/
    fpt=fopen("thresholded_image.ppm","wb");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(thresholded_image,COLS*ROWS,1,fpt);
    fclose(fpt);

    fpt=fopen("segmented_image.ppm","wb");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(segmented_image,COLS*ROWS,1,fpt);
    fclose(fpt);
    return 0;
}
