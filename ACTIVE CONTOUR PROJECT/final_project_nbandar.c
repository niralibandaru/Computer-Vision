#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <windows.h>
#include <wingdi.h>
#include <winuser.h>
#include <process.h>	/* needed for multithreading */
#include "resource.h"
#include "globals.h"


int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPTSTR lpCmdLine, int nCmdShow)

{
	MSG			msg;
	HWND		hWnd;
	WNDCLASS	wc;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, "ID_PLUS_ICON");
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = "ID_MAIN_MENU";
	wc.lpszClassName = "PLUS";

	if (!RegisterClass(&wc))
		return(FALSE);

	hWnd = CreateWindow("PLUS", "plus program",
		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
		CW_USEDEFAULT, 0, 400, 400, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return(FALSE);

	ShowScrollBar(hWnd, SB_BOTH, FALSE);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	MainWnd = hWnd;

	flag = 0;

	strcpy(filename, "");
	OriginalImage = NULL;
	ROWS = COLS = 0;

	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return(msg.wParam);
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)

{
	HMENU				hMenu;
	OPENFILENAME		ofn;
	FILE* fpt;
	HDC					hDC;
	char				header[320], text[320];
	int					BYTES, xPos, yPos;

	switch (uMsg)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_CLEARIMAGE:
			PaintImage();
			state = 0;
			ThreadRunning = 0;
			ThreadRunning1 = 0;
			break;
		case ID_FILE_LOAD:
			if (OriginalImage != NULL)
			{
				free(OriginalImage);
				OriginalImage = NULL;
			}
			memset(&(ofn), 0, sizeof(ofn));
			ofn.lStructSize = sizeof(ofn);
			ofn.lpstrFile = filename;
			filename[0] = 0;
			ofn.nMaxFile = MAX_FILENAME_CHARS;
			ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY;
			ofn.lpstrFilter = "PNM files\0*.pnm\0All files\0*.*\0\0";
			if (!(GetOpenFileName(&ofn)) || filename[0] == '\0')
				break;		/* user cancelled load */
			if ((fpt = fopen(filename, "rb")) == NULL)
			{
				MessageBox(NULL, "Unable to open file", filename, MB_OK | MB_APPLMODAL);
				break;
			}
			fscanf(fpt, "%s\n %d\n %d\n %d\n", header, &COLS, &ROWS, &BYTES);
			if (strcmp(header, "P6") != 0 || BYTES != 255)
			{
				MessageBox(NULL, "Not a PNM image", filename, MB_OK | MB_APPLMODAL);
				fclose(fpt);
				break;
			}
			Image = (unsigned char*)calloc(ROWS * COLS * 3, 1);
			OriginalImage = (unsigned char*)calloc(ROWS * COLS, 1);
			header[0] = fgetc(fpt);	/* whitespace character after header */
			fread(Image, 1, ROWS * COLS * 3, fpt);
			image_loaded = 1;
			j = 0;
			for (i = 0; i < (ROWS * COLS * 3); i += 3)
			{
				OriginalImage[j] = (Image[i] + Image[i + 1] + Image[i + 2]) / 3;
				j++;
			}
			fclose(fpt);
			sob_hor = (int*)malloc(ROWS * COLS * sizeof(int));
			sob_ver = (int*)malloc(ROWS * COLS * sizeof(int));
			sobel_image = (unsigned char*)malloc(ROWS * COLS * sizeof(unsigned char));
			sobel_out = (unsigned char*)malloc(ROWS * COLS * sizeof(unsigned char));

			GenerateSobelImage(OriginalImage);
			SetWindowText(hWnd, filename);
			PaintImage();
			break;

		case ID_FILE_QUIT:
			DestroyWindow(hWnd);
			break;
		}
		break;
	case WM_SIZE:		  /* could be used to detect when window size changes */
		PaintImage();
		hDC = GetDC(MainWnd);
		for (i = 0; i < 120; i++)
		{
			SetPixel(hDC, balloonx[i], balloony[i], RGB(0, 0, 255));
		}
		for (i = 0; i < buff_count / 5; i++)
		{
			SetPixel(hDC, downsampled_cols[i], downsampled_rows[i], RGB(255, 0, 0));
		}
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_PAINT:
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_LBUTTONDOWN:
		if (wParam == MK_LBUTTON)
		{

			hDC = GetDC(MainWnd);
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			if (GetPixel(hDC, xPos, yPos) == RGB(255, 0, 0))
			{
				state = 1;
				for (i = 0; i < (buff_count / 5); i++)
				{
					fgt = 0;
					if (downsampled_cols[i] == xPos) x = i;
					if (downsampled_rows[i] == yPos) y = i;
				}
			}
			if (GetPixel(hDC, xPos, yPos) == RGB(0, 0, 255))
			{
				state = 2;
				for (i = 0; i < 120; i++)
				{
					fgt = 0;
					if (balloonx[i] == xPos) x = i;
					if (balloony[i] == yPos) y = i;
				}
			}
			if (state == 0) buff_count = 0;
			flag = 1;

			if (end_rb == 1)
			{
				drag_rb_down = 1;
				CheckPoint(xPos, yPos);
			}
			else if (end_balloon == 1)
			{
				drag_balloon_down = 1;
				CheckPoint(xPos, yPos);
			}
		}

	case WM_LBUTTONUP:
		if (wParam != MK_LBUTTON)
		{
			if (state != 1 && state != 2)
			{
				downsampled_cols = (int*)malloc((buff_count / 5) * sizeof(int));
				downsampled_rows = (int*)malloc((buff_count / 5) * sizeof(int));
				for (i = 0; i < (buff_count / 5); i++)
				{
					downsampled_cols[i] = bufferx[i * 5];
					downsampled_rows[i] = buffery[i * 5];
				}
				circle = 0;
			}
			if (state == 1) state = 0;
			if (state == 2) state = 0;
			flag = 0;

			if (dragging_rb == 1)
			{
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);

				new_x = xPos;
				new_y = yPos;

				downsampled_cols[original_x] = new_x;
				downsampled_cols[original_y] = new_y;
				active_shrink(MainWnd);
			}

			else if (dragging_balloon == 1)
			{
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);

				new_x = xPos;
				new_y = yPos;

				balloonx[original_x] = new_x;
				balloony[original_y] = new_y;
				active_balloon(MainWnd);
			}
		}
	case WM_RBUTTONDOWN:
		if (wParam == MK_RBUTTON)
		{
			xPos = LOWORD(lParam);
			yPos = HIWORD(lParam);

			circle = 1;
		}
	case WM_RBUTTONUP:
		if (wParam == MK_LBUTTON)
			break;
		if (wParam != MK_RBUTTON)
		{
			if (circle == 1)
			{
				hDC = GetDC(MainWnd);
				xPos = LOWORD(lParam);
				yPos = HIWORD(lParam);
				j = 0;
				for (i = 0; i < 360; i++)
				{
					xt = (int)(10 * cos(i * 3.1415 / 180));
					yt = (int)(10 * sin(i * 3.1415 / 180));
					if (i % 3 == 0)
					{
						balloonx[j] = xPos + xt;
						balloony[j] = yPos + yt;
						j++;
					}
				}
				PaintImage();
				for (i = 0; i < 120; i++)
				{
					SetPixel(hDC, balloonx[i], balloony[i], RGB(0, 0, 255));
				}
				circle = 0;
			}
		}
	case WM_MOUSEMOVE:
		hDC = GetDC(MainWnd);
		xPos = LOWORD(lParam);
		yPos = HIWORD(lParam);
		if (flag == 1)
		{
			if (xPos >= 0 && xPos < COLS && yPos >= 0 && yPos < ROWS)
			{
				bufferx[buff_count] = xPos;
				buffery[buff_count] = yPos;
				if (buff_count % 5 == 0) {
					for (int r = xPos - 3; r <= xPos + 3; r++)
					{
						for (int c = yPos - 3; c <= yPos + 3; c++)
						{
							if (r == xPos || c == yPos) {
								SetPixel(hDC, r, c, RGB(255, 0, 0));	//color the cursor position red
							}
						}
					}
				}
				buff_count++;
			}
		}
		if (drag_approved_rb == 1)
		{
			dragging_rb = 1;
		}
		else if (drag_approved_balloon == 1)
		{
			dragging_balloon = 1;
		}
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_KEYDOWN:

		if ((TCHAR)wParam == '1')
		{
			ThreadRunning = 1;
			_beginthread(active_shrink, 0, MainWnd);
		}
		if ((TCHAR)wParam == '2')
		{
			ThreadRunning1 = 1;
			_beginthread(active_balloon, 0, MainWnd);
		}

		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case ID_CONTOUR_DRAG:
		if (ThreadRunning == 1)
		{
			drag = 1;
		}
		if (ThreadRunning1 == 1)
		{
			drag = 2;
		}
		if (drag == 1)
		{
			drag = 0;
		}
	case WM_HSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();	  /* direct PaintImage calls eliminate flicker; the alternative is InvalidateRect(hWnd,NULL,TRUE); UpdateWindow(hWnd); */
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_VSCROLL:	  /* this event could be used to change what part of the image to draw */
		PaintImage();
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return(DefWindowProc(hWnd, uMsg, wParam, lParam));
		break;
	}

	hMenu = GetMenu(MainWnd);
	if (drag == 1 || drag == 2)
		CheckMenuItem(hMenu, ID_CONTOUR_DRAG, MF_CHECKED);	/* you can also call EnableMenuItem() to grey(disable) an option */
	else {
		CheckMenuItem(hMenu, ID_CONTOUR_DRAG, MF_UNCHECKED);
	}

	return(0L);
}


