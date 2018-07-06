#include "xgraphics.h"
#include "jni.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "memory_helper.h"
#include "memory_tlspool.h"

#define _JNI_NAME_CAT_HELP(a,b) a##b
#define _JNI_NAME_CAT(a,b) _JNI_NAME_CAT_HELP(a,b)

//jni函数需要以特殊名称开头 Java_包名_类名_
#ifndef JNI_PREFIX
#error "must define JNI_PREFIX, sample:Java_com_sun_io_NativeLib_"
#endif

//定义jni函数名
#define JNI_FUNC(ret,fn)  extern "C" JNIEXPORT ret JNICALL _JNI_NAME_CAT(JNI_PREFIX,fn)



//static long loadImage(byte[] data)
JNI_FUNC(jlong, loadImage)(JNIEnv* env, jclass cls, jbyteArray arr) {
	jboolean copy = JNI_FALSE;
	jbyte* data = env->GetByteArrayElements(arr, &copy);
	jsize sz = env->GetArrayLength(arr);
	image_t img = image_load(data, sz);
	env->ReleaseByteArrayElements(arr, data, JNI_ABORT);
	return (jlong)(uintptr_t)img;
}

//static long loadImageFromFile(String filename)
JNI_FUNC(jlong, loadImageFromFile)(JNIEnv* env, jclass cls, jstring filename) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(filename, &copy);	
	image_t img = image_load_from_file(data);
	env->ReleaseStringUTFChars(filename, data);
	return (jlong)(uintptr_t)img;
}


//static int saveImageToFile(long img, int encode, String filename)
JNI_FUNC(jint, saveImageToFile)(JNIEnv* env, jclass cls, jlong img, jint encode, jstring filename) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(filename, &copy);
	jint ret = image_save_to_file((image_t)(uintptr_t)img, encode, data);
	env->ReleaseStringUTFChars(filename, data);
	return ret;
}



typedef struct write_byte_array_context {
	JNIEnv* env;
	void* buf;
	int buflen;
	int len;
}write_byte_array_context;

static void write_to_jbyteArray(void* arg, void* data, int len) {
	write_byte_array_context* ctx = (write_byte_array_context*) arg;
	if (!ctx->buf) {
		return;
	}
	if(ctx->len < 0)return;
	if (ctx->len + len >= ctx->buflen) {
		//内存不足		
		ctx->len = -1;	
		return;
	}
	memcpy((char*)(ctx->buf) +  ctx->len, data, len);
	ctx->len += len;
}

//static byte[] encodeImage(long img, int encode)
JNI_FUNC(jbyteArray, encodeImage)(JNIEnv* env, jclass cls, jlong img, jint encode) {
	jbyteArray arr = NULL;
	write_byte_array_context ctx;
	ctx.env = env;
	ctx.buflen = 1024 * 200; //预先分配 200K
	ctx.buf = malloc(ctx.buflen);	
	if (!ctx.buf) {
		return NULL;
	}
	ctx.len = 0;
	int err = image_save((image_t)(uintptr_t)img, encode, write_to_jbyteArray, &ctx);
	if (err == 0 && ctx.len > 0) { //成功
		arr = env->NewByteArray(ctx.len);
		env->SetByteArrayRegion(arr, 0, ctx.len, (jbyte*)ctx.buf);
	}
	if (ctx.buf) {
		free(ctx.buf);
	}	
	return arr;
}


//static long allocImage(int width, int height, int type)
JNI_FUNC(jlong, allocImage)(JNIEnv* env, jclass cls, jint width, jint height, jint type) {	
	image_t img = image_alloc(width, height, type);	
	return (jlong)(uintptr_t)img;
}

//static long createImage(byte[] data, int width, int height, int type)
JNI_FUNC(jlong, createImage)(JNIEnv* env, jclass cls, jbyteArray arr, jint width, jint height, jint type) {
	jboolean copy = JNI_FALSE;
	jbyte* data = env->GetByteArrayElements(arr, &copy);
	jsize sz = env->GetArrayLength(arr);
	image_t img = image_create(data, width, height, type);
	env->ReleaseByteArrayElements(arr, data, JNI_ABORT);
	return (jlong)(uintptr_t)img;
}

//static long copyImage(long img)
JNI_FUNC(jlong, copyImage)(JNIEnv* env, jclass cls, jlong oldimg) {	
	image_t img = image_copy((image_t)(uintptr_t)oldimg);
	return (jlong)(uintptr_t)img;
}

//static long scaleImage(long img, int width, int height)
JNI_FUNC(jlong, scaleImage)(JNIEnv* env, jclass cls, jlong oldimg, jint width, jint height) {
	image_t img = image_scaled((image_t)(uintptr_t)oldimg, width, height);
	return (jlong)(uintptr_t)img;
}

