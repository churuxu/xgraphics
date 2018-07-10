#include "xgraphics.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "memory_helper.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"


#include "memory_tlspool.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"



#include "blend_helper.h"

#ifdef HAVE_LIBJPEG
#include "libjpeg_helper.h"
#endif


struct _image {
	int width;
	int height;
	int comp;	
	void* pixel;
};

struct _font_family {
	stbtt_fontinfo info;
	void* buf;	
};

struct _font {
	font_family_t family;	
	int ascent;  //基线上面像素高度
	int descent; //基线下面像素高度（负数）
	int linegap;
	float scale; //缩放大小
	int size;
	int style;
	int pixelsize; //单个字型图片buffer size
};

struct _canvas {
	image_t image;
};


static void* load_file_content(const char* name, size_t* plen) {
	void* ret = NULL;
	FILE* f = fopen(name, "rb");
	if (!f)return NULL;
	fseek(f, 0, SEEK_END);
	long sz = ftell(f);	
	if (sz) {
		fseek(f, 0, SEEK_SET);
		void* data = malloc(sz);
		if (data) {
			int len = (int)fread(data, 1, sz, f);
			if (len > 0) {
				ret = data;
				if (plen)*plen = len;
			}
			else {
				free(data);
			}
		}
	}
	fclose(f);	
	return ret;
}


image_t image_load(void* data, int len) {
	if (!data)return NULL;
	image_t img = (image_t)malloc(sizeof(struct _image));
	if (img) {

#ifdef HAVE_LIBJPEG
		//尝试使用libjpeg解析
		if (0 == libjpeg_test_format(data, len)) { //是jpg文件格式
			if (0 == libjpeg_decode(data, len, &img->pixel, &img->width, &img->height)) {
				img->comp = 3;
				return img;
			}
			else {

			}
		}
#endif	

		img->pixel = stbi_load_from_memory((stbi_uc*)data, len, &img->width, &img->height, &img->comp, 0);
		if (!img->pixel) {
			free(img);
			return NULL;
		}
		if (img->comp != 3 && img->comp != 4) {
			stbi_image_free(img->pixel);
			free(img);
			return NULL;
		}
	}
	return img;
}

image_t image_load_from_file(const char* name) {
	size_t len = 0;
	void* data = load_file_content(name,&len);
	if (data) {
		image_t img = image_load(data, (int)len);
		free(data);
		return img;
	}	
	return NULL;
}

#ifndef JPG_QUALITY
#define JPG_QUALITY  50
#endif

int image_save(image_t img, int encode, image_write_func func, void* userdata) {
	int internalret = 0;
	switch (encode) {
	case IMAGE_ENCODE_PNG:
		internalret = stbi_write_png_to_func(func, userdata, img->width, img->height, img->comp, img->pixel, img->width*img->comp);
		break;
	case IMAGE_ENCODE_JPG:

#ifdef HAVE_LIBJPEG
	{
		void* data = NULL;
		int len = 0;
		internalret = libjpeg_encode(&data, &len, img->pixel, img->width, img->height, JPG_QUALITY);
		func(userdata, data, len);
		free(data);
		return internalret;
	}		
#else
		internalret = stbi_write_jpg_to_func(func, userdata, img->width, img->height, img->comp, img->pixel, JPG_QUALITY);
#endif		
		break;
	case IMAGE_ENCODE_BMP:
		internalret = stbi_write_bmp_to_func(func, userdata, img->width, img->height, img->comp, img->pixel);
		break;
	default:
		return EINVAL;
	};
	if (!internalret) {
		return ENOMEM;
	}
	return 0;
}

static void write_file_callback(void* ctx, void* data, int len) {
	FILE* f = (FILE*)ctx;
	fwrite(data, 1, len, f);
}

int image_save_to_file(image_t img, int encode, const char* name) {
	FILE* f = fopen(name, "wb");
	if (!f)return errno;
	int ret = image_save(img, encode, write_file_callback, f);
	fclose(f);
	return ret;	
}