void GenerateSobelImage(unsigned char* image) {

	FILE* fpt;

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

	for (r = 1; r < ROWS - 1; r++)
	{
		for (c = 1; c < COLS - 1; c++)
		{
			sum = 0;
			for (r2 = -1; r2 <= 1; r2++)
			{
				for (c2 = -1; c2 <= 1; c2++)
				{
					sum += OriginalImage[(r + r2) * COLS + (c + c2)] * sobel_horizontal_filter[(1 + r2) * 3 + (1 + c2)];
				}
			}
			sob_hor[r * COLS + c] = sum;
			sum = 0;
			for (r2 = -1; r2 <= 1; r2++)
			{
				for (c2 = -1; c2 <= 1; c2++)
				{
					sum += OriginalImage[(r + r2) * COLS + (c + c2)] * sobel_vertical_filter[(1 + r2) * 3 + (1 + c2)];
				}
			}
			sob_ver[r * COLS + c] = sum;
		}
	}
	for (r = 0; r < ROWS; r++)
	{
		for (c = 0; c < COLS; c++)
		{
			sum = sqrt(SQR(sob_hor[r * COLS + c]) + SQR(sob_ver[r * COLS + c]));
			sobel_image[r * COLS + c] = (unsigned char)sum;
		}
	}
	for (r = 0; r < ROWS; r++)
	{
		for (c = 0; c < COLS; c++)
		{
			sobel_out[r * COLS + c] = 255 - sobel_image[r * COLS + c];
		}
	}

	fpt = fopen("sobel_image.ppm", "wb");
	fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
	fwrite(sobel_out, ROWS * COLS, 1, fpt);
	fclose(fpt);

}

