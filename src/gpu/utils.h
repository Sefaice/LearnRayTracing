#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void saveTextureToBMP(int w, int h, uint8_t* img, const char* path) {
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

    // flip RGB channels, BMP stores BGR?
    int imgsize = 3 * w * h;
    unsigned char* oimg = (unsigned char*)malloc(3 * w * h);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            oimg[j * w * 3 + i * 3] = img[j * w * 3 + i * 3 + 2];
            oimg[j * w * 3 + i * 3 + 1] = img[j * w * 3 + i * 3 + 1];
            oimg[j * w * 3 + i * 3 + 2] = img[j * w * 3 + i * 3];
        }
    }

    // write
    errno_t err;
    err = fopen_s(&f, path, "wb");
    if (err == 0) {
        //printf("The file was opened\n");
        fwrite(bmpfileheader, 1, 14, f);
        fwrite(bmpinfoheader, 1, 40, f);
        for (int i = 0; i < h; i++)
        {
            fwrite(oimg + (i * w * 3), 3, w, f);
            fwrite(bmppad, 1, (4 - (w * 3) % 4) % 4, f);
        }
        fclose(f);
    }
    else {
        printf("The was not opened\n");
    }
}

void saveTextureToBinary_uint8(int w, int h, uint8_t* data, const char* path) {
   /* printf("TheEEEEEEEE %d", strlen(data));*/

    // write
    FILE* f = NULL;
    errno_t err;
    err = fopen_s(&f, path, "wb");
    if (err == 0) {
        //printf("The file was opened\n");
        for (int i = 0; i < h; i++)
        {
            fwrite(data + (h - i - 1) * w * 3, 3, w, f);
        }
        fclose(f);
    }
    else {
        printf("The file was not opened\n");
    }
}


void saveTextureToBinary_float(int w, int h, float* data, const char* path) {
    // write
    FILE* f = NULL;
    errno_t err;
    err = fopen_s(&f, path, "wb");
    if (err == 0) {
        //printf("The file was opened\n");
        for (int i = 0; i < h; i++)
        {
            fwrite(data + (h - i - 1) * w * 3, 12, w, f);
        }
        fclose(f);
    }
    else {
        printf("The file was not opened\n");
    }
}