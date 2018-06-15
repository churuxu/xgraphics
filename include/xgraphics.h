#pragma once



/*
常规说明：
操作函数返回int的，0表示成功，其它表示errno错误码

*/


//图像类型
#define IMAGE_TYPE_RGB   3
#define IMAGE_TYPE_RGBA  4

//图像编码类型
#define IMAGE_ENCODE_BMP 1
#define IMAGE_ENCODE_PNG 2
#define IMAGE_ENCODE_JPG 3

//字体样式
//#define FONT_STYLE_BOLD      1 //加粗 (目前未实现)
//#define FONT_STYLE_ITALIC    2 //斜体 (目前未实现)
#define FONT_STYLE_UNDERLINE 4 //下滑线
#define FONT_STYLE_STRIKEOUT 8 //删除线

//文本位置
#define TEXT_ALIGN_LEFT    0
#define TEXT_ALIGN_RIGHT   1
#define TEXT_ALIGN_CENTER  2
#define TEXT_VALIGN_TOP    0x00
#define TEXT_VALIGN_BOTTOM 0x10
#define TEXT_VALIGN_CENTER 0x20

//绘制文本选项
#define TEXT_FLAG_ELLIPSIS 0x0100  //超出显示省略号


typedef struct _image* image_t; //一个图像

typedef struct _font_family* font_family_t; //指定：宋体

typedef struct _font* font_t;  //指定：宋体 大小26 斜体

typedef struct _canvas* canvas_t; //一个画布

typedef unsigned int color_t; //颜色 0xff1122  

typedef struct {
	int left;
	int top;
	int right;
	int bottom;	
}rect; //区域

//从内存加载图像
image_t image_load(void* data, int len);

typedef void image_write_func(void* userdata, void *data, int size);

//保存图像为指定格式数据，调用‘写函数’来写数据
int image_save(image_t img, int encode, image_write_func func, void* userdata);

//从文件加载图像 
image_t image_load_from_file(const char* name);

//保存图像到文件
int image_save_to_file(image_t img, int encode, const char* name);

//直接创建一个指定大小的图像
image_t image_alloc(int width, int height, int type);

//直接创建一个使用指定内存像素的图像 (rgb内存大小为 width*height*3, rgba内存大小为 width*height*4)
image_t image_create(void* pixel, int width, int height, int type);

//拷贝,得到一个新图像
image_t image_copy(image_t img);

//缩放,得到一个新图像
image_t image_scaled(image_t img, int width, int height);

//缩放某一部分, 得到一个新图像
image_t image_scaled_partial(image_t img, int width, int height, const rect* rc);

//释放图片
void image_free(image_t img);


//获取图像宽度
int image_width(image_t img);

//获取图像高度
int image_height(image_t img);

//获取图像类型 （目前只有 RGB 或 RGBA）
int image_type(image_t img);

//获取图像像素内存地址（用于自由读写）
void* image_lock_pixel(image_t img, int* len);

//读写完后释放图像像素内存地址
void image_unlock_pixel(image_t img, void* pixel);


//获取系统默认字体family, (目前未实现)
font_family_t font_family_get_default();

//从ttf字体文件的内存数据加载字体family
font_family_t font_family_load(void* data, int len);

//从ttf字体文件加载字体family
font_family_t font_family_load_from_file(const char* file);

//释放字体family
void font_family_free(font_family_t f);

//按字体family、字体大小、字体样式，创建一个字体对象
font_t font_create(font_family_t f, int size, int style);

//释放字体对象
void font_free(font_t f);


//在指定的图像上创建一个画布
canvas_t canvas_create(image_t img);

//释放画布
void canvas_free(canvas_t f);

//指定位置画图
int canvas_draw_image(canvas_t c, image_t img, int x, int y);

//指定位置画图 （只绘制源图片的一部分）
int canvas_draw_image_partial(canvas_t c, image_t img, int x, int y, const rect* src);

//指定位置画图（缩放） 
int canvas_draw_image_scaled(canvas_t c, image_t img, const rect* dst);

//指定位置画图（缩放）（只绘制源图片的一部分） 
int canvas_draw_image_scaled_partial(canvas_t c, image_t img, const rect* dst, const rect* src);

//计算字符串显示宽度 (单行)
int canvas_measure_text_width(canvas_t c, font_t font, const char* str);

//画字符串 (单行)
int canvas_draw_text(canvas_t c, font_t font, color_t color, const char* str, int x, int y);

//画字符串（单行，可指定位置居中|靠右等）
int canvas_draw_text_aligned(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int align);

//画字符串（在矩形区域内, 自动换行, 支持超出显示...）
int canvas_draw_text_multiline(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int flag);

//填充矩形
int canvas_fill_rect(canvas_t c, color_t color, const rect* rc);
