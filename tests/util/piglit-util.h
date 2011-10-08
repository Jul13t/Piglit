/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"

#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_MSC_VER)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define bool BOOL
#define true 1
#define false 0
#else
#include <stdint.h>
#include <stdbool.h>
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <piglit/gl_wrap.h>
#include <piglit/glut_wrap.h>

#if defined(_MSC_VER)

#define snprintf sprintf_s

static __inline double
round(double x) {
	return x >= 0.0 ? floor(x + 0.5) : ceil(x - 0.5);
}

static __inline float
roundf(float x) {
	return x >= 0.0f ? floorf(x + 0.5f) : ceilf(x - 0.5f);
}

#define piglit_get_proc_address(x) wglGetProcAddress(x)

#else /* !defined(_MSC_VER) */

#define piglit_get_proc_address(x) glutGetProcAddress(x)

#endif /* !defined(_MSC_VER) */

enum piglit_result {
	PIGLIT_PASS,
	PIGLIT_FAIL,
	PIGLIT_SKIP,
	PIGLIT_WARN
};

#include "piglit-framework.h"
#include "piglit-shader.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))

extern const uint8_t fdo_bitmap[];
extern const unsigned int fdo_bitmap_width;
extern const unsigned int fdo_bitmap_height;

/**
 * Call glutInit() and, if EGLUT is used, also call glutInitAPIMask().
 */
void piglit_glutInit(int argc, char **argv);

/**
 * Determine if the API is OpenGL ES.
 */
bool piglit_is_gles();

/**
 * \brief Get version of OpenGL or OpenGL ES API.
 *
 * Returned version is multiplied by 10 to make it an integer.  So for
 * example, if the GL version is 2.1, the return value is 21.
 */
int piglit_get_gl_version();

/**
 * \precondition name is not null
 */
bool piglit_is_extension_supported(const char *name);

/**
 * \brief Convert a GL error to a string.
 *
 * For example, given GL_INVALID_ENUM, return "GL_INVALID_ENUM".
 *
 * Return "(unrecognized error)" if the enum is not recognized.
 *
 * \todo Create a more general function that stringifies any GLenum.
 */
const char* piglit_get_gl_error_name(GLenum error);

/**
 * \brief Check for unexpected GL errors and possibly terminate the test.
 *
 * If glGetError() returns an error other than \c expected_error, then
 * print a diagnostic and terminate the test with the given \c result.
 */
void piglit_check_gl_error(GLenum expected_error, enum piglit_result result);

int FindLine(const char *program, int position);
void piglit_report_result(enum piglit_result result);
void piglit_require_extension(const char *name);
void piglit_require_not_extension(const char *name);
int piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe);
int piglit_probe_pixel_rgb(int x, int y, const float* expected);
int piglit_probe_pixel_rgba(int x, int y, const float* expected);
int piglit_probe_rect_rgb(int x, int y, int w, int h, const float* expected);
int piglit_probe_rect_rgba(int x, int y, int w, int h, const float* expected);
int piglit_probe_image_rgb(int x, int y, int w, int h, const float *image);
int piglit_probe_image_rgba(int x, int y, int w, int h, const float *image);
int piglit_probe_texel_rect_rgb(int target, int level, int x, int y,
				int w, int h, const float *expected);
int piglit_probe_texel_rgb(int target, int level, int x, int y,
			   const float* expected);
int piglit_probe_texel_rect_rgba(int target, int level, int x, int y,
				 int w, int h, const float *expected);
int piglit_probe_texel_rgba(int target, int level, int x, int y,
			    const float* expected);
int piglit_probe_pixel_depth(int x, int y, float expected);
int piglit_probe_rect_depth(int x, int y, int w, int h, float expected);
int piglit_probe_pixel_stencil(int x, int y, unsigned expected);
int piglit_probe_rect_stencil(int x, int y, int w, int h, unsigned expected);
int piglit_probe_rect_halves_equal_rgba(int x, int y, int w, int h);

int piglit_use_fragment_program(void);
int piglit_use_vertex_program(void);
void piglit_require_fragment_program(void);
void piglit_require_vertex_program(void);
GLuint piglit_compile_program(GLenum target, const char* text);
GLvoid piglit_draw_rect(float x, float y, float w, float h);
GLvoid piglit_draw_rect_z(float z, float x, float y, float w, float h);
GLvoid piglit_draw_rect_tex(float x, float y, float w, float h,
                            float tx, float ty, float tw, float th);
GLvoid piglit_draw_rect_back(float x, float y, float w, float h);

void piglit_escape_exit_key(unsigned char key, int x, int y);

char *piglit_load_text_file(const char *file_name, unsigned *size);

void piglit_gen_ortho_projection(double left, double right, double bottom,
				 double top, double near_val, double far_val,
				 GLboolean push);
void piglit_ortho_projection(int w, int h, GLboolean push);

GLuint piglit_checkerboard_texture(GLuint tex, unsigned level,
    unsigned width, unsigned height,
    unsigned horiz_square_size, unsigned vert_square_size,
    const float *black, const float *white);
GLuint piglit_rgbw_texture(GLenum format, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype);
GLuint piglit_depth_texture(GLenum target, GLenum format, int w, int h, int d, GLboolean mip);
extern float piglit_tolerance[4];
void piglit_set_tolerance_for_bits(int rbits, int gbits, int bbits, int abits);

extern GLfloat cube_face_texcoords[6][4][3];
extern const char *cube_face_names[6];
extern const GLenum cube_face_targets[6];

/**
 * Common vertex program code to perform a model-view-project matrix transform
 */
#define PIGLIT_VERTEX_PROGRAM_MVP_TRANSFORM		\
	"ATTRIB	iPos = vertex.position;\n"		\
	"OUTPUT	oPos = result.position;\n"		\
	"PARAM	mvp[4] = { state.matrix.mvp };\n"	\
	"DP4	oPos.x, mvp[0], iPos;\n"		\
	"DP4	oPos.y, mvp[1], iPos;\n"		\
	"DP4	oPos.z, mvp[2], iPos;\n"		\
	"DP4	oPos.w, mvp[3], iPos;\n"

/**
 * Handle to a generic fragment program that passes the input color to output
 *
 * \note
 * Either \c piglit_use_fragment_program or \c piglit_require_fragment_program
 * must be called before using this program handle.
 */
extern GLint piglit_ARBfp_pass_through;

#ifndef HAVE_STRCHRNUL
char *strchrnul(const char *s, int c);
#endif

extern void piglit_set_rlimit(unsigned long lim);

static const GLint PIGLIT_ATTRIB_POS = 0;
static const GLint PIGLIT_ATTRIB_TEX = 1;

#ifdef __cplusplus
} /* end extern "C" */
#endif
