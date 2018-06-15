
#include "xgraphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clocks.h"
//#include "libyuv.h"
#include <assert.h>
//using namespace libyuv;

void* readFileContent(const char* name, size_t* plen) {
	FILE* f = fopen(name, "rb");
	if (!f)return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);
	fseek(f, 0, SEEK_SET);
	void* data = malloc(sz);
	size_t len = fread(data, 1, sz, f);
	if (plen)*plen = len;
	return data;
}

static void write_stdio(void* ctx, void* data, int len) {
	fwrite(data,1,len,(FILE*)ctx);
}

int wirteJpgToFile(image_t img, const char* filename) {
	FILE* f = fopen(filename, "wb");
	if (!f)return NULL;
	int ret = image_save(img, IMAGE_ENCODE_JPG, write_stdio, f);
	fclose(f);
	return ret;
}

void ShowImageInfo(image_t img) {
	printf("image: %dx%d\n", image_width(img), image_height(img));
}

void test() {
	uint64_t t1 = clock_get_tick();
	image_t img1 = image_load_from_file("1.jpg");
	image_t img2 = image_load_from_file("2.jpg");
	image_t img3 = image_load_from_file("3.png");
	image_t img4 = image_load_from_file("4.png");
	image_t img5 = image_load_from_file("5.png");
	image_t img6 = image_scaled(img2, 700, 700);
	font_family_t family = font_family_load_from_file("msyh.ttf");
	font_t font = font_create(family, 40, 0);
	uint64_t t2 = clock_get_tick();
	ShowImageInfo(img1);
	ShowImageInfo(img2);
	uint64_t t3 = clock_get_tick();
	rect rc = { 25,80,725,780 };
	rect part = { 30,30,180,180 };
	canvas_t c = canvas_create(img1);
	//canvas_draw_image_scaled(c, img2, &rc);
	//canvas_draw_image(c, img6, 25, 88);
	//canvas_draw_image_scaled_partial(c, img6, &rc, &part);
	canvas_draw_image(c, img3, 20, 20);;
	canvas_draw_text(c, font, 0xff0000, "hello world!", 500, 500);
	canvas_draw_image(c, img4, 60, 1000);
	canvas_draw_image(c, img5, 110, 110);
	uint64_t t4 = clock_get_tick();
	image_save_to_file(img1, IMAGE_ENCODE_JPG, "out.jpg");
	canvas_free(c);
	image_free(img1);
	image_free(img2);
	printf("decode used:%d\n", (int)(t2 - t1));
	printf("draw used:%d\n", (int)(t4 - t3));
}


void load(int count, const char* file) {
	uint64_t t1 = clock_get_tick();
	for (int i = 0; i < count; i++) {
		image_t img = image_load_from_file(file);
	}
	uint64_t t2 = clock_get_tick();
	printf("time used %d ms\n", (int)(t2-t1));
}

void draw(int count, const char* file) {
	image_t img = image_load_from_file(file);
	image_t back = image_alloc(500,500,IMAGE_TYPE_RGB);
	canvas_t c = canvas_create(back);
	uint64_t t1 = clock_get_tick();
	for (int i = 0; i < count; i++) {
		canvas_draw_image(c, img, 20, 20);
	}
	uint64_t t2 = clock_get_tick();
	printf("time used %d ms\n", (int)(t2 - t1));
}
void scale(int count, const char* file) {
	image_t img = image_load_from_file(file);	
	uint64_t t1 = clock_get_tick();
	for (int i = 0; i < count; i++) {
		image_t newimg = image_scaled(img, 500, 500);
	}
	uint64_t t2 = clock_get_tick();
	printf("time used %d ms\n", (int)(t2 - t1));
}
void font(int count, const char* file) {
	font_family_t family = font_family_load_from_file(file);
	//font_t font = font_create(family, 25, FONT_STYLE_UNDERLINE);
	font_t font = font_create(family, 25, FONT_STYLE_STRIKEOUT);

	image_t back = image_alloc(500, 500, IMAGE_TYPE_RGB);
	canvas_t c = canvas_create(back);
	uint64_t t1 = clock_get_tick();
	for (int i = 0; i < count; i++) {
		canvas_draw_text(c, font, 0xff1122, " abcdefg hijklmn ", 20, 20);
	}
	uint64_t t2 = clock_get_tick();
	image_save_to_file(back, IMAGE_ENCODE_JPG, "out.jpg");
	printf("time used %d ms\n", (int)(t2 - t1));
}