image_t image_alloc(int width, int height, int type) {
	if (type != 3 && type != 4)return NULL;
	if (width <= 0 || height <= 0)return NULL;
	int sz = width * height * type;
	image_t img = (image_t)malloc(sizeof(struct _image));
	if (img) {
		img->pixel = malloc(sz);
		if (!img->pixel) {
			free(img);
			return NULL;
		}
		memset(img->pixel, 0xff, sz);
		img->width = width;
		img->height = height;
		img->comp = type;
	}
	return img;
}

image_t image_create(void* pixel, int width, int height, int type) {
	if (type != 3 && type != 4)return NULL;
	if (width <= 0 || height <= 0)return NULL;
	int sz = width * height * type;
	image_t img = (image_t)malloc(sizeof(struct _image));
	if (img) {
		img->pixel = malloc(sz);
		if (!img->pixel) {
			free(img);
			return NULL;
		}
		memcpy(img->pixel, pixel, sz);
		img->width = width;
		img->height = height;
		img->comp = type;
	}
	return img;
}


void image_free(image_t img) {	
	stbi_image_free(img->pixel);
	free(img);	
}

image_t image_copy(image_t img) {	
	int pixellen = img->width * img->height * img->comp;
	image_t newimg = (image_t)malloc(sizeof(struct _image));
	if (!newimg)return NULL;
	void* newpixel = malloc(pixellen);
	if (!newpixel) { free(newimg); return NULL; }	
	memcpy(newpixel, img->pixel, pixellen);	
	newimg->pixel = newpixel;
	newimg->width = img->width;
	newimg->height = img->height;
	newimg->comp = img->comp;
	return newimg;
}

image_t image_scaled(image_t img, int width, int height) {
	int pixellen = width * height * img->comp;
	int stride = width* img->comp;
	void* newpixel = malloc(pixellen);
	if (!newpixel) { return NULL; }
	int ret = stbir_resize_uint8((unsigned char*)img->pixel, img->width, img->height, img->width * img->comp,
		(unsigned char*)newpixel, width, height, stride, img->comp);
	if (!ret) {
		free(newpixel);
		return NULL;
	}
	image_t newimg = (image_t)malloc(sizeof(struct _image));
	if (!newimg) {
		free(newpixel);
		return NULL;
	}
	newimg->comp = img->comp;
	newimg->width = width;
	newimg->height = height;
	newimg->pixel = newpixel;
	return newimg;
}

image_t image_scaled_partial(image_t img, int width, int height, const rect* rc) {	
	int pixellen = width * height * img->comp;
	int stride = width* img->comp;
	void* newpixel = malloc(pixellen);
	if (!newpixel) { return NULL; }
	int ret = stbir_resize_region((unsigned char*)img->pixel, img->width, img->height, img->width * img->comp,
		(unsigned char*)newpixel, width, height, stride, STBIR_TYPE_UINT8, img->comp, img->comp==4?1:0, 0,
		STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP, STBIR_FILTER_DEFAULT, STBIR_FILTER_DEFAULT, STBIR_COLORSPACE_LINEAR, NULL,
		(float)rc->left / (float)img->width, (float)rc->top / (float)img->height, (float)rc->right / (float)img->width, (float)rc->bottom / (float)img->height);
	if (!ret) {
		free(newpixel);
		return NULL;
	}
	image_t newimg = (image_t)malloc(sizeof(struct _image));
	if (!newimg) {
		free(newpixel);
		return NULL; 
	}
	newimg->comp = img->comp;
	newimg->width = width;
	newimg->height = height;
	newimg->pixel = newpixel;
	return newimg;
}


int image_width(image_t img) {
	return img->width;
}

int image_height(image_t img) {
	return img->height;
}

int image_type(image_t img) {
	return img->comp;
}

void* image_lock_pixel(image_t img, int* len) {
	if (len)*len = img->width * img->height * img->comp;
	return img->pixel;
}

void image_unlock_pixel(image_t img, void* pixel) {

}

