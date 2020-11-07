/* LAB 3 -- LETTERS
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
#include <stdbool.h>

int test_1(unsigned char* image, int r, int c, int num_rows, int num_cols)
{
    if(r == 0) // first row, nothing to the North
        return 1;
    else if(c == num_cols-1) // last column, nothing to the East
        return 1;
    else if(r == num_rows - 1 && c == 0) // bottom-left corner
        return 1;
    else if(r == num_rows - 1) //last row
    {
        if(image[r * num_cols + (c - 1)] == 0)
            return 1; //W
        if(image[(r-1)*num_cols+c] == 0)
            return 1; //N
        if(image[r*num_cols+(c+1)] == 0)
            return 1; //E
    }
    else if(c == 0)
    {// left column
        if(image[(r+1) * num_cols + c] == 0)
            return 1; //S
        if(image[r*num_cols+(c+1)] == 0)
            return 1; //E
        if(image[(r-1)*num_cols+c] == 0)
            return 1; //N
    }
    else
    {
        if(image[(r-1)*num_cols + c] == 0)  return 1; // check N
        if(image[r * num_cols + c + 1] == 0) return 1; // check E
        if(image[(r+1) * num_cols + c] == 0 && image[r * num_cols + c - 1] == 0) return 1;// check S & W

    }
    return 0;
    }




int test_2(unsigned char* image, int r, int c, int num_rows, int num_cols, int* marked_edges)
{
    int num_edge_neighbors = 0;

    if (r == 0 && c == 0) {
        num_edge_neighbors = marked_edges[c+1];
            num_edge_neighbors += (marked_edges[(r+1)*num_cols + c    ]+
                                   marked_edges[(r+1)*num_cols + (c+1)]);
    }
    else if (r == 0 && c == num_cols -1) {
        num_edge_neighbors = marked_edges[c-1] ;
        num_edge_neighbors += (marked_edges[(r+1)*num_cols + (c-1)]+
                            marked_edges[(r+1)*num_cols + c    ]);
    }
    else if (r == 0) {
        num_edge_neighbors = marked_edges[c-1] + marked_edges[c+1];
        num_edge_neighbors += (marked_edges[(r+1)*num_cols + (c-1)]+
                            marked_edges[(r+1)*num_cols + c    ]+
                            marked_edges[(r+1)*num_cols + (c+1)]);
    }
    else if(r == num_rows - 1 && c == 0)
    {
        num_edge_neighbors = marked_edges[(r-1)*num_cols+c] +
                            marked_edges[(r-1)*num_cols+(c+1)]+
                            marked_edges[(r)*num_cols+(c+1)];
    }
    else if(r == num_rows -1 && c == num_cols - 1)
    {
        num_edge_neighbors = marked_edges[(r-1)*num_cols+(c-1)]+
                             marked_edges[(r-1)*num_cols+c]+
                             marked_edges[r*num_cols+(c-1)];
    }
    else if(r == num_rows - 1)
    {
        num_edge_neighbors = marked_edges[(r-1)*num_cols+(c-1)]+
                             marked_edges[(r-1)*num_cols+c]+
                             marked_edges[(r-1)*num_cols+(c+1)]+
                             marked_edges[r*num_cols+(c-1)]+
                             marked_edges[r*num_cols+(c+1)];
    }
    else if (c == 0)
    {
        num_edge_neighbors = marked_edges[(r-1)*num_cols+c]+
                             marked_edges[(r-1)*num_cols+(c+1)]+
                             marked_edges[r*num_cols+(c+1)]+
                             marked_edges[(r+1)*num_cols+(c+1)]+
                             marked_edges[(r+1)*num_cols+c];
    }
    else if (c == num_cols - 1)
    {
        num_edge_neighbors = marked_edges[(r-1)*num_cols+c]+
                             marked_edges[(r-1)*num_cols+(c-1)]+
                             marked_edges[r*num_cols+(c-1)]+
                             marked_edges[(r+1)*num_cols+(c-1)]+
                             marked_edges[(r+1)*num_cols+c];
    }
    else
    {
        for(int i = -1; i <= 1; ++i)
        {
            for(int j = -1; j <= 1; ++j)
            {
                num_edge_neighbors += marked_edges[(r+i)*num_cols + (c+j)];
            }
        }
        num_edge_neighbors -= marked_edges[r*num_cols + c];
    }
    return (num_edge_neighbors >= 2 && num_edge_neighbors <= 6);
}


int test_3(unsigned char* image, int r, int c, int num_rows, int num_cols, int* marked_edges)
{
    int edge2non_edge = 0;

    // top left corner
    if(r == 0 && c == 0)
    {
        if(marked_edges[0 * num_cols + 1] == 1 && marked_edges[1 * num_cols + 1] == 0) edge2non_edge++ ;
        if(marked_edges[1 * num_cols + 1] == 1 && marked_edges[1 * num_cols + 0] == 0) edge2non_edge++ ;
        if(marked_edges[1 * num_cols + 0] == 1) edge2non_edge++;
    }
    //top right corner
    else if(r == 0 && c == num_cols-1)
    {
        if(marked_edges[(r+1) * num_cols + c] == 1 && marked_edges[(r+1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols + (c-1)] == 1 && marked_edges[(r)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r)*num_cols+(c-1)] == 1)edge2non_edge++;
    }
    //bottom left corner
    else if(r == num_rows-1 && c == 0)
    {
        if(marked_edges[(r-1)*num_cols+c] == 1 && marked_edges[(r-1)*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c+1)] == 1 && marked_edges[r*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c+1)] == 1) edge2non_edge++;
    }
    //bottom right corner
    else if (r == num_rows-1 && c == num_cols - 1)
    {
        if(marked_edges[r*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+c] == 1) edge2non_edge++;
    }
    //top row
    else if (r == 0)
    {
        if(marked_edges[(r+1)*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+c] == 1 && marked_edges[(r+1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+(c-1)] == 1 && marked_edges[r*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c-1)] == 1) edge2non_edge++;
        if(marked_edges[r*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+(c+1)] == 0) edge2non_edge++;
    }
    //bottom row
    else if (r == num_rows -1 )
    {
        if(marked_edges[r*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+c] == 1 && marked_edges[(r-1)*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c+1)] == 1 && marked_edges[r*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c+1)] == 1) edge2non_edge++;
    }
    //left col
    else if (c == 0)
    {
        if(marked_edges[(r-1)*num_cols+c] == 1 && marked_edges[(r-1)*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c+1)] == 1 && marked_edges[r*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+c] == 1) edge2non_edge++;
    }
    //right col
    else if (c == num_cols - 1)
    {
        if(marked_edges[(r+1)*num_cols+c] == 1 && marked_edges[(r+1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+(c-1)] == 1 && marked_edges[r*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c-1)]  == 1 && marked_edges[(r-1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+c] == 1) edge2non_edge++;
    }
    // else
    else
    {
        if(marked_edges[(r+1)*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+c] == 1 && marked_edges[(r+1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r+1)*num_cols+(c-1)] == 1 && marked_edges[r*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+(c-1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c-1)] == 1 && marked_edges[(r-1)*num_cols+c] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+c] == 1 && marked_edges[(r-1)*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[(r-1)*num_cols+(c+1)] == 1 && marked_edges[r*num_cols+(c+1)] == 0) edge2non_edge++;
        if(marked_edges[r*num_cols+(c+1)] == 1 && marked_edges[(r+1)*num_cols+(c+1)] == 0) edge2non_edge++;
    }

    return edge2non_edge;
}

int main(int argc, char *argv[])
{

//Declare variables-----------------------------------------------------------------------------------------

//input files
    FILE    *fpt, *fpt1, *fpt2;

//allocating memory for images, input parameters
    unsigned char   *msf_image, *input_image, *copy_image, *thin_image;
    char    header[80], header1[80], letter;

//defining rows, cols and bytes
    int     ROWS, COLS, BYTES, r, c, BYTES1, COPY_cols, COPY_rows;

//defining index parameters for input image and iterators
    int     dr, dc, i, j;

//defining ROC parameters
    int     TP, FP, T, TN, FN;

//defining ground truth parameters
    int     letter_c, letter_r;
    char    gt_letter[10];

//detected parameters
    int     det, iterations, num_det;

//Copy-image parameters
    int     copy_dr, copy_dc;

//edge detector parameters
    int     *marked_edges, edge_to_nedge, edge_neighbor_count;

//thinning parameters
    int     lr, lc, skeleton, Th, mydr, mydc, *edge_transitions;

//endpoints and branchpoints
    int     endpoints, branchpoints;

//detected
    int     letter_detected;
//ROC
    int     FPR, TPR;
//images

//Read input image and template image, and ground truth file-------------------------------------------------



    if (argc != 2)
    {
        printf("Usage: LETTERS [letter]\n");
        exit(0);
    }

    letter = argv[1][0];

    printf("Looking for %c\n", letter);
    fpt = fopen("parenthood.ppm", "rb");
    fpt1 = fopen("msf_out.ppm", "rb");
    fpt2 = fopen("groundtruth.txt", "r");

    if(fpt == NULL)
    {
        printf("Unable to open %s for reading\n", "parenthood.ppm");
        exit(0);
    }
    if(fpt1 == NULL)
    {
        printf("Unable to open %s for reading\n", "msf_out.ppm");
        exit(0);
    }
    if(fpt2 == NULL)
    {
        printf("Unable to open %s for reading\n", "groundtruth.txt");
        exit(0);
    }

    //printf("Reading input files\n");
    i = fscanf(fpt, "%s %d %d %d ", header, &COLS, &ROWS, &BYTES);
    if (i != 4 || strcmp(header, "P5") != 0 || BYTES != 255)
    {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n", "parenthood.ppm");
        fclose(fpt);
        exit(0);
    }

    j = fscanf(fpt1, "%s %d %d %d ", header1, &c, &r, &BYTES1);
    if (j != 4 || strcmp(header, "P5") != 0 || BYTES != 255)
    {
        printf("%s is not an 8-bit PPM greyscale (P5) image\n", "msf.ppm");
        fclose(fpt1);
        exit(0);
    }

// Allocate memory for images and output image-------------------------------------------------------------------

    //printf("Allocating memory\n");
    input_image = (unsigned char *)calloc(ROWS*COLS, sizeof(unsigned char));
    if (input_image == NULL)
    {
        printf("Unable to allocate %d x %d memory\n", COLS, ROWS);
        exit(0);
    }
    header[0]=fgetc(fpt);	/* read white-space character that separates header */
    fread(input_image,1,COLS*ROWS,fpt);
    fclose(fpt);

    msf_image = (unsigned char *)calloc(r*c, sizeof(unsigned char));
    if (msf_image == NULL)
    {
        printf("Unable to allocate %d x %d memory\n", c, r);
        exit(0);
    }
    fread(msf_image,1,c*r,fpt1);
    fclose(fpt1);
    //printf("ROWS: %d\nCOLS: %d\nMSF_ROWS: %d\nMSF_COLS: %d\n", ROWS, COLS, r, c);

    COPY_cols = 9;
    COPY_rows = 15;
    copy_image = calloc(COPY_rows*COPY_cols, sizeof(unsigned char));
    marked_edges = calloc(COPY_rows*COPY_cols, sizeof(int));
    thin_image = calloc(COPY_rows*COPY_cols, sizeof(unsigned char));
    edge_transitions = calloc(COPY_rows*COPY_cols, sizeof(int));

