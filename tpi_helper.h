
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <semaphore.h>

#define EGL_DUMMY 1

#include "GLES3/gl32.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
#include <drm_fourcc.h>



/* ------------------------------------------------------------------------- */
size_t tpi_fsize( FILE *stream )
{
	long int saved_pointer;
	long int stream_size;
	int  error;

	saved_pointer = ftell( stream );
	if( -1 == saved_pointer )
	{
		return 0;
	}

	error = fseek( stream, 0, SEEK_END );
	if( 0 != error )
	{
		return 0;
	}

	stream_size = ftell( stream );
	if( -1 == stream_size )
	{
		return 0;
	}

	fseek( stream, saved_pointer, SEEK_SET );

	return (size_t) stream_size;
}


/* ============================================================================
	Load Functions
============================================================================ */

void *gles_api_load_file(const char *filename, int *len)
{
	FILE *fstream;
	void *retval = NULL;
	size_t elements_read = 0;

	if ((NULL == filename) || (NULL == len))
	{
        printf("nothing to read\n");
		return NULL;
	}

	*len = 0;

	fstream = fopen(filename, "rb");
	if (NULL == fstream)
	{
		printf("load_file: failed to open %s\r\n", filename);
		return NULL;
	}

	*len = (int)tpi_fsize(fstream);
	if (0 < *len)
	{
		retval = malloc((unsigned int)*len + 1);
		if (NULL != retval)
		{
			memset(retval, 0x0, (unsigned int)*len + 1);
			elements_read = fread(retval, 1, (unsigned int)*len, fstream);
		}
	}

	fclose(fstream);

	if ((int)elements_read != *len)
	{
		printf("load_file: expected to read %d bytes but got %zu\r\n", *len, elements_read);
		if (NULL != retval)
		{
			free(retval);
		}
		return NULL;
	}

    //printf("read %d bytes from file %s\n",elements_read, filename );
	return retval;
}

#define GLES2_API_BUFFERSIZE 4096

GLuint gles_api_create_program_with_file(const char*vs, const char* fs)
{
    GLint compiled;
    GLchar *shadertext;
    GLsizei len = 0, rellen;
    GLuint program = 0;
	GLint linked = GL_FALSE;
    char* buffer = (char*)malloc(GLES2_API_BUFFERSIZE);

    GLuint vsid = glCreateShader(GL_VERTEX_SHADER);
    //printf("created vs:%d \n",vsid);

    void *vsdata = gles_api_load_file(vs, &len);
    if(!vsdata | !len)
    {
        printf("can't load shader file %s\n",vs);
    }
    shadertext = (GLchar*) vsdata;
    //shadertext[len] = 0;
    glShaderSource(vsid, 1, (const GLchar **)&shadertext, NULL);
    glCompileShader(vsid);
    glGetShaderiv(vsid, GL_COMPILE_STATUS, &compiled);
    if (GL_FALSE == compiled)
    {
        printf("gles_api_create_program_with_file: Unable to compile vertex shader '%s' \n", shadertext);
        glGetShaderInfoLog(vsid, GLES2_API_BUFFERSIZE - 1, NULL, buffer);
        printf(" - reason:\n%s\n\n", buffer);
        glDeleteShader(vsid);
    }

    GLuint fsid = glCreateShader(GL_FRAGMENT_SHADER);
    //printf("created fs:%d \n",fsid);

    void *fsdata = gles_api_load_file(fs, &len);
    if(!fsdata | !len)
    {
        printf("can't load shader file %s\n",fs);
    }
    shadertext = (GLchar*) fsdata;
    //shadertext[len] = 0;
    glShaderSource(fsid, 1, (const GLchar **)&shadertext, NULL);
    glCompileShader(fsid);
    glGetShaderiv(fsid, GL_COMPILE_STATUS, &compiled);
    if (GL_FALSE == compiled)
    {
        printf("gles_api_create_program_with_file: Unable to compile fragment shader '%s' \n", shadertext);
        glGetShaderInfoLog(fsid, GLES2_API_BUFFERSIZE - 1, NULL, buffer);
        printf(" - reason:\n%s\n\n", buffer);
        glDeleteShader(fsid);
    }

    program = glCreateProgram();

    glAttachShader(program, vsid);
    glAttachShader(program, fsid);
    glLinkProgram(program);

    /* Check the link status */
	glGetProgramiv(program, GL_LINK_STATUS, &linked);

    /* If it failed */
	if (GL_FALSE == linked)
	{
		buffer[0] = '\0';
		glGetProgramInfoLog(program, GLES2_API_BUFFERSIZE - 1, NULL, buffer);
        printf("gles_api_create_program_with_file: Unable to link program - reason:\n%s\n", buffer);
        program = 0;
    }

    glDeleteShader(vsid);
    glDeleteShader(fsid);
    free(vsdata);
    free(fsdata);
    return program;
}