//static long scaleImagePartial(long img, int width, int height, int srcx, int srcy, int srcw, int srch)
JNI_FUNC(jlong, scaleImagePartial)(JNIEnv* env, jclass cls, jlong oldimg, jint width, jint height, jint srcx, jint srcy, jint srcw, jint srch) {
	rect rc;
	rc.left = srcx;
	rc.top = srcy;
	rc.right = srcx + srcw;
	rc.bottom = srcy + srch;
	image_t img = image_scaled_partial((image_t)(uintptr_t)oldimg, width, height, &rc);
	return (jlong)(uintptr_t)img;
}

//static void freeImage(long img)
JNI_FUNC(void, freeImage)(JNIEnv* env, jclass cls, jlong oldimg) {
	image_free((image_t)(uintptr_t)oldimg);	
}


//static int getImageWidth(long img)
JNI_FUNC(jint, getImageWidth)(JNIEnv* env, jclass cls, jlong oldimg) {
	return image_width((image_t)(uintptr_t)oldimg);
}
//static int getImageHeight(long img)
JNI_FUNC(jint, getImageHeight)(JNIEnv* env, jclass cls, jlong oldimg) {
	return image_height((image_t)(uintptr_t)oldimg);
}
//static int getImageType(long img)
JNI_FUNC(jint, getImageType)(JNIEnv* env, jclass cls, jlong oldimg) {
	return image_type((image_t)(uintptr_t)oldimg);
}
//static byte[] getImagePixel(long img)
JNI_FUNC(jbyteArray, getImagePixel)(JNIEnv* env, jclass cls, jlong oldimg) {
	jbyteArray ret = NULL;
	image_t img = (image_t)(uintptr_t)oldimg;
	int len = 0;
	void* pixel = image_lock_pixel(img, &len);
	if (pixel) {
		ret = env->NewByteArray(len);
		if (ret) {
			env->SetByteArrayRegion(ret, 0, len, (jbyte*)pixel);
		}
		image_unlock_pixel(img, pixel);
	}
	return ret;
}

//static long loadFontFamily(byte[] data)
JNI_FUNC(jlong, loadFontFamily)(JNIEnv* env, jclass cls, jbyteArray arr) {
	jboolean copy = JNI_FALSE;
	jbyte* data = env->GetByteArrayElements(arr, &copy);
	jsize sz = env->GetArrayLength(arr);
	font_family_t ptr = font_family_load(data, sz);
	env->ReleaseByteArrayElements(arr, data, JNI_ABORT);
	return (jlong)(uintptr_t)ptr;
}

//static long loadFontFamilyFromFile(String filename)
JNI_FUNC(jlong, loadFontFamilyFromFile)(JNIEnv* env, jclass cls, jstring filename) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(filename, &copy);
	font_family_t ptr = font_family_load_from_file(data);
	env->ReleaseStringUTFChars(filename, data);
	return (jlong)(uintptr_t)ptr;
}

//static void freeFontFamily(long img)
JNI_FUNC(void, freeFontFamily)(JNIEnv* env, jclass cls, jlong oldimg) {
	font_family_free((font_family_t)(uintptr_t)oldimg);
}

//static long createFont(long family, int size, int style)
JNI_FUNC(jlong, createFont)(JNIEnv* env, jclass cls, jlong family, jint size, jint style) {
	font_t ptr = font_create((font_family_t)(uintptr_t)family, size, style);
	return (jlong)(uintptr_t)ptr;
}
//static void freeFont(long img)
JNI_FUNC(void, freeFont)(JNIEnv* env, jclass cls, jlong ptr) {
	font_free((font_t)(uintptr_t)ptr);
}

//static long createCanvas(long img)
JNI_FUNC(jlong, createCanvas)(JNIEnv* env, jclass cls, jlong ptr) {
	canvas_t c = canvas_create((image_t)(uintptr_t)ptr);
	return (jlong)(uintptr_t)c;
}
//static void freeCanvas(long img)
JNI_FUNC(void, freeCanvas)(JNIEnv* env, jclass cls, jlong ptr) {
	canvas_free((canvas_t)(uintptr_t)ptr);
}

//static int drawImage(long canvas, long img, int x, int y)
JNI_FUNC(jint, drawImage)(JNIEnv* env, jclass cls, jlong canvas, jlong img, jint x, jint y) {
	return canvas_draw_image((canvas_t)(uintptr_t)canvas, (image_t)(uintptr_t)img, x, y);
}

//static int drawImagePartial(long canvas, long img, int x, int y, int srcx, int srcy, int srcw, int srch)
JNI_FUNC(jint, drawImagePartial)(JNIEnv* env, jclass cls, jlong canvas, jlong img, jint x, jint y, jint srcx, jint srcy, jint srcw, jint srch) {
	rect rc;
	rc.left = srcx;
	rc.top = srcy;
	rc.right = srcx + srcw;
	rc.bottom = srcy + srch;
	return canvas_draw_image_partial((canvas_t)(uintptr_t)canvas, (image_t)(uintptr_t)img, x, y, &rc);
}