font_family_t font_family_get_default() {
	return NULL;
}

font_family_t font_family_load(void* data, int len) {
	if (!data)return NULL;
	font_family_t ret = (font_family_t)malloc(sizeof(struct _font_family));
	if (!ret)return NULL;
	void* buf = malloc(len);
	if (!buf) { free(ret); return NULL; }
	int ok = 0;
	int off = stbtt_GetFontOffsetForIndex((unsigned char*)data, 0);
	if (off>=0) {
		ok = stbtt_InitFont(&ret->info, (unsigned char*)data, stbtt_GetFontOffsetForIndex((unsigned char*)data, 0));
	}
	if (!ok) {
		free(ret);
		free(buf);
		return NULL;
	}
	memcpy(buf, data, len);
	ret->buf = buf;
	return ret;
}

font_family_t font_family_load_from_file(const char* file) {
	font_family_t ret = (font_family_t)malloc(sizeof(struct _font_family));
	if (!ret)return NULL;
	size_t len = 0;
	void* data = load_file_content(file , &len);
	if (data) {
		int ok = 0;
		int off = stbtt_GetFontOffsetForIndex((unsigned char*)data, 0);
		if (off >= 0) {
			ok = stbtt_InitFont(&ret->info, (unsigned char*)data, off);
		}		
		if (!ok) {
			free(ret);
			free(data);
			return NULL;
		}
		ret->buf = data;
	}
	else {
		free(ret);
		return NULL;
	}
	return ret;
}


void font_family_free(font_family_t f) {
	free(f->buf);
	free(f);
}



font_t font_create(font_family_t f, int size, int style) {
	font_t ret = (font_t)malloc(sizeof(struct _font));
	if (!ret)return NULL;
	int l = 0, t = 0, r = 0, b = 0;
	stbtt_GetFontBoundingBox(&f->info, &l, &t, &r, &b);
	ret->scale = stbtt_ScaleForPixelHeight(&f->info, (float)size);
	stbtt_GetFontVMetrics(&f->info, &ret->ascent, &ret->descent, &ret->linegap);
	ret->family = f;
	ret->size = size; 
	ret->style = style;
	ret->ascent = (int)(ret->scale * ret->ascent);
	ret->descent = (int)(ret->scale * ret->descent);
	ret->linegap = (int)(ret->scale * ret->linegap);
	ret->pixelsize = (int)((r - l)*(b - t)*(ret->scale)) + 4;	
	return ret;
}

void font_free(font_t f) {	
	free(f);
}

canvas_t canvas_create(image_t img) {
	canvas_t ret = (canvas_t)malloc(sizeof(struct _canvas));
	if (!ret)return NULL;
	ret->image = img;	
	return ret;
}

void canvas_free(canvas_t f) {
	free(f);
}

//根据不同图像类型获取绘制函数
static blend_func get_blend_func(image_t dst, image_t src) {
	blend_func render = NULL;
	if (dst->comp == 4) {
		if (src->comp == 4) {
			render = blend_rgba_rgba;
		}
		else if (src->comp == 3) {
			render = blend_rgba_rgb;
		}
	}
	else if (dst->comp == 3) {
		if (src->comp == 4) {
			render = blend_rgb_rgba;
		}
		else if (src->comp == 3) {
			render = blend_rgb_rgb;
		}
	}
	return render;
}

//获取绘制区域, 有返回1，没有返回0  参数：最大宽，最大高，目前绘制区域，得到区域
static int get_draw_rect(int maxw, int maxh, const rect* dst, rect* out, int* srcx, int* srcy) {
	out->left = (dst->left < 0) ? 0 : dst->left;
	out->top = (dst->top < 0) ? 0 : dst->top;
	out->right = (dst->right > maxw) ? maxw : dst->right;
	out->bottom = (dst->bottom > maxh) ? maxh : dst->bottom;
	if (out->right <= out->left || out->bottom <= out->top)return 0;
	if (dst->left < 0) {
		*srcx = (*srcx) - dst->left;
	}
	if (dst->top < 0) {
		*srcy = (*srcy) - dst->top;
	}
	return 1;
}