void PaintImage()

{
	PAINTSTRUCT			Painter;
	HDC					hDC, hdcScaled;
	BITMAPINFOHEADER	bm_info_header;
	BITMAPINFO* bm_info;
	int					i, r, c, DISPLAY_ROWS, DISPLAY_COLS;
	unsigned char* DisplayImage;
	HBITMAP				hbmScaled;
	if (OriginalImage == NULL)
		return;		/* no image to draw */

			  /* Windows pads to 4-byte boundaries.  We have to round the size up to 4 in each dimension, filling with black. */
	DISPLAY_ROWS = ROWS;
	DISPLAY_COLS = COLS;
	if (DISPLAY_ROWS % 4 != 0)
		DISPLAY_ROWS = (DISPLAY_ROWS / 4 + 1) * 4;
	if (DISPLAY_COLS % 4 != 0)
		DISPLAY_COLS = (DISPLAY_COLS / 4 + 1) * 4;
	DisplayImage = (unsigned char*)calloc(DISPLAY_ROWS * DISPLAY_COLS, 1);
	for (r = 0; r < ROWS; r++)
		for (c = 0; c < COLS; c++)
			DisplayImage[r * DISPLAY_COLS + c] = OriginalImage[r * COLS + c];

	BeginPaint(MainWnd, &Painter);
	hDC = GetDC(MainWnd);
	bm_info_header.biSize = sizeof(BITMAPINFOHEADER);
	bm_info_header.biWidth = DISPLAY_COLS;
	bm_info_header.biHeight = -DISPLAY_ROWS;
	bm_info_header.biPlanes = 1;
	bm_info_header.biBitCount = 8;
	bm_info_header.biCompression = BI_RGB;
	bm_info_header.biSizeImage = 0;
	bm_info_header.biXPelsPerMeter = 0;
	bm_info_header.biYPelsPerMeter = 0;
	bm_info_header.biClrUsed = 256;
	bm_info_header.biClrImportant = 256;
	bm_info = (BITMAPINFO*)calloc(1, sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD));
	bm_info->bmiHeader = bm_info_header;
	for (i = 0; i < 256; i++)
	{
		bm_info->bmiColors[i].rgbBlue = bm_info->bmiColors[i].rgbGreen = bm_info->bmiColors[i].rgbRed = i;
		bm_info->bmiColors[i].rgbReserved = 0;
	}
		/*
	RECT rc;
	GetClientRect(MainWnd, &rc);
	hdcScaled = CreateCompatibleDC(hDC);
	hbmScaled = CreateCompatibleBitmap(hDC, DISPLAY_COLS, DISPLAY_ROWS);
	SelectObject(hdcScaled, hbmScaled);
	SetDIBitsToDevice(hdcScaled, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, 0, 0,
		0, // first scan line
		DISPLAY_ROWS, // number of scan lines
		DisplayImage, bm_info, DIB_RGB_COLORS);
	StretchBlt(hDC, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcScaled, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, SRCCOPY);
	DeleteObject(hdcScaled);
	DeleteObject(hbmScaled);
	*/
	SetDIBitsToDevice(hDC, 0, 0, DISPLAY_COLS, DISPLAY_ROWS, 0, 0,
		0, // first scan line
		DISPLAY_ROWS, // number of scan lines
		DisplayImage, bm_info, DIB_RGB_COLORS);

	ReleaseDC(MainWnd, hDC);
	EndPaint(MainWnd, &Painter);


	free(DisplayImage);
	free(bm_info);
}




