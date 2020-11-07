#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define SQR(x) ((x)*(x))

int find_distance(int x1, int y1, int x2, int y2)
{
    int distance = 0;

    distance = sqrt(SQR(x2-x1) + SQR(y2-y1));

    return distance;
}
int sqr_distance(int x1, int y1, int x2, int y2)
{
    //calculates internal energy 1
    //square of distance between points
    //double distance = 0;
    int result = 0;

    result = SQR(find_distance(x1, y1, x2, y2));
    //result = (int) SQR(distance);

    return result;
    //unnecessary step
}

int sqr_dev(int average_distance, int x1, int y1, int x2, int y2)
{
    //(x1,y1) --> current contour point
    //(x2, y2) --> next contour point
    //square of deviation from the average distance between points

    int current_distance = 0;
    int deviation = 0;

    current_distance = (int) find_distance(x1, y1, x2, y2);
    deviation = (int) (SQR(average_distance-current_distance));

    return deviation;

}

void mark_contour_point(int r, int c, unsigned char* image, int COLS)
{
    int r1,c1;

    //Set 7x7 window black

    for (r1 = -3; r1 <=3; r1++)
    {
        for (c1 = -3; c1 <= 3; c1++)
        {
            //if ((r1>=-1 && r1<=1) || (c1>=-1 && c1<=1)) //big crosses
            if ((r1 == 0 || c1 == 0))
            {
                //printf("r1 = %d c1 = %d r = %d c = %d\n", r1, c1, r, c);
                image[(r+r1)*COLS+(c+c1)] =  0;
            }
            else
                continue;
        }
    }
    return;
}

int find_gradient_magnitude(int horizontal_value, int vertical_value)
{
    int magnitude = 0;

    magnitude = (int) ((sqrt(SQR(horizontal_value) + SQR(vertical_value))));

    return magnitude;
}

double normalize(int current_value, int newMin, int newMax, int min_value, int max_value)
{
    double normalized_value;

    normalized_value = ((double) current_value - (double) min_value)*(((double)newMax - (double) newMin)/((double) max_value - (double) min_value));

    return normalized_value;
}

int main(int argc, char *argv[])

