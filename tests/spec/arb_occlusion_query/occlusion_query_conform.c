/*  BEGIN_COPYRIGHT -*- glean -*-
 * 
 *  Copyright (C) 1999  Allen Akin   All Rights Reserved.
 *  Copyright (C) 2015  Intel Corporation.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *  Wei Wang <wei.z.wang@intel.com>
 * 
 */
 
/** @file occlusion_query_conform.c  
 *  
 *  Conformance test on ARB_occlusion_query extension.
 *
 */

#include "piglit-util-gl.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 180;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

#define GL_GLEXT_PROTOTYPES
#define START_QUERY(id)\
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);


#define TERM_QUERY()\
	glEndQueryARB(GL_SAMPLES_PASSED_ARB);\


/* Generate a box which will be occluded by the occluder */
void
gen_box(GLfloat left, GLfloat right,
			  GLfloat top, GLfloat btm)
{
	glBegin(GL_POLYGON);
	glVertex3f(left, top, 0);
	glVertex3f(right, top, 0);
	glVertex3f(right, btm, 0);
	glVertex3f(left,  btm, 0);
	glEnd();
}

bool
chk_ext()
{
	const char *ext = (const char *) glGetString(GL_EXTENSIONS);

	if (!strstr(ext, "GL_ARB _occlusion_query")) {
		printf(" %s Warning: Extension GL_ARB_occlusion_query is missing.\n", ext);
		return false;
	}

	return true;
}

void
setup()
{
}

GLuint 
find_unused_id()
{
	/* used the regular random number generator. */
	srand((unsigned)time(NULL));
	unsigned int id;
	int counter = 0;

#define MAX_FIND_ID_ROUND 256

	while (1) {
		/* assuming that at least 2^32-1 <id> can be generated */
		id = rand();
		if (id != 0 && glIsQueryARB(id) == GL_FALSE)
			return id;
		if (++ counter >= MAX_FIND_ID_ROUND) {
			char str[1000];
			sprintf(str, "Cannot find the unused id after [%d] tries.",
					MAX_FIND_ID_ROUND);
			printf("Warning: %s\n", str);
			return 0;
		}
	}
}


/* If multiple queries are issued on the same target and id prior to calling
 * GetQueryObject[u]iVARB, the result returned will always be from the last
 * query issued.  The results from any queries before the last one will be lost
 * if the results are not retrieved before starting a new query on the same
 * target and id.
 */
bool
conformOQ_GetObjivAval_multi1(GLuint id)
{
	GLint ready;
	GLuint passed = 0;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -1.0, 1.0, -1.0, 1.0, 0.0, 25.0 );
	
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0);

	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	gen_box(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef( 0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	/* draw the 1st box (gren) which is occluded by the occluder partly */
	START_QUERY(id);
	glColor3f(0, 1, 0);
	gen_box(-0.51,  0.51, 0.51, -0.51);
	TERM_QUERY();

	/* draw the 2nd box (blue) which is occluded by the occluder throughly */
	START_QUERY(id);
	glColor3f(0, 0, 1);
	gen_box(-0.4, 0.4, 0.4, -0.4); 
	TERM_QUERY();

	glPopMatrix();

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	
	do {
		glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, &ready);
	} while (!ready);
	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, &passed);

	/* 'passed' should be zero */
	return passed > 0 ? false : true;
}


/* If mutiple queries are issued on the same target and diff ids prior
 * to calling GetQueryObject[u]iVARB, the results should be
 * corresponding to those queries (ids) respectively.
 */
