#pragma once



/*
����˵����
������������int�ģ�0��ʾ�ɹ���������ʾerrno������

*/


//ͼ������
#define IMAGE_TYPE_RGB   3
#define IMAGE_TYPE_RGBA  4

//ͼ���������
#define IMAGE_ENCODE_BMP 1
#define IMAGE_ENCODE_PNG 2
#define IMAGE_ENCODE_JPG 3

//������ʽ
//#define FONT_STYLE_BOLD      1 //�Ӵ� (Ŀǰδʵ��)
//#define FONT_STYLE_ITALIC    2 //б�� (Ŀǰδʵ��)
#define FONT_STYLE_UNDERLINE 4 //�»���
#define FONT_STYLE_STRIKEOUT 8 //ɾ����

//�ı�λ��
#define TEXT_ALIGN_LEFT    0
#define TEXT_ALIGN_RIGHT   1
#define TEXT_ALIGN_CENTER  2
#define TEXT_VALIGN_TOP    0x00
#define TEXT_VALIGN_BOTTOM 0x10
#define TEXT_VALIGN_CENTER 0x20

//�����ı�ѡ��
#define TEXT_FLAG_ELLIPSIS 0x0100  //������ʾʡ�Ժ�


typedef struct _image* image_t; //һ��ͼ��

typedef struct _font_family* font_family_t; //ָ��������

typedef struct _font* font_t;  //ָ�������� ��С26 б��

typedef struct _canvas* canvas_t; //һ������

typedef unsigned int color_t; //��ɫ 0xff1122  

typedef struct {
	int left;
	int top;
	int right;
	int bottom;	
}rect; //����

//���ڴ����ͼ��
image_t image_load(void* data, int len);

typedef void image_write_func(void* userdata, void *data, int size);

//����ͼ��Ϊָ����ʽ���ݣ����á�д��������д����
int image_save(image_t img, int encode, image_write_func func, void* userdata);

//���ļ�����ͼ�� 
image_t image_load_from_file(const char* name);

//����ͼ���ļ�
int image_save_to_file(image_t img, int encode, const char* name);

//ֱ�Ӵ���һ��ָ����С��ͼ��
image_t image_alloc(int width, int height, int type);

//ֱ�Ӵ���һ��ʹ��ָ���ڴ����ص�ͼ�� (rgb�ڴ��СΪ width*height*3, rgba�ڴ��СΪ width*height*4)
image_t image_create(void* pixel, int width, int height, int type);

//����,�õ�һ����ͼ��
image_t image_copy(image_t img);

//����,�õ�һ����ͼ��
image_t image_scaled(image_t img, int width, int height);

//����ĳһ����, �õ�һ����ͼ��
image_t image_scaled_partial(image_t img, int width, int height, const rect* rc);

//�ͷ�ͼƬ
void image_free(image_t img);


//��ȡͼ����
int image_width(image_t img);

//��ȡͼ��߶�
int image_height(image_t img);

//��ȡͼ������ ��Ŀǰֻ�� RGB �� RGBA��
int image_type(image_t img);

//��ȡͼ�������ڴ��ַ���������ɶ�д��
void* image_lock_pixel(image_t img, int* len);

//��д����ͷ�ͼ�������ڴ��ַ
void image_unlock_pixel(image_t img, void* pixel);


//��ȡϵͳĬ������family, (Ŀǰδʵ��)
font_family_t font_family_get_default();

//��ttf�����ļ����ڴ����ݼ�������family
font_family_t font_family_load(void* data, int len);

//��ttf�����ļ���������family
font_family_t font_family_load_from_file(const char* file);

//�ͷ�����family
void font_family_free(font_family_t f);

//������family�������С��������ʽ������һ���������
font_t font_create(font_family_t f, int size, int style);

//�ͷ��������
void font_free(font_t f);


//��ָ����ͼ���ϴ���һ������
canvas_t canvas_create(image_t img);

//�ͷŻ���
void canvas_free(canvas_t f);

//ָ��λ�û�ͼ
int canvas_draw_image(canvas_t c, image_t img, int x, int y);

//ָ��λ�û�ͼ ��ֻ����ԴͼƬ��һ���֣�
int canvas_draw_image_partial(canvas_t c, image_t img, int x, int y, const rect* src);

//ָ��λ�û�ͼ�����ţ� 
int canvas_draw_image_scaled(canvas_t c, image_t img, const rect* dst);

//ָ��λ�û�ͼ�����ţ���ֻ����ԴͼƬ��һ���֣� 
int canvas_draw_image_scaled_partial(canvas_t c, image_t img, const rect* dst, const rect* src);

//�����ַ�����ʾ��� (����)
int canvas_measure_text_width(canvas_t c, font_t font, const char* str);

//���ַ��� (����)
int canvas_draw_text(canvas_t c, font_t font, color_t color, const char* str, int x, int y);

//���ַ��������У���ָ��λ�þ���|���ҵȣ�
int canvas_draw_text_aligned(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int align);

//���ַ������ھ���������, �Զ�����, ֧�ֳ�����ʾ...��
int canvas_draw_text_multiline(canvas_t c, font_t font, color_t color, const char* str, const rect* dst, int flag);

//������
int canvas_fill_rect(canvas_t c, color_t color, const rect* rc);