#define PRIMARY_DISPLAY_WIDTH 2560
#define PRIMARY_DISPLAY_HEIGHT 1620
#define PRIMARY_DISPLAY_BPP 4
#define PRIMARY_DISPLAY_FORMAT MALI_TPI_FORMAT_A8B8G8R8

static unsigned char primary_display_front_buffer[PRIMARY_DISPLAY_WIDTH *
                                                  PRIMARY_DISPLAY_HEIGHT *
                                                  PRIMARY_DISPLAY_BPP * 8];
dummy_display primary_display;
EGLDisplay dpy;
EGLContext ctx;
EGLSurface surf;
EGLConfig config;
EGLNativeWindowType window ;

EGLBoolean mali_tpi_egl_choose_config(EGLDisplay    display,
                                        EGLint const *min_attribs,
                                        EGLint const *exact_attribs,
                                        EGLConfig    *configs,
                                        EGLint        config_size,
                                        EGLint       *num_config)
{
	EGLint i;
	EGLint matched;
	EGLint num_min_config = 0;  /* Number of configs matching min_attribs */
	EGLConfig *min_configs = NULL; /* Configs matching min_attribs */
	static const EGLint dummy[] = { EGL_NONE };

	if( NULL == num_config )
	{
		/* For eglChooseConfig, this is an EGL_BAD_PARAMETER error.
		 * Create one ourselves.
		 */
		return eglChooseConfig(
			display, min_attribs, configs, config_size, num_config );
	}

	if( !eglChooseConfig( display, min_attribs, NULL, 0, &num_min_config ) )
	{
		return EGL_FALSE;
	}

	min_configs = (EGLConfig *) malloc( num_min_config * sizeof(EGLConfig) );
	if( NULL == min_configs )
	{
		/* Out of memory */
		return EGL_FALSE;
	}

	/* Obtain everything that matches min_attribs */
	if ( !eglChooseConfig(
		display, min_attribs, min_configs, num_min_config, &num_min_config) )
	{
		free( min_configs );
		return EGL_FALSE;
	}

	/* Filter by exact_attribs, in-place so as not to
	 * alter the outputs if an error occurs later.
	 */
	if( NULL == exact_attribs )
	{
		exact_attribs = dummy;
	}

	matched = 0;
	for( i = 0; i < num_min_config; i++ )
	{
		EGLint j;
		EGLBoolean match = EGL_TRUE;
		for( j = 0; match && (exact_attribs[j] != EGL_NONE); j += 2 )
		{
			EGLint attrib_value;

			if( (exact_attribs[j + 1] == EGL_DONT_CARE) &&
				(exact_attribs[j]     != EGL_LEVEL) )
			{
				/* Skip don't-cares; but it's not valid value for EGL_LEVEL */
				continue;
			}

			if( !eglGetConfigAttrib(
				display, min_configs[i], exact_attribs[j], &attrib_value ) )
			{
				free( min_configs );
				return EGL_FALSE;
			}

			if( attrib_value != exact_attribs[j + 1] )
			{
				match = EGL_FALSE;
			}
		}
		if( match )
		{
			min_configs[matched++] = min_configs[i];
		}
	}

	/* min_configs now holds the filtered results. Output them if necessary */
	if( NULL != configs )
	{
		if( config_size < matched )
		{
			/* This also applies the required clamping for the return value */
			matched = config_size;
		}
		memcpy( configs, min_configs, matched * sizeof(EGLConfig) );
	}

	*num_config = matched;
	free( min_configs );
	return EGL_TRUE;
}