bool
conformOQ_GetObjivAval_multi2()
{
	GLuint passed1 = 0, passed2 = 0, passed3 = 0;
	GLuint id1, id2, id3;

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( -1.0, 1.0, -1.0, 1.0, 0.0, 25.0 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
	glTranslatef( 0.0, 0.0, -10.0);


	/* draw the occluder (red) */
	glColorMask(1, 1, 1, 1);
	glDepthMask(GL_TRUE);
	glColor3f(1, 0, 0);
	gen_box(-0.5, 0.5, 0.5, -0.5);

	glPushMatrix();
	glTranslatef( 0.0, 0.0, -5.0);
	glColorMask(0, 0, 0, 0);
	glDepthMask(GL_FALSE);

	id1 = find_unused_id();
	START_QUERY(id1);
	/* draw green quad, much larger than occluder */
	glColor3f(0, 1, 0);
	gen_box(-0.7, 0.7, 0.7, -0.7);
	TERM_QUERY();

	id2 = find_unused_id();
	START_QUERY(id2);
	/* draw blue quad, slightly larger than occluder */
	glColor3f(0, 0, 1);
	gen_box(-0.53, 0.53, 0.53, -0.53);
	TERM_QUERY();

	id3 = find_unused_id();
	START_QUERY(id3);
	/* draw white quad, smaller than occluder (should not be visible) */
	glColor3f(1, 1, 1);
	gen_box(-0.4, 0.4, 0.4, -0.4);
	TERM_QUERY();

	glPopMatrix();

	glGetQueryObjectuivARB(id1, GL_QUERY_RESULT_ARB, &passed1);
	glGetQueryObjectuivARB(id2, GL_QUERY_RESULT_ARB, &passed2);
	glGetQueryObjectuivARB(id3, GL_QUERY_RESULT_ARB, &passed3);

	glDepthMask(GL_TRUE);

	
	glDeleteQueriesARB(1, &id1);
	glDeleteQueriesARB(1, &id2);
	glDeleteQueriesARB(1, &id3);

	glPopMatrix();
	glMatrixMode( GL_PROJECTION );
	glPopMatrix();
	
	if ( passed1 > passed2 && passed2 > passed3 && passed3 == 0)
		return true;
	else
		return false;
}


/* 
 * void GetQueryivARB(enum target, enum pname, int *params);
 *
 * If <pname> is QUERY_COUNTER_BITS_ARB, the number of bits in the counter
 * for <target> will be placed in <params>.  The minimum number of query
 * counter bits allowed is a function of the implementation's maximum
 * viewport dimensions (MAX_VIEWPORT_DIMS).  If the counter is non-zero,
 * then the counter must be able to represent at least two overdraws for
 * every pixel in the viewport using only one sample buffer.  The formula to
 * compute the allowable minimum value is below (where n is the minimum
 * number of bits): 
 *   n = (min (32, ceil (log2 (maxViewportWidth x maxViewportHeight x 2) ) ) ) or 0
 */
bool
conformOQ_GetQry_CnterBit()
{
	int bit_num, dims[2];
	GLenum err;
	float min_impl, min_bit_num;

	/* get the minimum bit number supported by the implementation, 
	 * and check the legality of result of GL_QUERY_COUNTER_BITS_ARB */
	glGetQueryivARB(GL_SAMPLES_PASSED_ARB, GL_QUERY_COUNTER_BITS_ARB, &bit_num);
	glGetIntegerv(GL_MAX_VIEWPORT_DIMS, dims);
	err = glGetError();
	if (err == GL_INVALID_OPERATION || err == GL_INVALID_ENUM) 
		return false;

	min_impl = ceil(logf((float)dims[0]*(float)dims[1]*1.0*2.0) / logf(2.0));
	min_bit_num = 32.0 > min_impl ? min_impl : 32.0;

	if ((float)bit_num < min_bit_num)
		return false;

	return true;
}


/* If BeginQueryARB is called with an unused <id>, that name is marked as used
 * and associated with a new query object. */
bool
conformOQ_Begin_unused_id()
{
	unsigned int id;
	bool pass = true;

	id = find_unused_id();

	if (id == 0)
		return false;

	glBeginQuery(GL_SAMPLES_PASSED_ARB, id);

	if (glIsQueryARB(id) == GL_FALSE) {
		printf("Error : Begin with a unused id failed.");
		pass = false;
	}

	glEndQuery(GL_SAMPLES_PASSED_ARB);

	return pass;
}


/* if EndQueryARB is called while no query with the same target is in progress,
 * an INVALID_OPERATION error is generated. */
bool
conformOQ_EndAfter(GLuint id)
{
	START_QUERY(id);
	TERM_QUERY();  

	glEndQueryARB(GL_SAMPLES_PASSED_ARB);

	if (glGetError() != GL_INVALID_OPERATION) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "EndQuery when there is no queries.\n");
		return false;
	}

	return true;
}


/* Calling either GenQueriesARB while any query of any target is active
 * should not cause any error to be generated. */
