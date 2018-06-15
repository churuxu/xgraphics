#pragma once

#include <string.h>

/*
常用的像素操作函数
*/

typedef void(*blend_func)(void* dst, int dstlinewidth, int x, int y, int w, int h, void* src, int srclinewidth, int srcx, int srcy, int color);


#define BLEND_FUNC_COPY_PIXEL(name, comp) \
static void name(void* dst, int dstlinewidth, int x, int y, int w, int h, void* src, int srclinewidth, int srcx, int srcy, int color) {\
	int i;\
	unsigned char* pdst = (unsigned char*)dst;\
	unsigned char* psrc = (unsigned char*)src;\
	int perline = w * comp;\
	pdst += (y * dstlinewidth + x * comp);\
	psrc += (srcy * srclinewidth + srcx * comp);\
	for (i = 0; i < h; ++i) {\
		memcpy(pdst, psrc, perline);\
		pdst += dstlinewidth;\
		psrc += srclinewidth;\
	}\
}\


#define BLEND_FUNC_OP_PIXEL(name, dstcomp, srccomp, op) \
static void name(void* dstdata, int dstlinewidth, int x, int y, int w, int h, void* srcdata, int srclinewidth, int srcx, int srcy, int color) {\
	int i,j;\
	unsigned char* pdst = (unsigned char*)dstdata;\
	unsigned char* psrc = (unsigned char*)srcdata;\
    int dstinc = dstlinewidth - w * dstcomp;\
	int srcinc = srclinewidth - w * srccomp;\
	pdst += (y * dstlinewidth + x * dstcomp);\
	psrc += (srcy * srclinewidth + srcx * srccomp);\
	for (i = 0; i < h; ++i) {\
		for(j = 0; j < w; ++j){\
			op(pdst,psrc,color);\
			pdst += dstcomp;\
			psrc += srccomp;\
		}\
		pdst += dstinc;\
		psrc += srcinc;\
	}\
}\



#define GET_COLOR_A(color) ((color>>24)&0xff)
#define GET_COLOR_R(color) ((color>>16)&0xff)
#define GET_COLOR_G(color) ((color>>8)&0xff)
#define GET_COLOR_B(color) ((color)&0xff)

#define BLEND_ALPHA(pdst, r, g, b, a) pdst[0] = ((0xff - a) * pdst[0] + a * r) / 0xff;pdst[1] = ((0xff - a) * pdst[1] + a *g) / 0xff;;pdst[2] = ((0xff - a) * pdst[2] + a * b) / 0xff;



//设置为源rgb
#define BLEND_OP_RGB_RGB(pdst, psrc, color) pdst[0] = psrc[0];pdst[1] = psrc[1];pdst[2] = psrc[2];

//设置为源rgba
#define BLEND_OP_RGBA_RGBA(pdst, psrc, color) (*(int*)(pdst)) = (*(int*)(psrc));

//设置为源rgb
#define BLEND_OP_RGBA_RGB(pdst, psrc, color) pdst[0] = psrc[0];pdst[1] = psrc[1];pdst[2] = psrc[2];pdst[3]=0xff;

//和源rgba alpha混合
#define BLEND_OP_RGB_RGBA(pdst, psrc, color) BLEND_ALPHA(pdst, psrc[0], psrc[1],  psrc[2], psrc[3])


//使用指定颜色的rgb，灰度源作为alpha， 进行alpha混合
#define BLEND_OP_RGB_GRAY_COLOR(pdst, psrc, color) BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), psrc[0])

//使用指定颜色的rgb，灰度源作为alpha， 进行alpha混合
#define BLEND_OP_RGBA_GRAY_COLOR(pdst, psrc, color) BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), psrc[0]); pdst[3]=0xff;

//使用指定颜色的rgba，和灰度源alpha， 进行alpha混合
#define BLEND_OP_RGB_GRAY_ALPHA_COLOR(pdst, psrc, color) BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), (GET_COLOR_A(color) * psrc[0] / 256))

//使用指定颜色的rgba，和灰度源alpha， 进行alpha混合
#define BLEND_OP_RGBA_GRAY_ALPHA_COLOR(pdst, psrc, color) BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), (GET_COLOR_A(color) * psrc[0] / 256) ); pdst[3]=0xff;


//设置为指定颜色
#define BLEND_OP_RGB_COLOR(pdst, psrc, color)  pdst[0] = GET_COLOR_R(color);pdst[1] = GET_COLOR_G(color);pdst[2] = GET_COLOR_B(color)

//设置为指定颜色
#define BLEND_OP_RGBA_COLOR(pdst, psrc, color)  (*(int*)(pdst)) = color;pdst[3]=0xff;

//和指定颜色进行alpha混合
#define BLEND_OP_RGB_ALPHA_COLOR(pdst, psrc, color)  BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), GET_COLOR_A(color))

//和指定颜色进行alpha混合
#define BLEND_OP_RGBA_ALPHA_COLOR(pdst, psrc, color)  BLEND_ALPHA(pdst, GET_COLOR_R(color),GET_COLOR_G(color), GET_COLOR_B(color), GET_COLOR_A(color));


//用于画图
BLEND_FUNC_COPY_PIXEL(blend_rgb_rgb, 3);
BLEND_FUNC_COPY_PIXEL(blend_rgba_rgba, 4);
BLEND_FUNC_OP_PIXEL(blend_rgb_rgba, 3, 4, BLEND_OP_RGB_RGBA);
BLEND_FUNC_OP_PIXEL(blend_rgba_rgb, 4, 3, BLEND_OP_RGBA_RGB);
//用于画字
BLEND_FUNC_OP_PIXEL(blend_rgb_gray_color, 3, 1, BLEND_OP_RGB_GRAY_COLOR); 
BLEND_FUNC_OP_PIXEL(blend_rgba_gray_color, 4, 1, BLEND_OP_RGBA_GRAY_COLOR); 
BLEND_FUNC_OP_PIXEL(blend_rgb_gray_alpha_color, 3, 1, BLEND_OP_RGB_GRAY_ALPHA_COLOR);
BLEND_FUNC_OP_PIXEL(blend_rgba_gray_alpha_color, 4, 1, BLEND_OP_RGBA_GRAY_ALPHA_COLOR);
//用于画形状
BLEND_FUNC_OP_PIXEL(blend_rgb_color, 3, 0, BLEND_OP_RGB_GRAY_COLOR); 
BLEND_FUNC_OP_PIXEL(blend_rgba_color, 4, 0, BLEND_OP_RGBA_GRAY_COLOR); 
BLEND_FUNC_OP_PIXEL(blend_rgb_alpha_color, 3, 0, BLEND_OP_RGB_GRAY_ALPHA_COLOR);
BLEND_FUNC_OP_PIXEL(blend_rgba_alpha_color, 4, 0, BLEND_OP_RGBA_GRAY_ALPHA_COLOR);