{
    FILE	        *fpt;
    unsigned char   *image, *initial_contour_image, *final_contour_image, *iter_contour_image, *sobel_image;
    int             ROWS, COLS, BYTES;
    char            header[80];
    int             r, c, r1, c1, r2, c2, rp, cp;

    int	            px[100],py[100];
    int             pr[100],pc[100];
    int	            i,total_points;
    //int	            window,x,y;
    int             x1, y1, x2, y2, average_distance;
    double          cur_dist, total_distance;

    int             iteration;
    int             *sobel_horizontal_filter, *sobel_vertical_filter;
    int             *sobel_horizontal, *sobel_vertical;
    int             *internal_energy1, *internal_energy2;
    double          *normalized_internal1, *normalized_internal2, *normalized_external;
    double          *total_normalized_energy;
    int             total_intensity;

    int             winrows = 13, wincols = 13;
    int             prow, pcol;
    int             dist_energy, dev_energy;
    int             *sob_hor, *sob_ver;
    //int             window_total_intensity;
    //int             tr, tc;
    int             *conv_hor, *conv_ver;
    int             *external_win_energy;
    int             ext_energy;
    int             min_int1, min_int2, min_ext, max_int1, max_int2, max_ext;
    int             *sobel_buffer, *normalized_sobel;
    double          min_energy, total_energy_value;
    int             img_min, img_max, x;
    int				final_pixel_row[200], final_pixel_col[200];


    printf("Reading inputs\n");
    if (argc != 2)
    {
        printf("Usage:  active [image.ppm] [window]\n");
        exit(0);
    }
    /* --------------Open image for reading----------------*/
    //window=atoi(argv[2]);
    fpt=fopen(argv[1],"rb");
    if (fpt == NULL)
    {
        printf("Unable to open %s for reading\n",argv[1]);
        exit(0);
    }
    /* read image header (simple 8-bit greyscale PPM only) */
    i=fscanf(fpt,"%s %d %d %d ",header,&COLS,&ROWS,&BYTES);
    if (i != 4  ||  strcmp(header,"P5") != 0  ||  BYTES != 255)
    {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n",argv[1]);
        fclose(fpt);
        exit(0);
    }

    printf("Allocating memory\n");
    /* allocate dynamic memory for image */
    image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
    if (image == NULL)
    {
        printf("Unable to allocate %d x %d memory\n",COLS,ROWS);
        exit(0);
    }
    /* read image data from file */
    fread(image,1,ROWS*COLS,fpt);
    //can use memcopy to copy image
    fclose(fpt);

    /* allocate memory for output images */
    initial_contour_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    final_contour_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    iter_contour_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    sobel_buffer = (int *)calloc(ROWS*COLS, sizeof(int));
    sobel_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    sobel_horizontal = (int *)calloc(ROWS*COLS, sizeof(int));
    sobel_vertical = (int *)calloc(ROWS*COLS, sizeof(int));

    conv_hor = (int *)calloc(ROWS*COLS, sizeof(int));
    conv_ver = (int *)calloc(ROWS*COLS, sizeof(int));

    normalized_sobel = (int *)calloc(ROWS*COLS, sizeof(int));

    internal_energy1 = (int *)calloc(winrows*wincols, sizeof(int));
    internal_energy2 = (int *)calloc(winrows*wincols, sizeof(int));
    sob_hor = (int *)calloc(winrows*wincols, sizeof(int));
    sob_ver = (int *)calloc(winrows*wincols, sizeof(int));

    external_win_energy = (int *)calloc(winrows*wincols, sizeof(int));
    normalized_internal1 = (double *)calloc(winrows*wincols, sizeof(double));
    normalized_internal2 = (double *)calloc(winrows*wincols, sizeof(double));
    normalized_external = (double *)calloc(winrows*wincols, sizeof(double));
    total_normalized_energy = (double *)calloc(winrows*wincols, sizeof(double));


    printf("Reading contour points\n");
    /*----------------read contour points file---------------*/
    if ((fpt=fopen("hawk_init.txt","r")) == NULL)
    {
        printf("Unable to open %s for reading\n",argv[1]);
        exit(0);
    }

    //Copy images to produce output images

    for (r = 0; r<ROWS; r++)
    {
        for (c = 0; c<COLS; c++)
        {
            initial_contour_image[r*COLS+c] = image[r*COLS+c];
            final_contour_image[r*COLS+c] = image[r*COLS+c];
            sobel_image[r*COLS+c] = image[r*COLS+c];
        }
    }

    // Read contour coordinates and store them in arrays
    total_points=0;
    /*-----------------loop over contour points--------------*/
    while (1)
    {
        i=fscanf(fpt,"%d %d",&px[total_points],&py[total_points]);

        //coordinate system is 4th quadrant cartesian. Must invert y values.
        //Row = -y
        //Col =  x
        //copy the points into more understandable variables
        pc[total_points] = px[total_points];
        pr[total_points] = py[total_points];
        if (i != 2)
            break;
        //printf("pr[%d] pc[%d] total_points = %d\n", pr[total_points], pc[total_points], total_points);
        total_points++;
        if (total_points > 100)
            break;

    }
    fclose(fpt);

    //Generate output: plot initial contour points on hawk.ppm

    //px[total_points] = x coordinates, py[total_points] = y coordinates
    printf("Marking initial contours on hawk.ppm\n");
    for (total_points = 0; total_points<42; total_points++)
    {
        //printf("pr[%d] = %d pc[%d] = %d px[%d] = %d py[%d] = %d\n", total_points, pr[total_points],
        //total_points, pc[total_points],total_points, px[total_points], total_points, py[total_points]);
        mark_contour_point(pr[total_points], pc[total_points], initial_contour_image, COLS);
    }

    fpt = fopen("initial_contour_image.ppm", "wb");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(initial_contour_image,ROWS*COLS,1,fpt);
    fclose(fpt);

    printf("Calculating average distance\n");
    //calculate the average distance between the points from the given coordinates
    total_distance = average_distance = 0;
    x1 = pc[0];
    y1 = pr[0]; //initial total_points --> first coordinate pair
    for (total_points = 0; total_points < 42; total_points++)
    {
        if (total_points == 41)
        {
            x1 = pc[41];
            y1 = pr[41];
            x2 = pc[0];
            y2 = pr[0];
        }
        else
        {
            //define second coordinate pair
            x2 = pc[total_points+1];
            y2 = pr[total_points+1];
        }
        //calculate distance between them
        cur_dist = find_distance(x2, y2, x1, y1);
        total_distance += cur_dist;
        x1 = x2;
        y1 = y2;
    }

    //calculate average distance between contour points
    average_distance = (int)(total_distance/42.0);
    printf("\n** ** ** Average distance = %d ** ** **\n\n", average_distance);

    printf("Initializing sobel filters\n");
    //Sobel initialization
    int sobrows = 3;
    int sobcols = 3;

    sobel_horizontal_filter = (int *)calloc(sobrows*sobcols, sizeof(int));
    sobel_vertical_filter = (int *)calloc(sobrows*sobcols, sizeof(int));

    sobel_horizontal_filter[0] = -1;
    sobel_horizontal_filter[1] = 0;
    sobel_horizontal_filter[2] = 1;
    sobel_horizontal_filter[3] = -2;
    sobel_horizontal_filter[4] = 0;
    sobel_horizontal_filter[5] = 2;
    sobel_horizontal_filter[6] = -1;
    sobel_horizontal_filter[7] = 0;
    sobel_horizontal_filter[8] = 1;

    sobel_vertical_filter[0] = -1;
    sobel_vertical_filter[1] = -2;
    sobel_vertical_filter[2] = -1;
    sobel_vertical_filter[3] = 0;
    sobel_vertical_filter[4] = 0;
    sobel_vertical_filter[5] = 0;
    sobel_vertical_filter[6] = 1;
    sobel_vertical_filter[7] = 2;
    sobel_vertical_filter[8] = 1;

    //Calculate SOBEL gradient image
    printf("Applying sobel filter to produce output sobel image\n");
    //Horizontal filter
    for (r1 = 1; r1<ROWS-1; r1++)
    {
        for (c1 = 1; c1<COLS-1; c1++)
        {
            total_intensity = 0;
            for (r2 = -1; r2<=1; r2++)
            {
                for (c2 = -1; c2<=1; c2++)
                {
                    total_intensity += image[(r1+r2)*COLS+(c1+c2)]*sobel_horizontal_filter[(r2+1)*sobcols+(c2+1)];

                }
            }
            sobel_horizontal[r1*COLS+c1] = total_intensity; //add magnitude to SOBEL Image
            conv_hor[(r1)*COLS+(c1)] = total_intensity;
        }
    }
    //printf("*********sobel_horizontal[80200] = %d\n",  sobel_horizontal[140700]);
    r1 = c1 = c2 = r2 = 0;
    total_intensity = 0;
    //sobel_value = 0;

    //Vertical filter
    for (r1 = 1; r1<ROWS-1; r1++)
    {
        for (c1 = 1; c1<COLS-1; c1++)
        {
            total_intensity = 0;
            for (r2 = -1; r2<=1; r2++)
            {
                for (c2 = -1; c2<=1; c2++)
                {
                    total_intensity += image[(r1+r2)*COLS+(c1+c2)]*sobel_vertical_filter[(r2+1)*sobcols+(c2+1)];
                }
            }
            sobel_vertical[r1*COLS+c1] = total_intensity;
            conv_ver[(r1)*COLS+(c1)] = total_intensity;
        }
    }
    //printf("*********sobel_vertical[80200] = %d\n",  sobel_vertical[140700]);

    //calculating the gradient magnitude and storing it in sobel image
    for (r = 1; r<ROWS-1; r++)
    {
        for (c = 1; c<COLS-1; c++)
        {
            sobel_buffer[r*COLS+c] = find_gradient_magnitude(sobel_horizontal[r*COLS+c], sobel_vertical[r*COLS+c]);

            //printf("%d %d %d\n", r, c, sobel_buffer[r*COLS+c]);
        }
    }
    //find min and max of original image
    img_min = 50000;
    img_max = 0;
    for (r = 1; r<ROWS-1; r++)
    {
        for (c = 1; c<COLS-1; c++)
        {
            if (sobel_buffer[r*COLS+c]<img_min)
                img_min = sobel_buffer[r*COLS+c];
            if (sobel_buffer[r*COLS+c]>img_max)
                img_max = sobel_buffer[r*COLS+c];
        }
    }

    //normalize the ints

    for (r = 1; r<ROWS-1; r++)
    {
        for (c = 1; c<COLS-1; c++)
        {
            normalized_sobel[r*COLS+c] = normalize(sobel_buffer[r*COLS+c], 0, 255, img_min, img_max);
        }
    }
    printf("*********normalized[80200] = %d\n",  normalized_sobel[80200]);


    //convert int to unsigned char

    for (r = 1; r<ROWS-1; r++)
    {
        for (c = 1; c<COLS-1; c++)
        {
            sobel_image[r*COLS+c] = (unsigned char) normalized_sobel[r*COLS+c];
        }
    }

    fpt = fopen("sobel_image.ppm", "wb");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(sobel_image,ROWS*COLS,1,fpt);
    fclose(fpt);

    //Normalize the sobel image
        //???

    //invert the sobel image

    for (r=1; r<ROWS-1; r++)
    {
        for (c=1; c<COLS-1; c++)
        {
            sobel_image[r*COLS+c] = (255-sobel_image[r*COLS+c]); //working, verified
        }
    }

    fpt = fopen("sobel_image_inverted.ppm", "wb");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(sobel_image,ROWS*COLS,1,fpt);
    fclose(fpt);

    //CONTOURING ALGORITHM BEGINS
    printf("Algorithm begins! Please wait...");

    iteration = 0;
    r1 = c1 = r2 = c2 = 0;


    while(iteration < 35)
    {
        //ACTIVE CONTOURING ALGORITHM
        printf("Entered while loop %d for contouring!\n", iteration);

        //printf("Resetting the contour points...\n");

            //printf("Re-calculating average distance...\n");

	int p;
	total_distance = average_distance = 0;
	x1 = pc[0];
	y1 = pr[0]; //initial total_points --> first coordinate pair
	for (p = 0; p < 42; p++)
	{
	    if (p == 41)
	    {
		x1 = pc[41];
		y1 = pr[41];
		x2 = pc[0];
		y2 = pr[0];
	    }
	    else
	    {
		//define second coordinate pair
		x2 = pc[p+1];
		y2 = pr[p+1];
	    }
	    //calculate distance between them
	    cur_dist = find_distance(x2, y2, x1, y1);
	    total_distance += cur_dist;
	    x1 = x2;
	    y1 = y2;
	}
        average_distance = (int)(total_distance/42.0);

        //printf("At %d th iteration, the average is %d\n", iteration, average_distance);

        for (total_points = 0; total_points < 42; total_points++)
        {
            r1 = pr[total_points];
            c1 = pc[total_points];
            if (total_points == 41){
                r2 = pr[0];
                c2 = pc[0];
            }
            else
            {
                r2 = pr[total_points + 1];
                c2 = pc[total_points + 1];
            }
            //printf("Breakpoint 1, iteration = %d\n", iteration);
            //printf("Entered for loop!\nr1 = %d  c1=%d\n", r1, c1); //verified
            max_ext = max_int1 = max_int2 = 0;
            min_ext = min_int1 = min_int2 = 20000000;
            //calculating internal energy for all pixels in 7x7 window around r,c
	    // printf("MARK: %d %d %d %d %d\n", total_points, r1, c1, r2, c2);
	    int win_lo = -6, win_hi = 6, win_mid = 6;
            for (prow = win_lo; prow <= win_hi; prow++)
            {
                for (pcol = win_lo; pcol <= win_hi; pcol++)
                {
                    //printf("Entered window for the '%d'th contour point!\n", total_points);
                    //printf("prow = %d   pcol = %d\n", prow, pcol);
                    rp = r1+prow; //verified
                    cp = c1+pcol; //verified

                    //printf("r2 = %d   c2 = %d\n", r2, c2);

                    dist_energy = (int) sqr_distance(rp, cp, r2, c2); //verified, working
                    dev_energy = (int) sqr_dev(average_distance, rp, cp, r2, c2); //verified, working

                    //printf("distance_energy = %d\ndeviation_energy = %d\n", dist_energy, dev_energy); //verified

                    //Add to energy matrices for this contour point
                    internal_energy1[(prow+win_mid)*wincols+(pcol+win_mid)] = dist_energy;
                    internal_energy2[(prow+win_mid)*wincols+(pcol+win_mid)] = dev_energy;

                    //printf("internal_energy1(dist_energy) = %d   internal_energy2(dev_energy) = %d\n", internal_energy1[prow*wincols+pcol],
                           //internal_energy2[prow*wincols+pcol]); //verified, same as dist_energy and dev_energy
                    //calculate external energy

                    //printf("Preparing to calculate external energy!\n");
                    //printf("HORIZONTAL CONVOLUTION AT PROW = %d   PCOL = %d\n", prow, pcol);



		    // printf("WARN: point %d : %d %d %d, %d %d %d | %d\n",total_points, r1, prow, winrows, c1, pcol, wincols, (r1+prow)*wincols+(c1+pcol));

                    sob_hor[(prow + win_mid)*wincols+(pcol + win_mid)] = conv_hor[(r1+prow)*COLS+(c1+pcol)];

                    sob_ver[(prow + win_mid)*wincols+(pcol + win_mid)] = conv_ver[(r1+prow)*COLS+(c1+pcol)];

                    //find magnitude gradient

		    external_win_energy[(prow + win_mid)*wincols+(pcol + win_mid)] =
			    find_gradient_magnitude(sob_hor[(prow + win_mid)*wincols+(pcol + win_mid)],
						    sob_ver[(prow + win_mid)*wincols+(pcol + win_mid)]);

                    ext_energy = external_win_energy[(prow + win_mid)*wincols+(pcol + win_mid)];
                    //printf("External gradient magnitude at prow = %d   pcol = %d\n is   %d",
                           //prow, pcol, external_win_energy[(r1+prow)*wincols+(c1+pcol)]);

                    //keep track of min and max for each energy to normalize

                    //MIN

                    if(dist_energy<min_int1)
                    {
                        min_int1 = dist_energy;
                    }
                    if(dev_energy<min_int2)
                    {
                        min_int2 = dev_energy;
                    }
                    if(ext_energy<min_ext)
                    {
                        min_ext = ext_energy;
                    }

                    //MAX

                    if(dist_energy>max_int1)
                    {
                        max_int1 = dist_energy;
                    }
                    if(dev_energy>max_int2)
                    {
                        max_int2 = dev_energy;
                    }
                    if(ext_energy>max_ext)
                    {
                        max_ext = ext_energy;
                    }
                }
            } //window loop ends

            //normalize the individual energies
            for (prow = win_lo; prow <= win_hi; prow++)
            {
                for (pcol = win_lo; pcol <= win_hi; pcol++)
                {
                    normalized_internal1[(prow+win_mid)*wincols+(pcol+win_mid)] = normalize(internal_energy1[(prow+win_mid)*wincols+(pcol+win_mid)], 0, 1, min_int1, max_int1);
                    normalized_internal2[(prow+win_mid)*wincols+(pcol+win_mid)] = normalize(internal_energy2[(prow+win_mid)*wincols+(pcol+win_mid)], 0, 1, min_int2, max_int2);
                    normalized_external[(prow+win_mid)*wincols+(pcol+win_mid)] = normalize(external_win_energy[(prow + win_mid)*wincols+(pcol + win_mid)], 0, 1, min_ext, max_ext);
                    //printf("Normalized values: int1 = %lf    int2 = %lf    ext = %lf\n",
                        //normalized_internal1[prow*wincols+pcol], normalized_internal2[prow*wincols+pcol],normalized_external[prow*wincols+pcol]);
                }
            }

            //invert external window

            for (prow = win_lo; prow <= win_hi; prow++)
            {
                for (pcol = win_lo; pcol <= win_hi; pcol++)
                {
                    normalized_external[(prow+win_mid)*wincols+(pcol+win_mid)] = 0-normalized_external[(prow+win_mid)*wincols+(pcol+win_mid)];
                }
            }

            //combine energies
            min_energy = 100000.0;
	    int me_r = 0;
	    int me_c = 0;


            for (prow = 0; prow <= 2*win_mid; prow++)
            {
                for (pcol = 0; pcol <=2*win_mid; pcol++)
                {
					if(iteration == 23) {
					printf("\t%d: (%d, %d):(%d, %d): %f %f %f\n", total_points, pr[total_points], pc[total_points],
						prow, pcol, normalized_internal1[prow*wincols+pcol],
						normalized_internal2[prow*wincols+pcol],
						normalized_external[prow*wincols+pcol]);
					}
                    total_normalized_energy[prow*wincols+pcol] = ((0.75)*normalized_internal1[prow*wincols+pcol]) +
                                                                 ((3.81)*normalized_internal2[prow*wincols+pcol]) +
                                                                 ((2)*normalized_external[prow*wincols+pcol]);
                    total_energy_value = total_normalized_energy[prow*wincols+pcol];
                    if (total_energy_value < min_energy)
                    {
			/*
                        pr[total_points] = pr[total_points] + prow-3;
                        pc[total_points] = pc[total_points] + pcol-3;
			*/
                        min_energy = total_energy_value;
						me_r = prow - win_mid;
						me_c = pcol - win_mid;
                    }
                }
            }
	    if(me_r != 0 || me_c != 0) {
		printf("%d: %f, %d %d -> %d %d\n", total_points, min_energy, pr[total_points], pc[total_points], me_r, me_c);
		pr[total_points] += me_r;
		pc[total_points] += me_c;
	    }

        } //contour point ends
        iteration++;

	for (int tmpr = 0; tmpr<ROWS; tmpr++)
	{
	    for (int tmpc = 0; tmpc<COLS; tmpc++)
	    {
		iter_contour_image[tmpr*COLS+tmpc] = final_contour_image[tmpr*COLS+tmpc];
	    }
	}
	for (x = 0; x<42; x++)
	{
	    mark_contour_point(pr[x], pc[x], iter_contour_image, COLS);


	}
	char* fname = (char *)malloc(40);
	sprintf(fname, "fci_%d.ppm", iteration);
	printf("file %s\n", fname);
	fpt = fopen(fname, "wb");
	if(fpt == NULL) {
	    printf("ERROR!\n");
	    exit(2);
	}
	fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
	fwrite(iter_contour_image,ROWS*COLS,1,fpt);
	fclose(fpt);
	free(fname);
    } //while loop ends

    for (x = 0; x<42; x++)
    {
        //printf("pr[%d] = %d pc[%d] = %d px[%d] = %d py[%d] = %d\n", total_points, pr[total_points],
        //total_points, pc[total_points],total_points, px[total_points], total_points, py[total_points]);
        mark_contour_point(pr[x], pc[x], final_contour_image, COLS);
		final_pixel_row[x] = pr[x];
		final_pixel_col[x] = pc[x];
		printf("Point %d is at row = %d		col = %d\n", x, final_pixel_row[x], final_pixel_col[x]);
    }

    fpt = fopen("final_contour_image.ppm", "wb");
    if(fpt == NULL) {
        printf("ERROR!\n");
        exit(2);
    }
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(final_contour_image,ROWS*COLS,1,fpt);
    fclose(fpt);

    return 0;

}