bool 
conformOQ_GenIn(GLuint id)
{
	int pass = true;

	START_QUERY(id);

	glGenQueriesARB(1, &id);
	if (glGetError() != GL_NO_ERROR) {
		printf(" Error: Error generated when GenQueries called "
		       "in the progress of another.\n");
		pass = false;
	}
  
	TERM_QUERY();

	return pass;
}


/* Calling either DeleteQueriesARB while any query of any target is active
 * should not cause any error to be generated. */
bool
conformOQ_DeleteIn(GLuint id)
{
	int pass = true;
	unsigned int another_id;

	START_QUERY(id);

	if (id > 0) {
		glGenQueriesARB(1, &another_id);
		glDeleteQueriesARB(1, &another_id);

		if (glGetError() != GL_NO_ERROR) {
			printf(" Error: Error generated when DeleteQueries called "
			       "in the progress of another.\n");
			pass = false;
		}
	}
  
	TERM_QUERY();

	return pass;
}


/* If BeginQueryARB is called while another query is already in progress with
 * the same target, an INVALID_OPERATION error should be generated. */
bool
conformOQ_BeginIn(GLuint id)
{
	int pass = true;

	START_QUERY(id);

	/* Issue another BeginQueryARB while another query is already in 
	   progress */
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, id);

	if (glGetError() != GL_INVALID_OPERATION) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "BeginQuery in the progress of another.\n");
		pass = false;
	}

	TERM_QUERY();
	return pass;
}


/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated. */
bool
conformOQ_GetObjAvalIn(GLuint id)
{
	int pass = true;
	GLint param;

	START_QUERY(id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, &param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_AVAILABLE_ARB, (GLuint *)&param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	if (pass == false) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "GetQueryObjectuiv with GL_QUERY_RESULT_AVAILABLE_ARB "
		       "in the active progress.\n");
	}
	TERM_QUERY();

	return pass;
}


/* if the query object named by <id> is currently active, then an
 * INVALID_OPERATION error is generated. */
bool
conformOQ_GetObjResultIn(GLuint id)
{
	int pass = true;
	GLint param;

	START_QUERY(id);

	glGetQueryObjectivARB(id, GL_QUERY_RESULT_ARB, &param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	glGetQueryObjectuivARB(id, GL_QUERY_RESULT_ARB, (GLuint *)&param);
	if (glGetError() != GL_INVALID_OPERATION)
		pass = false;

	if (pass == false) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
		       "GetQueryObject[u]iv with GL_QUERY_RESULT_ARB "
		       "in the active progress.\n");
	}
	TERM_QUERY();

	return pass;
}


/* If <id> is not the name of a query object, then an INVALID_OPERATION error
 * is generated. */
bool
conformOQ_GetObjivAval(GLuint id)
{
	GLuint id_tmp;
	GLint param;
	
	START_QUERY(id);
	TERM_QUERY();  
	
	id_tmp = find_unused_id();
 
	if (id_tmp == 0)
		return false;

	glGetQueryObjectivARB(id_tmp, GL_QUERY_RESULT_AVAILABLE_ARB, &param);

	if (glGetError() != GL_INVALID_OPERATION) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
			"GetQueryObjectuiv can still query the result"
			"by an unused query id. \n");
		return false;
	}

	return true;
}