int canvas_draw_image_partial(canvas_t c, image_t img, int x, int y, const rect* src) {
	blend_func render = get_blend_func(c->image, img);
	if (!render)return EINVAL;
	rect rc;
	rect dst;
	int srcx = src->left;
	int srcy = src->top;
	dst.left = x;
	dst.top = y;
	dst.right = x + src->right - src->left;
	dst.bottom = y + src->bottom - src->top;
	if (!get_draw_rect(c->image->width, c->image->height, &dst, &rc, &srcx, &srcy))return 0;
	render(c->image->pixel, c->image->width * c->image->comp, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, img->pixel, img->width*img->comp, srcx, srcy, 0);
	return 0;
}

int canvas_draw_image(canvas_t c, image_t img, int x, int y) {
	rect rc;
	rc.left = 0;
	rc.top = 0;
	rc.right = img->width;
	rc.bottom = img->height;
	return canvas_draw_image_partial(c, img, x, y, &rc);
}

int canvas_draw_image_scaled(canvas_t c, image_t img, const rect* dst) {
	int ret = 0;
	if ((dst->right - dst->left) != img->width || (dst->bottom - dst->top) != img->height) {
		//大小不一样则先缩放，再绘制
		image_t newimg = image_scaled(img, dst->right - dst->left, dst->bottom - dst->top);
		if (newimg) {
			ret = canvas_draw_image(c, newimg, dst->left, dst->top);
			image_free(newimg);
		}
		else {
			ret = ENOMEM;
		}
	}
	else {
		//大小一样直接绘制
		ret = canvas_draw_image(c, img, dst->left, dst->top);
	}
	return ret;
}

int canvas_draw_image_scaled_partial(canvas_t c, image_t img, const rect* dst, const rect* src) {
	int ret = 0;
	if ((dst->right - dst->left) != (src->right - src->left) || (dst->bottom - dst->top) != (src->bottom - src->top)) {
		//大小不一样则先缩放，再绘制
		image_t newimg = image_scaled_partial(img, dst->right - dst->left, dst->bottom - dst->top, src);
		if (newimg) {
			ret = canvas_draw_image(c, newimg, dst->left, dst->top);
			image_free(newimg);
		}
		else {
			ret = ENOMEM;
		}
	}
	else {
		//大小一样直接绘制 
		ret = canvas_draw_image_partial(c, img, dst->left, dst->top, src);
	}
	return ret;
}


//utf8字符串 获取下一个unicode字符, 结束或失败返回0
static uint32_t get_unicode_char(const char* str, const char** pnext) {
	if (!str)return 0;
	unsigned char* s = (unsigned char*)str;
	unsigned char c = s[0];
	int next = 0;
	uint32_t ret = 0;
	if (c < 0x80) {	
		ret = (int)c;
		next = 1;		
	}
	else if (c < 0xc2) {
		//invalid
		return 0;
	}
	else if (c < 0xe0) {
		if (!((s[1] ^ 0x80) < 0x40)) {
			//invalid
			return 0;
		}			
		ret = ((uint32_t)(c & 0x1f) << 6)
			| (uint32_t)(s[1] ^ 0x80);
		next = 2;
	}
	else if (c < 0xf0) {		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (c >= 0xe1 || s[1] >= 0xa0))) {
			//invalid
			return 0;
		}
		ret = ((uint32_t)(c & 0x0f) << 12)
			| ((uint32_t)(s[1] ^ 0x80) << 6)
			| (uint32_t)(s[2] ^ 0x80);
		next = 3;
	}
	else if (c < 0xf8) {		
		if (!((s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
			&& (s[3] ^ 0x80) < 0x40
			&& (c >= 0xf1 || s[1] >= 0x90))) {
			//invalid
			return 0;
		}		
		ret = ((uint32_t)(c & 0x07) << 18)
			| ((uint32_t)(s[1] ^ 0x80) << 12)
			| ((uint32_t)(s[2] ^ 0x80) << 6)
			| (uint32_t)(s[3] ^ 0x80);
		next = 4;
	}
	else {
		//invalid
		return 0;
	}
	if (pnext)*pnext = str + next;
	return ret;
}

