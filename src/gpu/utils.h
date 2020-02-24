#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void saveTextureToBMP(int w, int h, unsigned char* img, const char* path) {
    FILE* f = NULL;
    int filesize = 54 + 3 * w * h;  //w is your image width, h is image height, both int
   
    // header format
    unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
    unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
    unsigned char bmppad[3] = { 0,0,0 };

    bmpfileheader[2] = (unsigned char)(filesize);
    bmpfileheader[3] = (unsigned char)(filesize >> 8);
    bmpfileheader[4] = (unsigned char)(filesize >> 16);
    bmpfileheader[5] = (unsigned char)(filesize >> 24);

    bmpinfoheader[4] = (unsigned char)(w);
    bmpinfoheader[5] = (unsigned char)(w >> 8);
    bmpinfoheader[6] = (unsigned char)(w >> 16);
    bmpinfoheader[7] = (unsigned char)(w >> 24);
    bmpinfoheader[8] = (unsigned char)(h);
    bmpinfoheader[9] = (unsigned char)(h >> 8);
    bmpinfoheader[10] = (unsigned char)(h >> 16);
    bmpinfoheader[11] = (unsigned char)(h >> 24);

    // flop axis and RGB channels
    int imgsize = 3 * w * h;
    unsigned char* oimg = (unsigned char*)malloc(3 * w * h);
    for (int i = 0; i < imgsize; i++) {
        oimg[i] = img[imgsize - 1 - i];
    }

    // write
    errno_t err;
    err = fopen_s(&f, path, "wb");
    if (err == 0)
    {
        printf("The file was opened\n");
        fwrite(bmpfileheader, 1, 14, f);
        fwrite(bmpinfoheader, 1, 40, f);
        for (int i = 0; i < h; i++)
        {
            fwrite(oimg + (w * (h - i - 1) * 3), 3, w, f);
            fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
        }
        fclose(f);
    }
    else
    {
        printf("The was not opened\n");
    }
}
