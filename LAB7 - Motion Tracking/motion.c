/* NIRALI BANDARU
   INTRODUCTION TO COMPUTER VISION
   DR. ADAM HOOVER
   LAB 7 - MOTION TRACKING
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define SQR(x) ((x)*(x))
#define accX 0.01870728

double* smooth(double *arr, int arr_length, int smooth_window)
{
    //Declare variables
    double *smoothed_data;
    int i,j;
    double sum = 0;
    int half_window = 0;

    //allocate memory to output array
    smoothed_data = calloc(arr_length, sizeof(double));

    //for ease in indexing, declare half_window
    half_window = smooth_window/2;

    //calculate two-tailed moving average for smoothing of data
    for (i = half_window; i < arr_length-half_window; i++)
    {
        sum = 0;
        for (j = i-half_window; j < i+half_window; j++)
        {
            sum += arr[j];
        }
        smoothed_data[i] = (double)((double) sum/ (double) smooth_window);
    }

    //return smoothed data
    return smoothed_data;
}

double calculate_variance(double *arr, int arr_length, int variance_window, int current_point)
{

    int i;
    double sum = 0;
    double avg = 0;
    double variance = 0, power = 0;
    int window = 0;

    //Account for corner cases

    if ((current_point + variance_window) < arr_length)
    {
        window = variance_window;
    }
    else
    {
        window = abs(current_point - arr_length);
    }

    // calculate sum of window
    for (i = current_point; i < current_point + window; i++)
    {
        sum += arr[i];
    }

    // calculate average
    avg = (double) sum/ (double) window;

    // calculate variance

    for (i = 0; i < window; i++)
    {
        power += (double) pow((arr[current_point + i]-avg), 2);
    }
    variance = (double) power/ (double) (window-1);

    return variance;
}

double calculate_distance_gyro(double element)
{
    double sample_time = 0.05;
    double distance = 0;

    distance = element * sample_time;

    return distance;
}

double calculate_distance_acc(double element, double previous_velocity)
{
    double sample_time = 0.05;
    double velocity = 0, average_velocity = 0;
    double distance_traveled = 0;

    average_velocity = (double) (velocity+previous_velocity)/2.0;
    distance_traveled = average_velocity*sample_time;

    return distance_traveled;
}

double calculate_velocity(double element)
{
    double gravity = 9.81;
    double sample_time = 0.05;

    double velocity = element*gravity*sample_time;

    return velocity;
}
int main(int argc, char *argv[])
{
    //VARIABLE DECLARATION

    //storage

    FILE        *fpt, *fpt1, *variance_file, *original_data_file;
    double      *time_array, *acc_x, *acc_y, *acc_z;
    double      *pitch_array, *roll_array, *yaw_array;

    char        x[50], y[50], z[50], pitch[50], roll[50], yaw[50];
    char        time[50];

    double      segment[100], segment_x[100], segment_y[100], segment_z[100];
    double      segment_time[100], segment_pitch[100], segment_roll[100], segment_yaw[100];

    int         *filesize;
    char        file_name[250], c;

    //Iterators, loop variables

    int         i = 0, j = 0;
    int         line;

    //Smooth data variables

    int         smooth_window;
    double      *smoothed_accx, *smoothed_accy, *smoothed_accz;
    double      *smoothed_pitch, *smoothed_yaw, *smoothed_roll;
    int         arr_length = 1250;

    //Variance

    int         variance_window;
    double      var_accx, var_accy, var_accz, var_pitch, var_yaw, var_roll;
    double      accx_threshold, accy_threshold, accz_threshold, pitch_threshold, roll_threshold, yaw_threshold;

    //Moving/Still variables

    int         moving, still, startmovement = 0, endmovement = 0;
    double      start_time, end_time;
    double      sample_time = 0.05;
    double      starting_points_array[25], ending_points_array[25];

    //distance, velocity variables

    double      previous_velocity_x = 0, previous_velocity_y = 0, previous_velocity_z = 0;
    double      velocity_x = 0, velocity_y = 0, velocity_z = 0;
    double      velocity_pitch = 0, velocity_roll = 0, velocity_yaw = 0;
    double      distance_x = 0, distance_y = 0, distance_z = 0;
    double      distance_pitch = 0, distance_roll = 0, distance_yaw = 0;
    double      gravity = 9.81;

    double      distance_x_array[25], distance_y_array[25],distance_z_array[25], distance_pitch_array[25],
                distance_roll_array[25], distance_yaw_array[25];

    double      total_distance_x, total_distance_y, total_distance_z, total_distance_pitch, total_distance_roll,
                total_distance_yaw;

    int         count = 0;
    int         starting_point = 0, ending_point = 0, start_index_array[25], end_index_array[25];


    //LOAD FILE AND ARGUMENTS

    if (argc != 1)
    {
        printf("Usage:  motion tracking\n");
        exit(0);
    }

    fpt = fopen("motion_data.txt", "r");

    if (fpt == NULL)
    {
        printf("ERROR: Unable to open motion_data.txt for reading\n");
        exit(0);
    }

    sprintf(file_name, "original_data.csv");
    original_data_file = fopen(file_name, "w");


    //ALLOCATE MEMORY

    time_array = calloc(arr_length, sizeof(double));
    acc_x = calloc(arr_length, sizeof(double));
    acc_y = calloc(arr_length, sizeof(double));
    acc_z = calloc(arr_length, sizeof(double));
    roll_array = calloc(arr_length, sizeof(double));
    pitch_array = calloc(arr_length, sizeof(double));
    yaw_array = calloc(arr_length, sizeof(double));

    smoothed_accx = calloc(arr_length, sizeof(double));
    smoothed_accy = calloc(arr_length, sizeof(double));
    smoothed_accz = calloc(arr_length, sizeof(double));
    smoothed_pitch = calloc(arr_length, sizeof(double));
    smoothed_yaw = calloc(arr_length, sizeof(double));
    smoothed_roll = calloc(arr_length, sizeof(double));


    //READ TEXT FILE
    fscanf(fpt, "%s %s %s %s %s %s %s\n", time, x,
                   y, z, pitch, roll, yaw);
    for (line = 0; line < arr_length; line++)
    {
        i = fscanf(fpt, "%s %s %s %s %s %s %s\n", time, x,
                   y, z, pitch, roll, yaw);
        time_array[line] = strtod(time, NULL);
        acc_x[line] = strtod(x, NULL);
        acc_y[line] = strtod(y, NULL);
        acc_z[line] = strtod(z, NULL);
        pitch_array[line] = strtod(pitch, NULL);
        roll_array[line] = strtod(roll, NULL);
        yaw_array[line] = strtod(yaw, NULL);
        /*
        printf("%s %.10f\t", time, time_array[line]);
        printf("%s %.10f\t", x, acc_x[line]);
        printf("%s %.10f\t", y, acc_y[line]);
        printf("%s %.10f\t", z, acc_z[line]);
        printf("%s %.10f\t", pitch, pitch_array[line]);
        printf("%s %.10f\t", roll, roll_array[line]);
        printf("%s %.10f\n", yaw, yaw_array[line]);*/
    }
    fclose(fpt);

    fpt1 = fopen("original_data.csv", "w");
    fprintf(fpt1, "Time,AccX,AccY,AccZ,Pitch,Roll,Yaw\n");
    for (i = 0; i < arr_length; i++)
    {
        fprintf(fpt1, "%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                time_array[i], acc_x[i], acc_y[i], acc_z[i], pitch_array[i], roll_array[i],yaw_array[i]);
    }
    fclose(fpt1);


    //SMOOTH THE DATA

    smooth_window = 10;

    smoothed_accx = smooth(acc_x, arr_length, smooth_window);
    smoothed_accy = smooth(acc_y, arr_length, smooth_window);
    smoothed_accz = smooth(acc_z, arr_length, smooth_window);
    smoothed_pitch = smooth(pitch_array, arr_length, smooth_window);
    smoothed_roll = smooth(roll_array, arr_length, smooth_window);
    smoothed_yaw = smooth(yaw_array, arr_length, smooth_window);

    //Save smoothed data

    fpt1 = fopen("smoothed_data.csv", "w");
    fprintf(fpt1, "Time,AccX,AccY,AccZ,Pitch,Roll,Yaw\n");
    for (i = 0; i < arr_length; i++)
    {
        fprintf(fpt1, "%.10f,%.10f,%.10f,%.10f,%.10f,%.10f,%.10f\n",
                time_array[i], smoothed_accx[i], smoothed_accy[i], smoothed_accz[i], smoothed_pitch[i], smoothed_roll[i], smoothed_yaw[i]);
    }
    fclose(fpt1);

    //CALCULATE VARIANCE
    variance_window = 11;
    count = 0;
    sprintf(file_name, "variance.csv");
    variance_file = fopen(file_name, "w");
    if(variance_file == NULL) {
        printf("ERROR: variance file is null!\n");
        exit(1);
    }

    for (i = 0; i < arr_length; i++)
    {
        //initialize variances
        var_accx = var_accy = var_accz = var_pitch = var_yaw = var_roll = 0;

        //calculate variance
        var_accx = calculate_variance(acc_x, arr_length, variance_window, i);
        var_accy = calculate_variance(acc_y, arr_length, variance_window, i);
        var_accz = calculate_variance(acc_z, arr_length, variance_window, i);
        var_pitch = calculate_variance(pitch_array, arr_length, variance_window, i);
        var_yaw = calculate_variance(yaw_array, arr_length, variance_window, i);
        var_roll = calculate_variance(roll_array, arr_length, variance_window, i);

        fprintf(variance_file, "%d, %.10f, %.10f, %.10f, %.10f, %.10f, %.10f\n", i, var_accx, var_accy, var_accz, var_pitch, var_roll, var_yaw);

        //After plotting the variances, it was found that the following values are suitable thresholds to segment the data

        accx_threshold = 0.045;
        accy_threshold = 0.005;
        accz_threshold = 0.04;
        pitch_threshold = 0.006;
        roll_threshold  = 0.006;
        yaw_threshold   = 0.006;

        //If variance is above threshold, segment = moving. Else, segment = still

        if ((var_accx > accx_threshold) ||
            (var_accy > accy_threshold) ||
            (var_accz > accz_threshold) ||
            (var_pitch > pitch_threshold) ||
            (var_roll > roll_threshold) ||
            (var_yaw > yaw_threshold))
        {
            moving = 1;
        }
        else
        {
            moving = 0;
        }

        //DETERMINE IF MOVING OR STILL

        if (moving == 1 && startmovement == 0)
        {
            startmovement = 1;
            starting_point = i;
            start_time = starting_point*sample_time;
        }


        if (moving == 0 && startmovement == 1 && endmovement == 0)
        {
            endmovement = 1;
            ending_point = i;
            end_time = ending_point*sample_time;
        }

        //CALCULATE LINEAR DISTANCE AND ANGULAR MOTION

        //if we have reached the end of a moving segment
        if (startmovement == 1 && endmovement == 1)
        {
            //keep track of starting and ending points by storing in an array
            starting_points_array[count] = start_time;
            ending_points_array[count] = end_time;
            start_index_array[count] = starting_point;
            end_index_array[count] = ending_point;

            distance_x = distance_y = distance_z = 0;
            velocity_x = velocity_y = velocity_z = 0;
            distance_pitch = distance_roll = distance_yaw = 0;


            for (j = starting_point; j < ending_point; j++)
            {
                //accelerometer distance calculation - double integration

                previous_velocity_x = velocity_x;
                velocity_x += calculate_velocity(acc_x[j]);
                distance_x += calculate_distance_acc(acc_x[j], previous_velocity_x);

                previous_velocity_y = velocity_y;
                velocity_y += calculate_velocity(acc_y[j]);
                distance_y += calculate_distance_acc(acc_y[j], previous_velocity_y);

                previous_velocity_z = velocity_z;
                velocity_z += calculate_velocity(acc_z[j]);
                distance_z += calculate_distance_acc(acc_z[j], previous_velocity_z);

                //gyroscopes angular distance calculation - integration

                distance_pitch += calculate_distance_gyro(pitch_array[j]);
                distance_roll += calculate_distance_gyro(roll_array[j]);
                distance_yaw += calculate_distance_gyro(yaw_array[j]);

            } //end for loop

            total_distance_x += distance_x;
            total_distance_y += distance_y;
            total_distance_z += distance_z;
            total_distance_pitch += distance_pitch;
            total_distance_roll += distance_roll;
            total_distance_yaw += distance_yaw;

            distance_x_array[count] = distance_x;
            distance_y_array[count] = distance_y;
            distance_z_array[count] = distance_z;
            distance_pitch_array[count] = distance_pitch;
            distance_roll_array[count] = distance_roll;
            distance_yaw_array[count] = distance_yaw;

            startmovement = 0;
            endmovement = 0;
            start_time = 0;
            end_time = 0;

            count += 1;

        } //end movement segment loop

        moving = 0;

    } //end loop for determining movement
    fclose(variance_file);


    printf("count = %d\n", count);
    printf("Start End dx dy dz p r y\n");
    // loop to print distance_*_array to file
    for (line = 0; line < count; line++)
    {
        printf("%d %d %f %f %.10f %.10f %.10f %.10f %.10f %.10f\n", start_index_array[line], end_index_array[line],
               starting_points_array[line], ending_points_array[line],
               distance_x_array[line],distance_y_array[line], distance_z_array[line],
               distance_pitch_array[line],distance_roll_array[line], distance_yaw_array[line]);
    }
    printf("\n\n");
    printf("total distance x = %.10f\n", total_distance_x);
    printf("total distance y = %.10f\n", total_distance_y);
    printf("total distance z = %.10f\n", total_distance_z);
    printf("total distance pitch = %.10f\n", total_distance_pitch);
    printf("total distance roll = %.10f\n", total_distance_roll);
    printf("total distance yaw = %.10f\n", total_distance_yaw);

    return 0;
}