//获取画字所需的blend函数
static blend_func get_text_blend_func(image_t img, color_t color) {
	color_t a = color & 0xff000000;
	if (a && a != 0xff000000) {  //指定颜色带alpha
		if (img->comp == IMAGE_TYPE_RGB) {
			return blend_rgb_gray_alpha_color;
		}
		else if (img->comp == IMAGE_TYPE_RGBA) {
			return blend_rgba_gray_alpha_color;
		}
	}
	else {   //指定颜色不带alpha
		if (img->comp == IMAGE_TYPE_RGB) {
			return blend_rgb_gray_color;
		}
		else if (img->comp == IMAGE_TYPE_RGBA) {
			return blend_rgba_gray_color;
		}
	}
	return NULL;
}


//获取画颜色所需blend函数
static blend_func get_color_blend_func(image_t img, color_t color) {
	color_t a = color & 0xff000000;
	if (a && a != 0xff000000) {  //指定颜色带alpha
		if (img->comp == IMAGE_TYPE_RGB) {			
			return blend_rgb_alpha_color;
		}
		else if (img->comp == IMAGE_TYPE_RGBA) {			
			return blend_rgba_alpha_color;
		}
	}
	else {   //指定颜色不带alpha
		if (img->comp == IMAGE_TYPE_RGB) {			
			return blend_rgb_color;
		}
		else if (img->comp == IMAGE_TYPE_RGBA) {
			
			return blend_rgba_color;
		}
	}
	return NULL;
}



typedef struct _glyph {
	font_t font;
	int advance; //x跨度
	int bearing; //x偏移
	int width;
	int height;
	int left; //
	int top;
	int right;
	int bottom;
	unsigned char* pixel;
}glyph;

//为字形预分配内存, 成功返回0 失败返回ENOMEM
static int glyph_prepare(font_t font, glyph* out) {
	out->font = font;
	out->pixel = (unsigned char*)malloc(font->pixelsize);
	if (!out->pixel)return ENOMEM;
	return 0;
}

//释放字形的内存
static void glyph_release(glyph* g) {
	free(g->pixel);
}

//获取一个字型，存在返回1，不存在返回0，存在时可用字形pixel，不存在时只可用advance
static int get_glyph(font_t font, int codepoint, glyph* g) {
	int ret = 0;
	stbtt_fontinfo* info = &(font->family->info);
	if (codepoint == '\r' || codepoint == '\n') {
		g->advance = 0;		 
	}
	else if (codepoint == '\t') {
		g->advance = font->size * 2;
	}
	else {
		int index = stbtt_FindGlyphIndex(info, codepoint);
		stbtt_GetGlyphHMetrics(info, index, &g->advance, &g->bearing);
		stbtt_GetGlyphBitmapBox(info, index, font->scale, font->scale, &g->left, &g->top, &g->right, &g->bottom);
		g->advance = (int)(g->advance * font->scale);
		//g->bearing = (int)(g->bearing * font->scale); //目前未使用

		if (g->right - g->left == 0)return 0;
		g->width = g->right - g->left;
		g->height = g->bottom - g->top;
		//printf("adv:%d width:%d\n", g->advance, g->width);
		stbtt_MakeGlyphBitmap(info, g->pixel, g->width, g->height, g->width, font->scale, font->scale, index);
		ret = 1;
	}
	return ret;
}