void active_shrink(HWND AnimationWindowHandle)

{
	HDC		hDC;
	float		max1 = 18.0, min1 = 0.0;
	float		max2, min2;
	float		max3 = 65025.0, min3 = 0.0;
	float		internal_energy1, internal_energy2, external_energy;
	float		sum1, dist_energy, mini;
	int			j, tempx, tempy;
	long int	cc, cr;

	int count = buff_count / 5;

	end_rb = 0;
	while (ThreadRunning == 1 && state != 1 && iteration < 100)
	{
		hDC = GetDC(MainWnd);

		sum = cc = cr = 0;

		for (j = 0; j < (count); j++) {
			cc += downsampled_cols[j];
			cr += downsampled_rows[j];
		}
		cc /= (count);
		cr /= (count);

		/* calculate energy terms for contours in 7x7 window */
		for (j = 0; j < (count); j++) {
			for (r2 = -3; r2 <= 3; r2++)
			{
				for (c2 = -3; c2 <= 3; c2++)
				{
					dist_energy = SQR(abs(downsampled_cols[j] + c2 - cc)) + SQR(abs(downsampled_rows[j] + r2 - cr));
					if (r2 == -3 && c2 == -3)
					{
						min2 = max2 = dist_energy;
					}
					else {
						if (dist_energy > max2) max2 = dist_energy;
						if (dist_energy < min2) min2 = dist_energy;
					}
				}
			}

			for (r2 = -3; r2 <= 3; r2++)
			{
				for (c2 = -3; c2 <= 3; c2++)
				{
					dist_energy = SQR(abs(downsampled_cols[j] + c2 - cc)) + SQR(abs(downsampled_rows[j] + r2 - cr));
					if ((r2 == 0) && (c2 == 0))
					{
						internal_energy1 = 0;
					}
					else
					{
						internal_energy1 = (SQR(abs(c2)) + (SQR(abs(r2))) - min1) / (max1 - min1);
					}

					internal_energy2 = (dist_energy - min2) / (max2 - min2);
					external_energy = (SQR((sobel_image[(downsampled_rows[j] + r2) * COLS + (downsampled_cols[j] + c2)])) - min3) / (max3 - min3);
					external_energy *= 1.4;
					if ((r2 == -3) && (c2 == -3)) {
						mini = internal_energy1 + internal_energy2 - external_energy;
						tempx = downsampled_cols[j] + c2;
						tempy = downsampled_rows[j] + r2;
					}
					else
					{
						if (((internal_energy1 + internal_energy2 - external_energy) < mini)) {
							mini = internal_energy1 + internal_energy2 - external_energy;
							tempx = downsampled_cols[j] + c2;
							tempy = downsampled_rows[j] + r2;
						}
					}
				}
			}
			downsampled_cols[j] = tempx;
			downsampled_rows[j] = tempy;
		}
		/* Draw during iterations */

		PaintImage();

		for (i = 0; i < 120; i++)
		{
			SetPixel(hDC, balloonx[i], balloony[i], RGB(0, 0, 255));
		}
		for (i = 0; i < count; i++)
		{
			for (r2 = -3; r2 <= 3; r2++)
			{
				for (c2 = -3; c2 <= 3; c2++)
					if (r2 == 0 || c2 == 0) {
						SetPixel(hDC, downsampled_cols[i] + c2, downsampled_rows[i] + r2, RGB(255, 0, 0));
					}
			}
		}
		iteration++;
		/* Sleep thread 10ms for viewing */
		Sleep(10);
	}
	end_rb = 1;
}

