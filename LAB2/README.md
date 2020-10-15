LAB 2

Optical character recognition

In this project each student must implement a matched filter (normalized cross-correlation) to recognize letters in an image of text.
The following materials are supplied at the web site and should be used:
parenthood.ppm
input image
parenthood_e_template.ppm
template image
parenthood_gt.txt
ground truth
The ground truth file lists all the letters and image pixel coordinates of text in the image. The pixel coordinates are for the center point of each letter.
The program should perform the following steps:
1. Read the input image, template image, and ground truth files.
2. Calculate the matched-spatial filter (MSF) image (remember, this is not 8-bits).
3. Normalize the MSF image to 8-bits.
4. Loop through the following steps for a range of T:
a. Threshold at T the normalized MSF image to create a binary image.
b. Loop through the ground truth letter locations.
i. Check a 9 x 15 pixel area centered at the ground truth location. If any pixel in the msf image is greater than the threshold, consider the letter “detected”. If none of the pixels in the 9 x 15 area are greater than the threshold, consider the letter “not detected”.
c. Categorize and count the detected letters as FP (“detected” but the letter is not ‘e’) and TP (“detected” and the letter is ‘e’).
d. Output the total FP and TP for each T.
Using any desired program, you must create an ROC curve from the program output.
Note that for each letter, considering a 9x15 window centered at its ground truth location, if any pixel in the MSF image is above the given threshold within this window, then the letter is considered “detected”. It does not matter how many pixels in the MSF image are above the threshold, the letter can only be detected once. If the detected letter is an “e” then it is a TP, otherwise it is a FP.
You must write a brief report that includes the code and the ROC curve. Show your MSF image. Identify the optimal T and FP and TP values at that threshold.