//绘制一个字形
static int render_glyph(image_t dst, int dstlinew, blend_func render, glyph* g, color_t color, int x, int y, const rect* allowable) {
	rect rc;
	int srcx = 0;
	int srcy = 0;
	if (allowable) {
		int minx = allowable->left;
		int maxx = allowable->right;
		if ((x < minx) || ((x + g->width) > maxx))return 0;
	}
	rect dstrc;
	dstrc.left = x + g->left;
	dstrc.top = y + g->font->ascent + g->top;
	dstrc.right = dstrc.left + g->width;
	dstrc.bottom = dstrc.top + g->height;	
	if (get_draw_rect(dst->width, dst->height, &dstrc, &rc, &srcx, &srcy)) {
		render(dst->pixel, dstlinew, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, g->pixel, g->width, srcx, srcy, color);		
		return 1;
	}
	return 0;
}

//绘制下滑线或删除线
static void font_draw_line( image_t dst, int dstlinew, font_t font, blend_func render, color_t color, int fromx, int tox, int y) {
	rect rc;
	int srcx = 0;
	int srcy = 0;
	rect dstrc;
	dstrc.left = fromx;	
	dstrc.right = tox;
	int linew = 1 + font->size / 20;
	if (font->style & FONT_STYLE_UNDERLINE) {
		dstrc.top = y + font->ascent + 1;
		dstrc.bottom = dstrc.top + linew;
		if (get_draw_rect(dst->width, dst->height, &dstrc, &rc, &srcx, &srcy)) {
			render(dst->pixel, dstlinew, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, dst->pixel, dst->width, srcx, srcy, color);
		}
	}
	if (font->style & FONT_STYLE_STRIKEOUT) {
		dstrc.top = y + font->ascent / 2 - font->descent;
		dstrc.bottom = dstrc.top + linew;
		if (get_draw_rect(dst->width, dst->height, &dstrc, &rc, &srcx, &srcy)) {
			render(dst->pixel, dstlinew, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, dst->pixel, dst->width, srcx, srcy, color);
		}
	}
}

#define VSPACING 0 //默认行间距 (目前未实现)
#define HSPACING 1 //默认字间距 

int canvas_measure_text_width(canvas_t c, font_t font, const char* str) {
	uint32_t ch = 0;
	int ret = 0;
	int adv;
	int bear;
	const char* pstr = str;
	stbtt_fontinfo* info = &(font->family->info);	
	ch = get_unicode_char(pstr, &pstr);
	while (ch) {
		stbtt_GetCodepointHMetrics(info, ch, &adv, &bear);
		ret += (adv + HSPACING);
		ch = get_unicode_char(pstr, &pstr);
	}
	return (int)(ret * font->scale);
}


//在区域内画字符串
static int canvas_draw_text_in_rect(canvas_t c, font_t font, color_t color, const char* str, int x, int y, const rect* rc) {
	uint32_t ch = 0;
	int firstx = x;
	glyph g;
	const char* pstr = str;
	stbtt_fontinfo* info = &(font->family->info);
	blend_func linerender = get_color_blend_func(c->image, color);
	blend_func render = get_text_blend_func(c->image, color);
	if (!render)return EINVAL;
	int dstw = c->image->width * c->image->comp;
	if (glyph_prepare(font, &g))return ENOMEM;
	ch = get_unicode_char(pstr, &pstr);
	while (ch) {
		if (get_glyph(font, ch, &g)) { //获取到字型			
			render_glyph(c->image, dstw, render, &g, color, x, y, rc);			
		}

		x += (g.advance + HSPACING);
		ch = get_unicode_char(pstr, &pstr);
	}
	glyph_release(&g);
	if (font->style) {
		font_draw_line(c->image, dstw, font, linerender, color, firstx, x, y);
	}
	return 0;
}


//画字符串
int canvas_draw_text(canvas_t c, font_t font, color_t color, const char* str, int x, int y) {	
	return canvas_draw_text_in_rect(c, font, color, str, x, y, NULL);
}