void test_font() {
	font_family_t family = font_family_load_from_file("msyh.ttf");
	//font_family_t family = font_family_load_from_file("1.jpg");
	font_t font = font_create(family, 25, FONT_STYLE_UNDERLINE&0);
	//image_t back = image_alloc(500, 500, IMAGE_TYPE_RGB);
	image_t back = image_load_from_file("1.jpg");
	image_t img = image_load_from_file("2.jpg");

	canvas_t c = canvas_create(back);
	//canvas_draw_text(c, font, 0xff1122, "hello world", -20, 20);
	rect rc = { 0,0,480,50 };
	//canvas_draw_text_aligned(c, font, 0xff1122, "hello world hello world hello world hello world hello world", &rc, TEXT_ALIGN_CENTER| TEXT_VALIGN_CENTER);
	//canvas_draw_text_multiline(c, font, 0xff1122, "hello world hello world hello world hello world", &rc, 0);
	canvas_draw_text_multiline(c, font, 0, "hello\tworld\nhello world\r\nhello world", &rc, TEXT_FLAG_ELLIPSIS);

	
	int ret = canvas_draw_text(c, font, 0xff1122, NULL,30, 160);
	rect rc0 = { 25, 90, 725, 790 };
	ret = canvas_draw_image_scaled(c, img, &rc0);

	rect rc1 = {36, 988, 200, 1032};
	ret = canvas_draw_text_aligned(c, font, 0xffffff, "12345", &rc1, TEXT_ALIGN_CENTER | TEXT_VALIGN_CENTER);
	rect rc2 = { 250, 988, 410, 1032 };
	ret = canvas_draw_text_aligned(c, font, 0xffffff, "67890", &rc2, TEXT_ALIGN_CENTER | TEXT_VALIGN_CENTER);
	rect rc3 = { 25, 90, 725, 160 };
	ret = canvas_fill_rect(c, 0xc0ffffff, &rc3);
	ret = image_save_to_file(back, IMAGE_ENCODE_JPG, "out.jpg");
	assert(ret ==  0);
	printf("return %d \n", ret);
}


void test_invalid_file() {
	int buf = 0;

	//²âÊÔ´íÎóÊý¾Ý¼ÓÔØfont_family
	size_t ttflen = 0;
	void* ttfdata = readFileContent("msyh.ttf", &ttflen);
	ttflen -= 200000;

	font_family_t family1 = font_family_load_from_file("1.jpg");
	font_family_t family2 = font_family_load_from_file("not exist.ttf");
	font_family_t family3 = font_family_load(&buf, 4);
	font_family_t family4 = font_family_load(NULL, 4);
	font_family_t family5 = font_family_load(ttfdata, ttflen);

	//²âÊÔ´íÎóÊý¾Ý¼ÓÔØimage
	size_t imglen = 0;
	void* imgdata = readFileContent("1.jpg", &imglen);
	imglen -= 200;

	image_t img1 = image_load_from_file("msyh.ttf");
	image_t img2 = image_load_from_file("not exist.ttf");
	image_t img3 = image_load(&buf, 4);
	image_t img4 = image_load(NULL, 4);
	image_t img5 = image_load(imgdata, imglen);


	int a = 0;
}

void dumy_save(void*,void*,int) {

}


void encode_test() {
	image_t img = image_load_from_file("E:\\gmtest\\r.jpg");
	if (!img)return;
	uint64_t t0 = clock_get_tick();
	for (int i = 0; i < 10; i++) {
		image_save(img, IMAGE_ENCODE_JPG, dumy_save, NULL);
	}
	uint64_t t1 = clock_get_tick();

	image_save_to_file(img, IMAGE_ENCODE_JPG, "E:\\gmtest\\ro.jpg");
	printf("encode used %d\n" ,(int) (t1-t0));
}

void scale_test() {
	image_t img = image_load_from_file("E:\\gmtest\\r.jpg");
	if (!img)return;
	int len = 0;
	void* inpixel = image_lock_pixel(img, &len);
	int w = image_width(img);
	int h = image_height(img);
	int stride = w * image_type(img);
	int outw = 700;
	int outh = 700;
	int outstride = outw * 3;
	void* buf = malloc(outstride * outh);
	uint64_t t0 = clock_get_tick();
	for (int i = 0; i < 10; i++) {
		//image_save(img, IMAGE_ENCODE_JPG, dumy_save, NULL);
		
		//libyuv::ScalePlane((uint8*)inpixel, stride, w, h, (uint8*)buf, outstride, outw, outh, kFilterLinear);
	}
	uint64_t t1 = clock_get_tick();
	image_t newimg = image_create(buf, outw, outh, 3);
	image_save_to_file(newimg, IMAGE_ENCODE_JPG, "E:\\gmtest\\ro.jpg");
	printf("encode used %d\n", (int)(t1 - t0));
}


int main(int argc, char* arg[]) {
	const char* cmd = "";
	const char* file = "";
	int count = 10;
	if (argc > 3) {
		cmd = arg[1];
		file = arg[2];
		count = atoi(arg[3]);
	}
	else {		
		printf("usage xgraphics <load> <file> <count>\n");
		return 1;
	}
	printf("do %s %d times ...\n",cmd, count);
	if (strcmp(cmd, "load") == 0) {
		load(count, file);
	}else if (strcmp(cmd, "draw") == 0) {
		draw(count, file);
	}
	else if (strcmp(cmd, "scale") == 0) {
		scale(count, file);
	}
	else if (strcmp(cmd, "font") == 0) {
		font(count, file);
	}
	else if (strcmp(cmd, "test") == 0) {
		//encode_test();
		test_font();
		//test_invalid_file();
		//scale_test();
	}
	else {		
		printf("not support command\n");
	}
	//getchar();
	return 0;

}