void egl_init()
{
    memset(&primary_display, 0x0, sizeof(primary_display));
    primary_display.format.fourcc = DRM_FORMAT_ABGR8888;
    /* Set-up the primary display to defaults*/
    primary_display.width = PRIMARY_DISPLAY_WIDTH;
    primary_display.height = PRIMARY_DISPLAY_HEIGHT;
    primary_display.planes[0].byte_stride = PRIMARY_DISPLAY_BPP * primary_display.width;
    primary_display.planes[0].data = primary_display_front_buffer;
    primary_display.planes[1].byte_stride = 0;
    primary_display.planes[1].data = NULL;
    primary_display.planes[2].byte_stride = 0;
    primary_display.planes[2].data = NULL;

    dpy = eglGetDisplay(&primary_display);
    if (EGL_NO_DISPLAY == dpy)
	{
        printf("Can't get display\n");
    }

    if(!eglInitialize(dpy,NULL,NULL))
    {
        printf("eglInitialize failed\n");
    };

    EGLint config_min_attribs[] = { EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_SURFACE_TYPE,
		                            EGL_DONT_CARE, /* allows window, pixmap, and pbuffer surfaces */
		                            EGL_COLOR_COMPONENT_TYPE_EXT, EGL_COLOR_COMPONENT_TYPE_FIXED_EXT, EGL_NONE };

	EGLint config_exact_attribs[] = { EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
		                              EGL_DEPTH_SIZE, 24, EGL_STENCIL_SIZE, 8, EGL_SAMPLES, 0, EGL_NONE };

	EGLint version_context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 0, EGL_NONE };

    EGLint num_config;

    if (!mali_tpi_egl_choose_config(dpy, config_min_attribs, config_exact_attribs, &config, 1,
	                                &num_config))
	{
		printf("EGL config selection failed.\n");
    }
    if (num_config < 1)
	{
		printf("No matching EGL config found.\n");
		return;
	}

    EGLint surface_type;
    if (!eglGetConfigAttrib(dpy, config, EGL_SURFACE_TYPE, &surface_type))
	{
		printf("EGL config query failed.\n");
		return;
	}

    
    fbdev_window *win;
    win = (fbdev_window *)malloc(sizeof(*win));
    win->width = 1024;
	win->height = 1024;//Fred-Todo

    window = win;
    int setenv_ret;
    setenv_ret = setenv("MALI_EGL_DUMMY_WINDOW_FORMAT", "EGL_COLOR_BUFFER_FORMAT_ABGR8888", 1);
    if( 0 != setenv_ret )
    {
        printf("setenv MALI_EGL_DUMMY_WINDOW_FORMAT failed\n");
    }

    surf = eglCreateWindowSurface(dpy, config, window, NULL);
    if(!surf)
    {
        printf("eglSurface create failed\n");
    }

    // EGLBoolean res;
    // res = eglBindAPI(EGL_OPENGL_ES_API);

    EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    ctx = eglCreateContext(dpy, config, NULL, context_attribs);
    if(ctx == NULL)
    {
        printf("eglContext create failed\n");
    }

    if(!eglMakeCurrent(dpy, surf, surf, ctx))
    {
        printf("EGL switch current failed\n");
    }
    
}