//static int drawImageScaled(long canvas, long img, int x, int y, int w, int h)
JNI_FUNC(jint, drawImageScaled)(JNIEnv* env, jclass cls, jlong canvas, jlong img, jint x, jint y, jint w, jint h) {
	rect rc;
	rc.left = x;
	rc.top = y;
	rc.right = x + w;
	rc.bottom = y + h;
	return canvas_draw_image_scaled((canvas_t)(uintptr_t)canvas, (image_t)(uintptr_t)img, &rc);
}

//static int drawImageScaledPartial(long canvas, long img, int x, int y, int w, int h, int srcx, int srcy, int srcw, int srch)
JNI_FUNC(jint, drawImageScaledPartial)(JNIEnv* env, jclass cls, jlong canvas, jlong img, jint x, jint y, jint w, jint h, jint srcx, jint srcy, jint srcw, jint srch) {
	rect src;
	src.left = srcx;
	src.top = srcy;
	src.right = srcx + srcw;
	src.bottom = srcy + srch;
	rect dst;
	dst.left = x;
	dst.top = y;
	dst.right = x + w;
	dst.bottom = y + h;
	return canvas_draw_image_scaled_partial((canvas_t)(uintptr_t)canvas, (image_t)(uintptr_t)img, &dst, &src);
}


//static int drawText(long canvas, long font, int color, String str, int x, int y)
JNI_FUNC(jint, drawText)(JNIEnv* env, jclass cls, jlong canvas, jlong font, jint color, jstring str, jint x, jint y) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(str, &copy);	
	int ret = canvas_draw_text((canvas_t)(uintptr_t)canvas, (font_t)(uintptr_t)font, color, data, x, y);
	env->ReleaseStringUTFChars(str, data);
	return ret;
}

//static int drawTextAligned(long canvas, long font, int color, String str, int x, int y, int w, int h, int align)
JNI_FUNC(jint, drawTextAligned)(JNIEnv* env, jclass cls, jlong canvas, jlong font, jint color, jstring str, jint srcx, jint srcy, jint srcw, jint srch, jint align) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(str, &copy);
	rect rc;
	rc.left = srcx;
	rc.top = srcy;
	rc.right = srcx + srcw;
	rc.bottom = srcy + srch;
	int ret = canvas_draw_text_aligned((canvas_t)(uintptr_t)canvas, (font_t)(uintptr_t)font, color, data, &rc, align);
	env->ReleaseStringUTFChars(str, data);
	return ret;
}

//static int drawTextMultiLine(long canvas, long font, int color, String str, int x, int y, int w, int h, int flag)
JNI_FUNC(jint, drawTextMultiLine)(JNIEnv* env, jclass cls, jlong canvas, jlong font, jint color, jstring str, jint srcx, jint srcy, jint srcw, jint srch, jint flag) {
	jboolean copy = JNI_FALSE;
	const char* data = env->GetStringUTFChars(str, &copy);
	rect rc;
	rc.left = srcx;
	rc.top = srcy;
	rc.right = srcx + srcw;
	rc.bottom = srcy + srch;
	int ret = canvas_draw_text_multiline((canvas_t)(uintptr_t)canvas, (font_t)(uintptr_t)font, color, data, &rc, flag);
	env->ReleaseStringUTFChars(str, data);
	return ret;
}

//static int fillRect(long canvas, int color, int x, int y, int w, int h)
JNI_FUNC(jint, fillRect)(JNIEnv* env, jclass cls, jlong canvas, jint color, jint srcx, jint srcy, jint srcw, jint srch) {		
	rect rc;
	rc.left = srcx;
	rc.top = srcy;
	rc.right = srcx + srcw;
	rc.bottom = srcy + srch;
	int ret = canvas_fill_rect((canvas_t)(uintptr_t)canvas, color, &rc);	
	return ret;
}

//static void clearMemory()
JNI_FUNC(void, clearMemory)(JNIEnv* env, jclass cls) {
#ifdef USE_MEMORY_HELPER
	memory_record_clear();
#endif
}
//static void printMemory()
JNI_FUNC(void, printMemory)(JNIEnv* env, jclass cls) {
#ifdef USE_MEMORY_HELPER
	memory_record_print();
#endif
}

//static void setMemorySize()
JNI_FUNC(void, setMemorySize)(JNIEnv* env, jclass cls, jint sz) {
#ifdef USE_MEMORY_TLSPOOL
	memory_tlspool_init(sz);
#endif
}

//static void resetMemory()
JNI_FUNC(void, resetMemory)(JNIEnv* env, jclass cls) {
#ifdef USE_MEMORY_TLSPOOL
	memory_tlspool_reset();
#endif
}
