#pragma once


#include "jpeglib.h"  

/**
libjpeg辅助函数
*/


static int libjpeg_test_format(void* data, int len);

 
//解码图片  使用free释放内存
static int libjpeg_decode(void* data, int len, void** outpixel, int* w, int* h);

//编码图片  使用free释放内存
static int libjpeg_encode(void** outdata, int* len, void* pixel, int w, int h, int quality);

//释放内存
//free


static void libjpeg_on_error(j_common_ptr cinfo){	
	//cinfo->client_data;
	//longjmp(myerr->setjmp_buffer, 1);
	throw 1;
}

int libjpeg_test_format(void* data, int len) {
	unsigned char* b = (unsigned char*)data;
	if (len < 4)return -1;
	if (b[0] == 0xff && b[1] == 0xd8 && b[2] == 0xff)return 0;
	return -1;
}


int libjpeg_decode(void* data, int len, void** opixel, int* ow, int* oh) {
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;
	int w,h,sz;
	unsigned char* pixel;
	JSAMPROW row_pointer[1];
	int stride;
	jerr.error_exit = libjpeg_on_error;
	cinfo.err = jpeg_std_error(&jerr);
	int ret = -1;
	jpeg_create_decompress(&cinfo);
	
	try {
		jpeg_mem_src(&cinfo, (unsigned char*)data, len);

		jpeg_read_header(&cinfo, TRUE);
		if (cinfo.output_components != 3)throw 2;

		w = cinfo.output_width;
		h = cinfo.output_width;
		sz = w * h * 3;
		pixel = (unsigned char*)malloc(sz);
		if (!pixel) throw ENOMEM;
		stride = w * 3;

		jpeg_start_decompress(&cinfo);
		
		while (cinfo.output_scanline < (unsigned int)h){
			row_pointer[0] = pixel;
			jpeg_read_scanlines(&cinfo, row_pointer, 1);
			pixel += stride;
		}

		jpeg_finish_decompress(&cinfo);

		ret = 0;
		(*opixel) = pixel;
		*ow = w;
		*oh = h;
	}
	catch (int e) {
		ret = e;
	}
	jpeg_create_decompress(&cinfo);
	return ret;
}


static int libjpeg_encode(void** outdata, int* len, void* pixel, int w, int h, int quality) {
	jpeg_compress_struct cinfo;
	jpeg_error_mgr jerr;
	int ret = -1;
	JSAMPROW row_pointer[1];
	unsigned char* buf = NULL;
	unsigned long sz = 0;
	unsigned char* ppixel = (unsigned char*) pixel;
	int stride;
	jerr.error_exit = libjpeg_on_error;
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	try {
		jpeg_mem_dest(&cinfo, &buf, &sz);

		cinfo.image_width = w;
		cinfo.image_height = h;
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, 1);
		jpeg_start_compress(&cinfo, TRUE);
		stride = w * 3;

		while (cinfo.next_scanline < (unsigned int)h){
			row_pointer[0] = ppixel;
			jpeg_write_scanlines(&cinfo, row_pointer, 1);
			ppixel += stride;
		}

		jpeg_finish_compress(&cinfo);

		ret = 0;
		(*outdata) = buf;
		*len = sz;
	}
	catch (int e) {
		ret = e;
	}
	
	jpeg_destroy_compress(&cinfo);
	return ret;
}