void active_balloon(HWND AnimationWindowHandle)

{
	HDC		    hDC;
	float		max1 = 18.0, min1 = 0.0;
	float		max2, min2;
	float		max3 = 65025.0, min3 = 0.0;
	float		internal_energy1, internal_energy2, external_energy;
	float		sum1, dist_energy, mini;
	int			j, tempx, tempy;
	long int	cc, cr;

	end_balloon = 0;
	iteration = 0;
	while (ThreadRunning1 == 1 && state != 2 && iteration < 100)
	{
		hDC = GetDC(MainWnd);

		sum = 0;
		cc = 0;
		cr = 0;
		for (j = 0; j < 120; j++) {
			cc += balloonx[j];
			cr += balloony[j];
		}
		cc /= 120;
		cr /= 120;

		/* calculate energy terms for contours in 7x7 window */
		for (j = 0; j < 120; j++) {
			for (r2 = -5; r2 <= 5; r2++)
			{
				for (c2 = -5; c2 <= 5; c2++)
				{
					dist_energy = SQR(abs(balloonx[j] + c2 - cc)) + SQR(abs(balloony[j] + r2 - cr));
					if (r2 == -5 && c2 == -5)
					{
						min2 = max2 = dist_energy;
					}
					else {
						if (dist_energy > max2) max2 = dist_energy;
						if (dist_energy < min2) min2 = dist_energy;
					}
				}
			}

			for (r2 = -5; r2 <= 5; r2++)
			{
				for (c2 = -5; c2 <= 5; c2++)
				{
					dist_energy = SQR(abs(balloonx[j] + c2 - cc)) + SQR(abs(balloony[j] + r2 - cr));
					if ((r2 == 0) && (c2 == 0)) internal_energy1 = 0;
					else
						internal_energy1 = (abs(c2) * abs(c2) + abs(r2) * abs(r2) - min1) / (max1 - min1);
					internal_energy2 = (dist_energy - max2) / (min2 - max2);
					external_energy = (sobel_image[(balloony[j] + r2) * COLS + (balloonx[j] + c2)] * sobel_image[(balloony[j] + r2) * COLS + (balloonx[j] + c2)] - min3) / (max3 - min3);
					internal_energy1 *= 0.5;
					internal_energy2 *= 1.3;
					external_energy *= 0.4;
					if ((r2 == -5) && (c2 == -5)) {
						mini = internal_energy1 + internal_energy2 - external_energy;
						tempx = balloonx[j] + c2;
						tempy = balloony[j] + r2;
					}
					else {
						if (((internal_energy1 + internal_energy2 - external_energy) < mini)) {
							mini = internal_energy1 + internal_energy2 - external_energy;
							tempx = balloonx[j] + c2;
							tempy = balloony[j] + r2;
						}
					}
				}
			}
			balloonx[j] = tempx;
			balloony[j] = tempy;
		}
		/* Draw during iterations */

		PaintImage();

		for (i = 0; i < 120; i++)
		{
			for (r2 = -3; r2 <= 3; r2++)
			{
				for (c2 = -3; c2 <= 3; c2++)
				{
					if (r2 == 0 || c2 == 0)
					{
						SetPixel(hDC, balloonx[i] + c2, balloony[i] + r2, RGB(0, 0, 255));
					}
				}
			}
		}

		iteration++;
		Sleep(10);
	}
	end_balloon = 1;
}

void CheckPoint(int x, int y)
{
	if (drag_rb_down == 1)
	{
		for (i = 0; i < buff_count / 5; i++)
		{
			for (r = x - 3; r<=3; r++)
			{
				for (c = y - 3; c <= 3; c++)
				{
					if (x == downsampled_rows[i] && y == downsampled_cols[i])
					{
						drag_approved_rb = 1;
						original_x = x;
						original_y = y;
					}
					else continue;
				}
			}
		}
	}
	else if (drag_balloon_down == 1)
	{
		for (i = 0; i < 120; i++)
		{
			for (r = x - 3; r <= 3; r++)
			{
				for (c = y - 3; c <= 3; c++)
				{
					if (x == balloonx[i] && y == balloony[i])
					{
						drag_approved_balloon = 1;
						original_x = x;
						original_y = y;
					}
					else continue;
				}
			}
		}
	}
}