/* Basic tests on query id generation and deletion */
bool
conformOQ_Gen_Delete(unsigned int id_n)
{
	GLuint *ids1 = NULL, *ids2 = NULL;
	unsigned int i, j;
	bool pass = true;

	ids1 = (GLuint *)malloc(id_n * sizeof(GLuint));
	ids2 = (GLuint *)malloc(id_n * sizeof(GLuint));

	if (!ids1 || !ids2) {
		printf(" Error: Cannot alloc memory to pointer ids[12].\n");
		if (ids1)
			free(ids1);
		if (ids2)
			free(ids2);		
		return false;
	}

	glGenQueriesARB(id_n, ids1);
	glGenQueriesARB(id_n, ids2);

	/* compare whether <id> generated during the previous 2 rounds are
	 * duplicated */
	for (i = 0; i < id_n; i ++) {
		for (j = 0; j < id_n; j ++) {
			if (ids1[i] == ids2[j]) {
				char str[1000];
				sprintf(str, "ids1[%d] == ids2[%d] == %u.", i, j, ids1[i]);
				printf(" Error:  %s\n", str);
				pass = false;
			}
		}
	}

	/* Note: the spec seems to indicate that glGenQueries reserves query
	 * IDs but doesn't create query objects for those IDs.  A query object
	 * isn't created until they are used by glBeginQuery.  So this part
	 * of the test is invalid.
	 */
#if 0
	/* Checkout whether the Query ID just generated is valid */	
	for (i = 0; i < id_n; i ++) {
		if (glIsQueryARB(ids1[i]) == GL_FALSE) {
			char str[1000];
			sprintf(str, "id [%d] just generated is not valid.", ids1[i]);
			printf(" Error: %s\n", str);
			pass = false;
		}
	}
#endif

	/* if <id> is a non-zero value that is not the name of a query object,
	 * IsQueryARB returns FALSE. */
	glDeleteQueriesARB(id_n, ids1);
	for (i = 0; i < id_n; i ++) {
		if (glIsQueryARB(ids1[i]) == GL_TRUE) {
			char str[1000];
			sprintf(str, "id [%d] just deleted is still valid.", ids1[i]);
			printf(" Error: %s\n", str);
			pass = false;
		}
	}

	/* Delete only for sanity purpose */
	glDeleteQueriesARB(id_n, ids2);

	if (ids1)
		free(ids1);
	if (ids2)
		free(ids2);


	ids1 = (GLuint *)malloc(id_n * sizeof(GLuint));
	if (ids1 == NULL)
		return false;

	for (i = 0; i < id_n; i ++) {
		glGenQueriesARB(1, ids1 + i);
		for (j = 0; j < i; j ++) {
			if (ids1[i] == ids1[j]) {
				char str[1000];
				sprintf(str, "duplicated id generated [%u]", ids1[i]);
				printf(" Error: %s\n", str);
				pass = false;
			}
		}
	}

	glDeleteQueriesARB(id_n, ids1);
	if (ids1)
		free(ids1);

	return pass;
}


/* If <id> is zero, IsQueryARB should return FALSE.*/
bool
conformOQ_IsIdZero(void)
{
	if (glIsQueryARB(0) == GL_TRUE) {
		printf(" Error: zero is treated as a valid id by glIsQueryARB().\n");
		return false;
	}
		
	return true;
}


/* If BeginQueryARB is called with an <id> of zero, an INVALID_OPERATION error
 * should be generated. */
bool
conformOQ_BeginIdZero(void)
{
	glBeginQueryARB(GL_SAMPLES_PASSED_ARB, 0);
	if (glGetError() != GL_INVALID_OPERATION) {
		printf(" Error: No GL_INVALID_OPERATION generated if "
			"BeginQuery with zero ID.");
		return false;
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
	glClearColor(0.0, 0.2, 0.3, 0.0);
	glClearDepth(1.0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	piglit_require_extension("GL_ARB_occlusion_query");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLuint queryId;

	if (!chk_ext())
		return PIGLIT_FAIL;
	setup();
	glEnable(GL_DEPTH_TEST);
	glGenQueriesARB(1, &queryId);

	if (queryId == 0)
		return PIGLIT_FAIL;

#if defined(GL_ARB_occlusion_query)
	pass &= conformOQ_GetQry_CnterBit();
	pass &= conformOQ_GetObjivAval_multi1(queryId);
	pass &= conformOQ_GetObjivAval_multi2();
	pass &= conformOQ_Begin_unused_id();
	pass &= conformOQ_EndAfter(queryId);
	pass &= conformOQ_GenIn(queryId);
	pass &= conformOQ_BeginIn(queryId);
	pass &= conformOQ_DeleteIn(queryId);
	pass &= conformOQ_GetObjAvalIn(queryId);
	pass &= conformOQ_GetObjResultIn(queryId);
	pass &= conformOQ_GetObjivAval(queryId);
	pass &= conformOQ_Gen_Delete(64);
	pass &= conformOQ_IsIdZero();
	pass &= conformOQ_BeginIdZero();
	glDeleteQueriesARB(1, &queryId);

	piglit_present_results();
#endif

	return pass ?  PIGLIT_PASS: PIGLIT_FAIL;
}
