#include <stdio.h>
#include <jpeglib.h>
#include <string.h>
#include <jerror.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <jconfig.h>
#include <jmorecfg.h>

#define WIDTH 640
#define HEIGHT 480
#define SIGLE_CHANNEL_SIZE      (WIDTH * HEIGHT)
#define YUYV_SIZE               (WIDTH * HEIGHT * 2)
#define YUV420_SIZE             (WIDTH * HEIGHT / 2 * 3)

int yuv420p_to_jpeg(const char *filename, char *pdata, int image_width, int image_height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    FILE *outfile; // target file
    if ((outfile = fopen(filename, "wb+")) == NULL)
    {
        printf("can't open %s\n", filename);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, outfile);
    cinfo.image_width = image_width; // image width and height, in pixels
    cinfo.image_height = image_height;
    cinfo.input_components = 3;       // # of color components per pixel
    cinfo.in_color_space = JCS_YCbCr; //colorspace of input image
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);

    cinfo.jpeg_color_space = JCS_YCbCr;
    cinfo.comp_info[0].h_samp_factor = 2;
    cinfo.comp_info[0].v_samp_factor = 2;
    jpeg_start_compress(&cinfo, TRUE);
    JSAMPROW row_pointer[1];

    unsigned char *yuvbuf;
    if ((yuvbuf = (unsigned char *)malloc(image_width * 3)) != NULL)
        memset(yuvbuf, 0, image_width * 3);
    else
    {
        printf("fail alloc \n");
        return -1;
    }
    unsigned char *ybase, *ubase;
    ybase = pdata;
    ubase = pdata + image_width * image_height;
    int j = 0;
    while (cinfo.next_scanline < cinfo.image_height)
    {
        int idx = 0;
        for (int i = 0; i < image_width; i++)
        {
            yuvbuf[idx++] = ybase[i + j * image_width];
            yuvbuf[idx++] = ubase[j / 2 * image_width + (i / 2) * 2];
            yuvbuf[idx++] = ubase[j / 2 * image_width + (i / 2) * 2 + 1];
        }
        row_pointer[0] = yuvbuf;
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        j++;
    }
    free(yuvbuf);
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    return 0;
}

int main()
{
    char buff[YUV420_SIZE];
    const char filename[20] = "picture1.jpg";
    int fp1 = open("picture1.yuv", O_RDONLY);
    if (fp1 < 0)
    {
        printf("can`t open p1.yuv \n");
        return 1;
    }    
    read(fp1, buff, YUYV_SIZE);
    int ret = yuv420p_to_jpeg(filename, buff, WIDTH, HEIGHT, 80);
    if(ret < 0)
    {
        printf("error *********************\n");
        return -1;
    }
    printf("end convernt \n");
    return 0;
}