//Looping through MSF Image-----------------------------------------------------------------------

    T = 210;
    printf("Analyzing at Threshold --> T = %d\n", T);
    for (dr = 7; dr < ROWS-7; dr++)
    {
        for (dc = 4; dc < COLS-4; dc++)
        {
            if (msf_image[dr*COLS+dc] > T)
            {
                msf_image[dr*COLS+dc] = 255;
            }
            else
            {
                msf_image[dr*COLS+dc] = 0;
            }
        }
    }

//Writing the binary image output
    //printf("Converting MSF image into binary\n");
    fpt=fopen("binary_out.ppm","w");
    fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
    fwrite(msf_image,COLS*ROWS,1,fpt);
    fclose(fpt);

//Looping through the ground truth text file
    COPY_cols = 9;
    num_det = TP = FP = 0;
    fpt=fopen("groundtruth.txt","r");
    //printf("Algorithm begins.\n");
    iterations = 0;

    while (iterations < 6)
    {
        iterations += 1;
        i=fscanf(fpt,"%s %d %d", gt_letter,&letter_c,&letter_r);
        if (i != 3)
        {
            break;
        }
        det = 0;
        mydr = 0;
        mydc = 0;

        //Obtaining ground truth locations
        for (dr=letter_r-7; det != 1 && dr<=letter_r+7; dr++)
        {
            for (dc=letter_c-4; det != 1 && dc<=letter_c+4; dc++)
            {
                if (msf_image[dr*COLS+dc] > T)
                {
                    det = 1;
                    mydr = letter_r;
                    mydc = letter_c;
                    //printf("Check in Original image at (%d, %d)\n", my_letter_c, my_letter_r);
                    num_det += 1;
                }
                else
                {
                    mydr = 0;
                    mydc = 0;
                    det = 0;
                }
            }
        }
        //printf("\n\n *******************************************\nCheck in Original image for %s at (%d, %d)\n", gt_letter, mydr, mydc);
        //Creating a 9x15 image from original image
        for(int i = 0; i < COPY_cols*COPY_rows; ++i)
            edge_transitions[i] = 0;
        if (det == 1)
        {
            /*
            printf("input image:\n");
            for (copy_dr = mydr - 7; copy_dr <= mydr + 7; copy_dr++)
                for (copy_dc = mydc - 4; copy_dc <= mydc + 4; copy_dc++)
                    printf("  %d  ", input_image[copy_dr*COLS+copy_dc]);
*/
            for (copy_dr = mydr - 7; copy_dr <= mydr + 7; copy_dr++)
            {
                for (copy_dc = mydc - 4; copy_dc <= mydc + 4; copy_dc++)
                {
                    //printf("Copying image at (%d, %d)\n", copy_dr, copy_dc);
                    //printf("mydr: %d    mydc: %d\n", mydr, mydc);
                    copy_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = input_image[copy_dr*COLS+copy_dc];
                    thin_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = input_image[copy_dr*COLS+copy_dc];

                    //Threshold at Th = 128 to create a binary image of original image.
                    Th = 128;
                    if(copy_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] < Th)
                    {
                        copy_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 0;
                        thin_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 0;
                        //edge = 1;
                        marked_edges[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 1;
                    }
                    else
                    {
                        copy_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 255;
                        thin_image[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 255;
                        //edge = 0;
                        marked_edges[(copy_dr - (mydr-7))*COPY_cols+(copy_dc - (mydc-4))] = 0;
                    }
                }
            }


            /*
            printf("\ncopy image:\n");
            for(copy_dr = 0; copy_dr < 9*15; copy_dr++)
                printf("  %d  ", copy_image[copy_dr]);
            printf("\nmarked edges:\n");
            for(copy_dr = 0; copy_dr < 9*15; copy_dr++)
                printf("  %d  ", marked_edges[copy_dr]);
                */
            //printf("\nBefore thinning....\n");
//--------------------------------------------------------------------------------------------------------------------------------------------------
            skeleton = 1;
            int num_pix_changed = 0;
            //THINNING THE THRESHOLDED IMAGE
            while(skeleton == 1)
            {
                skeleton = 0;
                for (copy_dr = 0; copy_dr < 15; copy_dr++)
                {
                    for (copy_dc = 0; copy_dc <9; copy_dc++)
                    {
#ifdef DEBUG
                        //printf("\nAt (%d, %d) th is %d\n", copy_dr, copy_dc, thin_image[copy_dr*COPY_cols+copy_dc]);
#endif // DEBUG

                        //for (m = 0; m<10; m++) {
                        if(thin_image[copy_dr*COPY_cols+copy_dc] == 0)   //pixel is an edge, do the following
                        {
                            /**
                            r3 1 1 1
                            r4 1 0 0
                            r5 1 0 1
                            **/
                            #ifdef DEBUG
                            printf("\t(%d %d): t1 %d\tt2 %d\tt3 %d\n", copy_dr, copy_dc,
                               test_1(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols),
                               test_2(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols, marked_edges),
                               test_3(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols, marked_edges));
                            #endif // DEBUG

                            edge_to_nedge = test_3(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols, marked_edges);
                            edge_transitions[copy_dr*COPY_cols+copy_dc] = edge_to_nedge;
                            if(test_1(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols) &&
                                    test_2(thin_image, copy_dr, copy_dc, COPY_rows, COPY_cols, marked_edges))
                            {
                                // printf("At (%d, %d) --> edge to non-edge transitions = %d\n", copy_dr, copy_dc, edge_to_nedge);
                                if(edge_to_nedge == 1)
                                {
                                    thin_image[copy_dr*COPY_cols+copy_dc] = 255; //Erase the edge by setting it equal to 255
                                    marked_edges[copy_dr*COPY_cols+copy_dc] = 0;
                                    skeleton = 1;
                                    num_pix_changed++ ;
                                    #ifdef DEBUG
                                    printf("Pixel erased at (%d,%d)\n", copy_dr, copy_dc);
                                    #endif // DEBUG
                                }

                            } // t_1 & t_2

                        } // marked_edge
                    } // copy_dc
                } // copy_dr
            }// skeleton == 1
            //printf("    %s: %d pixels changed\n", gt_letter, num_pix_changed);
            //Check for branch points and endpoints
            endpoints = 0;
            branchpoints = 0;
             for (copy_dr = 0; copy_dr < 15; copy_dr++)
            {
                for (copy_dc = 0; copy_dc < 9; copy_dc++)
                {
                    if(thin_image[copy_dr*COPY_cols+copy_dc] == 0)
                    {
                        if (edge_transitions[copy_dr*COPY_cols+copy_dc] == 1)
                        {
                            endpoints += 1;
                        }
                        if (edge_transitions[copy_dr*COPY_cols+copy_dc] > 2)
                        {
                            branchpoints += 1;
                        }
                    }
                }
            }
            #ifdef DEBUG
            printf("    ep %d bp %d\n", endpoints, branchpoints);
            #endif // DEBUG
            //mark if letter is detected or undetected

            if (endpoints == 1 && branchpoints == 1)
            {
                letter_detected = 1;
            }
            else
            {
                letter_detected = 0;
            }

            //mark if TP or FP
            if(letter_detected == 1 && gt_letter[0] == letter )
            {
                TP += 1;
            }
            else
            {
                FP += 1;
            }

        } // if det == 1

    } // while(1)


//--------------------------------------OUTPUTS-------------------------------------------------------------------------------------
//Sample Copied Image
    fpt = fopen("copied_image.ppm", "w");
    fprintf(fpt,"P5 %d %d 255\n",COPY_cols,COPY_rows);
    fwrite(copy_image,COPY_cols*COPY_rows,1,fpt);
    fclose(fpt);


//Sample Thin Image
    fpt = fopen("thin_image.ppm", "w");
    fprintf(fpt,"P5 %d %d 255\n",COPY_cols,COPY_rows);
    fwrite(thin_image,COPY_cols*COPY_rows,1,fpt);
    fclose(fpt);

//Calculating FP, TP, TPR and FPR at T

    FN = 151-TP;
    TN = 1111-FP;

    printf("Number of iterations: %d\n", iterations);
    printf("Retrieved Outputs from Analysis: \n\n");
    printf("Detected = %d\n", num_det);
    printf("TP = %d       ", TP);
    printf("FP = %d\n", FP);
    printf("FN = %d         ", FN);
    printf("TN = %d\n", TN);

    printf("End of analysis.");

    return 0;
} // main