//画字符串（居中|靠右等）
int canvas_draw_text_aligned(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int align) {
	int w = canvas_measure_text_width(c, font, str);
	int h = font->size;
	//if (dst->right - dst->left < w || dst->bottom - dst->top < h)return EINVAL; //范围太小
	int x, y;
	int halign = align & 0xf;
	int valign = (align & 0xf0) >> 4;
	//水平x
	if (halign == 1) {
		x = dst->right - w;
	}
	else if(halign == 2) {
		x = dst->left + (dst->right - dst->left - w) / 2;
	}
	else {
		x = dst->left;
	}
	//垂直y
	if (valign == 1) {
		y = dst->bottom - h;
	}
	else if (valign == 2) {
		y = dst->top + (dst->bottom - dst->top - h) / 2;
	}
	else {
		y = dst->top;
	}
	return canvas_draw_text_in_rect(c, font, color, str, x, y, dst);
}


//画字符串 (多行)
int canvas_draw_text_multiline(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int flag) {
	uint32_t ch;
	int lastline;
	int changeline = 0;
	int x = dst->left;
	int y = dst->top;
	int nextx;
	int hasglyph;
	int ellipsislen = 0;
	glyph g;
	const char* pstr = str;
	stbtt_fontinfo* info = &(font->family->info);
	blend_func linerender = get_color_blend_func(c->image, color);
	blend_func render = get_text_blend_func(c->image, color);
	if (!render)return EINVAL;
	int dstw = c->image->width * c->image->comp;
	if(glyph_prepare(font, &g))return ENOMEM;
	lastline = ((dst->bottom - y) < (font->size * 2));
	if (flag & TEXT_FLAG_ELLIPSIS) {
		ellipsislen = canvas_measure_text_width(c, font, "...");
	}
	ch = get_unicode_char(pstr, &pstr);
	while (ch) {
		if (ch == '\n') { //换行符换行
			changeline = 1;
			hasglyph = 0;
			g.advance = 0;
		}
		else {
			hasglyph = get_glyph(font, ch, &g);

			nextx = x + g.advance + HSPACING;
			if (lastline && ellipsislen) { //最后一行，计算超出，"..."宽度
				if (nextx > (dst->right - ellipsislen)) {
					//超出, 绘制3个点		
					hasglyph = get_glyph(font, '.', &g);
					if (hasglyph) {
						render_glyph(c->image, dstw, render, &g, color, x, y, NULL);
						x += g.advance + HSPACING;
						render_glyph(c->image, dstw, render, &g, color, x, y, NULL);
						x += g.advance + HSPACING;
						render_glyph(c->image, dstw, render, &g, color, x, y, NULL);
					}
					break;
				}
			}
			else { //非最后一行
				if (nextx > dst->right) { //超出换行
					changeline = 1;
				}
			}
		}


		if (changeline) { //换行处理
			changeline = 0;	
			if (y + font->size > (dst->bottom - font->size)) {
				nextx = x;
				break; 
			}
			if (font->style) {
				font_draw_line(c->image, dstw, font, linerender, color, dst->left, x, y);
			}
			y += font->size;
			x = dst->left;
			nextx = x + g.advance + HSPACING;
			lastline = ((dst->bottom - y) < (font->size * 2));			
		}

		if (hasglyph) { //字形存在则绘制
			render_glyph(c->image, dstw, render, &g, color, x, y, NULL);
		}

		x = nextx;
		ch = get_unicode_char(pstr, &pstr);
	}
	glyph_release(&g);
	if (font->style) {
		font_draw_line(c->image, dstw, font, linerender, color, dst->left, nextx, y);
	}
	return 0;
}



//填充矩形
int canvas_fill_rect(canvas_t c, color_t color, const rect* rcs) {
	blend_func render = get_color_blend_func(c->image, color);	
	if (!render)return EINVAL;
	image_t dst = c->image;
	int dstlinew = dst->width * dst->comp;
	rect rc;
	int srcx = 0;
	int srcy = 0;
	if (get_draw_rect(dst->width, dst->height, rcs, &rc, &srcx, &srcy)) {
		render(dst->pixel, dstlinew, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, dst->pixel, dst->width, srcx, srcy, color);
	}
	return 0;
}


