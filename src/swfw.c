/*
Copyright (C) 2017 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to3
the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
     that you wrote the original software. If you use this software in a
     product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "swfw.h"

#ifdef SWFW_WAYLAND
	#include <syscall.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/mman.h>
#endif

#define DEFAULT_HINT_USE_HARDWARE_ACCELERATION true
#define DEFAULT_HINT_SIZE_WIDTH 256
#define DEFAULT_HINT_SIZE_HEIGHT 256

#ifdef SWFW_EGL
static enum swfw_status swfw_egl_get_config(struct swfw_context_egl *swfw_ctx_egl)
{
	enum swfw_status status = SWFW_OK;
	EGLConfig egl_conf = {0};
	EGLConfig *configs = NULL;
	EGLint egl_num_configs = 0;
	EGLint val_EGL_SURFACE_TYPE = 0;
	EGLint val_EGL_RENDERABLE_TYPE = 0;
	EGLint val_EGL_COLOR_BUFFER_TYPE = 0;
	EGLint val_EGL_RED_SIZE = 0;
	EGLint val_EGL_GREEN_SIZE = 0;
	EGLint val_EGL_BLUE_SIZE = 0;
	EGLint val_EGL_ALPHA_SIZE = 0;
	EGLint val_EGL_DEPTH_SIZE = 0;
	EGLBoolean result = false;
	EGLint i = 0;
	result = eglGetConfigs(swfw_ctx_egl->display, NULL, 0, &egl_num_configs);
	if (result != EGL_TRUE) {
		status = SWFW_ERROR;
		goto done;
	}
	if (egl_num_configs == 0) {
		status = SWFW_ERROR;
		goto done;
	}
	configs = malloc(egl_num_configs * sizeof(*configs));
	if (configs == NULL) {
		status = SWFW_ERROR;
		goto done;
	}
	result = eglGetConfigs(swfw_ctx_egl->display, configs, egl_num_configs, &egl_num_configs);
	if (result != EGL_TRUE) {
		free(configs);
		status = SWFW_ERROR;
		goto done;
	}
	status = SWFW_ERROR;
	while (i < egl_num_configs) {
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_SURFACE_TYPE, &val_EGL_SURFACE_TYPE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_COLOR_BUFFER_TYPE, &val_EGL_COLOR_BUFFER_TYPE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_RENDERABLE_TYPE, &val_EGL_RENDERABLE_TYPE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_RED_SIZE, &val_EGL_RED_SIZE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_GREEN_SIZE, &val_EGL_GREEN_SIZE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_BLUE_SIZE, &val_EGL_BLUE_SIZE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_ALPHA_SIZE, &val_EGL_ALPHA_SIZE);
		eglGetConfigAttrib(swfw_ctx_egl->display, configs[i], EGL_DEPTH_SIZE, &val_EGL_DEPTH_SIZE);
		if (val_EGL_SURFACE_TYPE & EGL_WINDOW_BIT &&
		val_EGL_COLOR_BUFFER_TYPE & EGL_RGB_BUFFER &&
		val_EGL_RENDERABLE_TYPE & EGL_OPENGL_ES2_BIT &&
		val_EGL_RED_SIZE == 8 &&
		val_EGL_GREEN_SIZE == 8 &&
		val_EGL_BLUE_SIZE == 8 &&
		val_EGL_ALPHA_SIZE == 8 &&
		val_EGL_DEPTH_SIZE != 0) {
			egl_conf = configs[i];
			i = egl_num_configs;
			status = SWFW_OK;
		}
		i++;
	}
	free(configs);
	swfw_ctx_egl->config = egl_conf;
done:
	return status;
}

static enum swfw_status swfw_egl_swap_interval(struct swfw_context_egl *swfw_ctx_egl, int32_t interval)
{
	enum swfw_status status = SWFW_OK;
	eglSwapInterval(swfw_ctx_egl->display, interval);
	if (eglGetError() != EGL_SUCCESS) {
		status = SWFW_ERROR;
	}
	return status;
}

static enum swfw_status swfw_egl_swap_buffers(struct swfw_context_egl *swfw_ctx_egl)
{
	enum swfw_status status = SWFW_OK;
	eglSwapBuffers(swfw_ctx_egl->display, swfw_ctx_egl->surface);
	if (eglGetError() != EGL_SUCCESS) {
		status = SWFW_ERROR;
	}
	return status;
}

static enum swfw_status initialize_egl(struct swfw_context_egl *swfw_ctx_egl, void *native_display)
{
	enum swfw_status status = SWFW_OK;
	swfw_ctx_egl->display = eglGetDisplay(native_display);
	if (swfw_ctx_egl->display == EGL_NO_DISPLAY) {
		status = SWFW_ERROR;
	} else {
		if (eglInitialize(swfw_ctx_egl->display, &swfw_ctx_egl->major, &swfw_ctx_egl->minor) != EGL_TRUE) {
			status = SWFW_ERROR;
		}
	}
	return status;
}

static enum swfw_status swfw_egl_create_context(struct swfw_context_egl *swfw_ctx_egl)
{
	enum swfw_status status = SWFW_OK;
	EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	if (!eglBindAPI(EGL_OPENGL_API)) {
		status = SWFW_ERROR;
		goto done;
	}
	swfw_ctx_egl->context = eglCreateContext(swfw_ctx_egl->display,
		swfw_ctx_egl->config,
		EGL_NO_CONTEXT,
		context_attribs);
	if (swfw_ctx_egl->context == EGL_NO_CONTEXT) {
		status = SWFW_ERROR;
	}
done:
	return status;
}

static enum swfw_status swfw_egl_create_surface(struct swfw_context_egl *swfw_ctx_egl, EGLNativeWindowType native_window)
{
	enum swfw_status status = SWFW_OK;
	EGLint egl_surf_attr[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE
	};
	swfw_ctx_egl->surface = eglCreateWindowSurface(swfw_ctx_egl->display,
		swfw_ctx_egl->config,
		native_window,
		egl_surf_attr);
	if (swfw_ctx_egl->surface == EGL_NO_SURFACE) {
		status = SWFW_ERROR;
	}
	return status;
}

static enum swfw_status swfw_egl_make_current(struct swfw_context_egl *swfw_ctx_egl)
{
	enum swfw_status status = SWFW_OK;
	if (eglMakeCurrent(swfw_ctx_egl->display, swfw_ctx_egl->surface, swfw_ctx_egl->surface, swfw_ctx_egl->context) == EGL_FALSE) {
		status = SWFW_ERROR;
	}
	return status;
}
#endif

#ifdef SWFW_X11
#ifndef _NET_WM_MOVERESIZE
#define _NET_WM_MOVERESIZE 8
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_TOPLEFT
#define _NET_WM_MOVERESIZE_SIZE_TOPLEFT 0
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_TOP
#define _NET_WM_MOVERESIZE_SIZE_TOP 1
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_TOPRIGHT
#define _NET_WM_MOVERESIZE_SIZE_TOPRIGHT 2
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_RIGHT
#define _NET_WM_MOVERESIZE_SIZE_RIGHT 3
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT 4
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_BOTTOM
#define _NET_WM_MOVERESIZE_SIZE_BOTTOM 5
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT
#define _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT 6
#endif
#ifndef _NET_WM_MOVERESIZE_SIZE_LEFT
#define _NET_WM_MOVERESIZE_SIZE_LEFT 7
#endif
#ifndef Button6
#define Button6 6
#endif
#ifndef Button7
#define Button7 7
#endif

struct x11_hints {
	uint64_t flags;
	uint64_t functions;
	uint64_t decorations;
	int64_t input_mode;
	uint64_t status;
};

enum swfw_status swfw_drag_window_x11(struct swfw_window_x11 *swfw_win_x11)
{
	Window child;
	Window root;
	int32_t window_x = 0;
	int32_t window_y = 0;
	int32_t root_x = 0;
	int32_t root_y = 0;
	int32_t cursor_x = 0;
	int32_t cursor_y = 0;
	unsigned int mask = 0;
	XClientMessageEvent xclient = {0};
	XUngrabPointer(swfw_win_x11->swfw_ctx_x11->display, 0);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	XTranslateCoordinates(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		swfw_win_x11->swfw_ctx_x11->root,
		0, 0,
		&window_x, &window_y,
		&child);
	XQueryPointer(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		&root, &child,
		&root_x, &root_y, &cursor_x, &cursor_y,
		&mask);
	xclient.type = ClientMessage;
	xclient.window = swfw_win_x11->window;
	xclient.message_type = swfw_win_x11->swfw_ctx_x11->atom_NET_WM_MOVERESIZE;
	xclient.format = 32;
	xclient.data.l[0] = window_x + cursor_x;
	xclient.data.l[1] = window_y + cursor_y;
	xclient.data.l[2] = _NET_WM_MOVERESIZE;
	xclient.data.l[3] = 0;
	xclient.data.l[4] = 0;
	XSendEvent(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->swfw_ctx_x11->root,
		False,
		SubstructureRedirectMask | SubstructureNotifyMask,
		(XEvent *)&xclient);
	return SWFW_OK;
}

enum swfw_status swfw_resize_window_x11(struct swfw_window_x11 *swfw_win_x11, enum swfw_window_border window_border)
{
	enum swfw_status status = SWFW_OK;
	Window child;
	Window root;
	int32_t window_x = 0;
	int32_t window_y = 0;
	int32_t root_x = 0;
	int32_t root_y = 0;
	int32_t cursor_x = 0.0;
	int32_t cursor_y = 0.0;
	unsigned int mask = 0;
	XClientMessageEvent xclient = {0};
	XUngrabPointer(swfw_win_x11->swfw_ctx_x11->display, 0);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	XTranslateCoordinates(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		swfw_win_x11->swfw_ctx_x11->root,
		0, 0,
		&window_x, &window_y,
		&child);
	XQueryPointer(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		&root, &child,
		&root_x, &root_y, &cursor_x, &cursor_y,
		&mask);
	xclient.type = ClientMessage;
	xclient.window = swfw_win_x11->window;
	xclient.message_type = swfw_win_x11->swfw_ctx_x11->atom_NET_WM_MOVERESIZE;
	xclient.format = 32;
	xclient.data.l[0] = window_x + cursor_x;
	xclient.data.l[1] = window_y + cursor_y;
	xclient.data.l[3] = 0;
	xclient.data.l[4] = 0;
	if (window_border == SWFW_WINDOW_BORDER_LEFT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_LEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_TOP;
	} else if (window_border == SWFW_WINDOW_BORDER_RIGHT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_RIGHT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_BOTTOM;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP_LEFT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_TOPLEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP_RIGHT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_TOPRIGHT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM_LEFT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM_RIGHT) {
		xclient.data.l[2] = _NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT;
	} else {
		status = SWFW_ERROR;
	}
	if (status == SWFW_OK) {
		XSendEvent(swfw_win_x11->swfw_ctx_x11->display,
			swfw_win_x11->swfw_ctx_x11->root,
			False,
			SubstructureRedirectMask | SubstructureNotifyMask,
			(XEvent *)&xclient);
	}
	return status;
}

enum swfw_status swfw_hide_window_x11(struct swfw_window_x11 *swfw_win_x11)
{
	XUnmapWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_show_window_x11(struct swfw_window_x11 *swfw_win_x11)
{
	XMapWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_get_window_work_area_x11(struct swfw_window_x11 *swfw_win_x11, int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
	Atom *extents = NULL;
	Atom actual_type;
	int32_t actual_format = 0;
	uint64_t item_count = 0;
	uint64_t bytes_after = 0;
	XGetWindowProperty(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->swfw_ctx_x11->root,
		swfw_win_x11->swfw_ctx_x11->atom_NET_WORKAREA,
		0,
		4 * sizeof(uint64_t),
		False,
		AnyPropertyType,
		&actual_type,
		&actual_format,
		&item_count,
		&bytes_after,
		(uint8_t **)&extents);
	if (extents) {
		*x = extents[0];
		*y = extents[1];
		*width = extents[2];
		*height = extents[3];
		XFree(extents);
	}
	return SWFW_OK;
}

enum swfw_status swfw_set_window_position_x11(struct swfw_window_x11 *swfw_win_x11, int32_t x, int32_t y)
{
	XMoveWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, x, y);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_set_window_size_x11(struct swfw_window_x11 *swfw_win_x11, int32_t width, int32_t height)
{
	XResizeWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, width, height);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_set_window_size_limits_x11(struct swfw_window_x11 *swfw_win_x11, int32_t min_width, int32_t min_height, int32_t max_width, int32_t max_height)
{
	XWindowAttributes attribs = {0};
	XSizeHints size_hints = {0};
	size_hints.flags = PMinSize | PMaxSize;
	XGetWindowAttributes(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		&attribs);
	if (min_width > 0) {
		size_hints.min_width = min_width;
	}
	if (min_height > 0) {
		size_hints.min_height = min_height;
	}
	if (min_width > 0) {
		size_hints.max_width = max_width;
	}
	if (min_width > 0) {
		size_hints.max_height = max_height;
	}
	XSetWMNormalHints(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, &size_hints);
	return SWFW_OK;
}

enum swfw_status swfw_set_window_resizable_x11(struct swfw_window_x11 *swfw_win_x11, bool resizable)
{
	XWindowAttributes attribs = {0};
	XSizeHints size_hints = {0};
	size_hints.flags = PMinSize | PMaxSize;
	XGetWindowAttributes(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, &attribs);
	if (resizable) {
		size_hints.min_width = 0;
		size_hints.min_height = 0;
		size_hints.max_width = INT32_MAX;
		size_hints.max_height = INT32_MAX;
	} else {
		size_hints.min_width = attribs.width;
		size_hints.min_height = attribs.height;
		size_hints.max_width = attribs.width;
		size_hints.max_height = attribs.height;
	}
	XSetWMNormalHints(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, &size_hints);
	return SWFW_OK;
}

enum swfw_status swfw_set_window_decorated_x11(struct swfw_window_x11 *swfw_win_x11, bool decorated)
{
	struct x11_hints hints = {0};
	if (decorated) {
		XDeleteProperty(swfw_win_x11->swfw_ctx_x11->display,
			swfw_win_x11->window,
			swfw_win_x11->swfw_ctx_x11->atom_MOTIF_WM_HINTS);
	} else if (swfw_win_x11->swfw_ctx_x11->atom_MOTIF_WM_HINTS != None) {
		hints.flags = 2;
		hints.decorations = 0;
		XChangeProperty(swfw_win_x11->swfw_ctx_x11->display,
			swfw_win_x11->window,
			swfw_win_x11->swfw_ctx_x11->atom_MOTIF_WM_HINTS,
			swfw_win_x11->swfw_ctx_x11->atom_MOTIF_WM_HINTS,
			32,
			PropModeReplace,
			(uint8_t *)&hints,
			sizeof(struct x11_hints));
		XFlush(swfw_win_x11->swfw_ctx_x11->display);
	}
	return SWFW_OK;
}

enum swfw_status swfw_set_window_title_x11(struct swfw_window_x11 *swfw_win_x11, char *title)
{
	XStoreName(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window, title);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_window_swap_interval_x11(struct swfw_window_x11 *swfw_win_x11, int32_t interval)
{
	enum swfw_status status = SWFW_OK;
#ifdef SWFW_EGL
	status = swfw_egl_swap_interval(&swfw_win_x11->swfw_ctx_egl, interval);
#endif
	return status;
}

enum swfw_status swfw_window_swap_buffers_x11(struct swfw_window_x11 *swfw_win_x11)
{
	enum swfw_status status = SWFW_OK;
#ifdef SWFW_EGL
	status = swfw_egl_swap_buffers(&swfw_win_x11->swfw_ctx_egl);
#endif
	return status;
}

enum swfw_status swfw_destroy_window_x11(struct swfw_window_x11 *swfw_win_x11)
{
	XUnmapWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window);
	XDestroyWindow(swfw_win_x11->swfw_ctx_x11->display, swfw_win_x11->window);
	XFlush(swfw_win_x11->swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_make_window_x11(struct swfw_context_x11 *swfw_ctx_x11, struct swfw_window_x11 *swfw_win_x11, struct swfw_hints hints)
{
	enum swfw_status status = SWFW_OK;
	swfw_win_x11->swfw_ctx_x11 = swfw_ctx_x11;
	swfw_win_x11->window = XCreateWindow(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->swfw_ctx_x11->root,
		hints.x, hints.y,
		hints.width, hints.height,
		0,
		swfw_win_x11->swfw_ctx_x11->depth,
		InputOutput,
		swfw_win_x11->swfw_ctx_x11->visual,
		swfw_win_x11->swfw_ctx_x11->mask,
		&swfw_win_x11->swfw_ctx_x11->attributes);
	if (swfw_win_x11->window) {
		XSetWMProtocols(swfw_win_x11->swfw_ctx_x11->display,
			swfw_win_x11->window,
			&swfw_win_x11->swfw_ctx_x11->atom_WM_DELETE_WINDOW,
			1);
		XFlush(swfw_win_x11->swfw_ctx_x11->display);
	} else {
		status = SWFW_ERROR;
	}
	if (swfw_win_x11->swfw_ctx_x11->im) {
		swfw_win_x11->ic = XCreateIC(swfw_win_x11->swfw_ctx_x11->im, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, swfw_win_x11->window, NULL);
		if (swfw_win_x11->ic) {
			XSetICFocus(swfw_win_x11->ic);
		}
	}
	XSaveContext(swfw_win_x11->swfw_ctx_x11->display,
		swfw_win_x11->window,
		swfw_win_x11->swfw_ctx_x11->context,
		(char *)swfw_win_x11);
	swfw_win_x11->use_hardware_acceleration = hints.use_hardware_acceleration;
#ifdef SWFW_EGL
	if (swfw_win_x11->use_hardware_acceleration) {
		if (initialize_egl(&swfw_win_x11->swfw_ctx_egl, swfw_ctx_x11->display) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_get_config(&swfw_win_x11->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_create_context(&swfw_win_x11->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_create_surface(&swfw_win_x11->swfw_ctx_egl, (EGLNativeWindowType)swfw_win_x11->window) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_make_current(&swfw_win_x11->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		if (swfw_window_swap_buffers_x11(swfw_win_x11) != SWFW_OK) {
			abort();
		}
	}
#endif
	return status;
}

enum swfw_status swfw_get_screen_size_x11(struct swfw_context_x11 *swfw_ctx_x11, int32_t i, int32_t *width, int32_t *height)
{
	enum swfw_status status = SWFW_OK;
	if (i > XScreenCount(swfw_ctx_x11->display)) {
		status = SWFW_ERROR;
		goto done;
	}
	*width = XWidthOfScreen(swfw_ctx_x11->screen);
	*height = XHeightOfScreen(swfw_ctx_x11->screen);
done:
	return status;
}

enum swfw_status swfw_get_screens_x11(struct swfw_context_x11 *swfw_ctx_x11, int32_t *i)
{
	*i = XScreenCount(swfw_ctx_x11->display);
	return SWFW_OK;
}

bool swfw_poll_event_x11(struct swfw_context_x11 *swfw_ctx_x11, struct swfw_event *event)
{
	struct swfw_event e = {0};
	XEvent x11_event = {0};
	bool has_event = false;
	struct swfw_window_x11 *swfw_win_x11 = NULL;
	bool event_filtered = false;
	if (XPending(swfw_ctx_x11->display) > 0) {
		XNextEvent(swfw_ctx_x11->display, &x11_event);
		if (XFilterEvent(&x11_event, None)) {
			event_filtered = true;
		}
		if (XFindContext(swfw_ctx_x11->display, x11_event.xany.window, swfw_ctx_x11->context, (char **)&swfw_win_x11) != 0) {
			abort();
		}
		if (x11_event.type == Expose) {
			e.type = SWFW_EVENT_EXPOSE;
		} else if (x11_event.type == KeyPress) {
			KeySym keysym = 0;
			Status status = 0;
			e.type = SWFW_EVENT_KEY_PRESS;
			e.key_code = x11_event.xkey.keycode;
			if (swfw_win_x11->ic) {
				e.string_length = Xutf8LookupString(swfw_win_x11->ic, &x11_event.xkey, e.string, 4, &keysym, &status);
				if ((status == XLookupChars || status == XLookupBoth) && status != XBufferOverflow) {
					if (event_filtered) {
						e.string_length = 0;
					}
				} else {
					e.string_length = 0;
				}
				e.key_sym = keysym;
			}
		} else if (x11_event.type == KeyRelease) {
			e.type = SWFW_EVENT_KEY_RELEASE;
			e.key_code = x11_event.xkey.keycode;
		} else if (x11_event.type == MotionNotify) {
			e.type = SWFW_EVENT_CURSOR_MOTION;
			e.x = x11_event.xmotion.x;
			e.y = x11_event.xmotion.y;
		} else if (x11_event.type == ButtonPress || x11_event.type == ButtonRelease) {
			if (x11_event.xbutton.button == Button1 || x11_event.xbutton.button == Button2 || x11_event.xbutton.button == Button3) {
				if (x11_event.type == ButtonPress) {
					e.type = SWFW_EVENT_BUTTON_PRESS;
				} else if (x11_event.type == ButtonRelease) {
					e.type = SWFW_EVENT_BUTTON_RELEASE;
				}
				e.button = x11_event.xbutton.button;
			} else {
				e.type = SWFW_EVENT_SCROLL;
				if (x11_event.xbutton.button == Button4) {
					e.axis = 0;
					e.scroll = -1;
				}
				if (x11_event.xbutton.button == Button5) {
					e.axis = 0;
					e.scroll = 1;
				}
				if (x11_event.xbutton.button == Button6) {
					e.axis = 1;
					e.scroll = -1;
				}
				if (x11_event.xbutton.button == Button7) {
					e.axis = 1;
					e.scroll = 1;
				}
			}
		} else if (x11_event.type == EnterNotify) {
			e.type = SWFW_EVENT_CURSOR_ENTER;
		} else if (x11_event.type == LeaveNotify) {
			e.type = SWFW_EVENT_CURSOR_LEAVE;
		} else if (x11_event.type == ConfigureNotify) {
			e.type = SWFW_EVENT_CONFIGURE;
		} else if (x11_event.type == MapNotify) {
			e.type = SWFW_EVENT_MAP;
		} else if (x11_event.type == UnmapNotify) {
			e.type = SWFW_EVENT_UNMAP;
		} else if (x11_event.type == DestroyNotify) {
			e.type = SWFW_EVENT_DESTROY;
		} else if (x11_event.type == ClientMessage) {
			if(x11_event.xclient.data.l[0] == swfw_ctx_x11->atom_WM_DELETE_WINDOW) {
				e.type = SWFW_EVENT_DESTROY;
			}
		}
		if (e.type != 0) {
			has_event = true;
		}
	}
	*event = e;
	return has_event;
}

enum swfw_status swfw_destroy_context_x11(struct swfw_context_x11 *swfw_ctx_x11)
{
	XCloseDisplay(swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_make_context_x11(struct swfw_context_x11 *swfw_ctx_x11)
{
	enum swfw_status status = SWFW_OK;
	bool locale_ok = true;
	if (setlocale(LC_ALL, "") == NULL) {
		locale_ok = false;
	}
	if (locale_ok) {
		if (!XSupportsLocale()) {
			locale_ok = false;
		}
	}
	if (locale_ok) {
		if (XSetLocaleModifiers("@im=none") == NULL) {
			locale_ok = false;
		}
	}
	/* Display */
	swfw_ctx_x11->display = XOpenDisplay(NULL);
	swfw_ctx_x11->screen = XDefaultScreenOfDisplay(swfw_ctx_x11->display);
	swfw_ctx_x11->visual = XDefaultVisualOfScreen(swfw_ctx_x11->screen);
	swfw_ctx_x11->root = XRootWindowOfScreen(swfw_ctx_x11->screen);
	swfw_ctx_x11->depth = XDefaultDepthOfScreen(swfw_ctx_x11->screen);
	swfw_ctx_x11->colormap = XCreateColormap(swfw_ctx_x11->display,
		swfw_ctx_x11->root,
		swfw_ctx_x11->visual,
		AllocNone);
	XFlush(swfw_ctx_x11->display);
	/* Attributes */
	swfw_ctx_x11->mask = CWBackPixel | CWColormap | CWEventMask;
	swfw_ctx_x11->attributes.background_pixel = 0;
	swfw_ctx_x11->attributes.event_mask = StructureNotifyMask |
		KeyPressMask | KeyReleaseMask |
		PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
		ExposureMask | FocusChangeMask | VisibilityChangeMask |
		EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
	/* Save X11 atoms */
	swfw_ctx_x11->atom_WM_PROTOCOLS = XInternAtom(swfw_ctx_x11->display, "WM_PROTOCOLS", False);
	swfw_ctx_x11->atom_WM_STATE = XInternAtom(swfw_ctx_x11->display, "WM_STATE", False);
	swfw_ctx_x11->atom_WM_DELETE_WINDOW = XInternAtom(swfw_ctx_x11->display, "WM_DELETE_WINDOW", False);
	swfw_ctx_x11->atom_NET_WM_ICON = XInternAtom(swfw_ctx_x11->display, "_NET_WM_ICON", False);
	swfw_ctx_x11->atom_NET_WM_PING = XInternAtom(swfw_ctx_x11->display, "_NET_WM_PING", False);
	swfw_ctx_x11->atom_NET_WM_PID = XInternAtom(swfw_ctx_x11->display, "_NET_WM_PID", False);
	swfw_ctx_x11->atom_NET_WM_NAME = XInternAtom(swfw_ctx_x11->display, "_NET_WM_NAME", False);
	swfw_ctx_x11->atom_NET_WM_ICON_NAME = XInternAtom(swfw_ctx_x11->display, "_NET_WM_ICON_NAME", False);
	swfw_ctx_x11->atom_NET_WM_MOVERESIZE = XInternAtom(swfw_ctx_x11->display, "_NET_WM_MOVERESIZE", False);
	swfw_ctx_x11->atom_NET_WORKAREA = XInternAtom(swfw_ctx_x11->display, "_NET_WORKAREA", True);
	swfw_ctx_x11->atom_NET_WM_BYPASS_COMPOSITOR = XInternAtom(swfw_ctx_x11->display, "_NET_WM_BYPASS_COMPOSITOR", False);
	swfw_ctx_x11->atom_MOTIF_WM_HINTS = XInternAtom(swfw_ctx_x11->display, "_MOTIF_WM_HINTS", True);
	if (locale_ok) {
		swfw_ctx_x11->im = XOpenIM(swfw_ctx_x11->display, NULL, NULL, NULL);
		if (swfw_ctx_x11->im) {
			char *arg = XGetIMValues(swfw_ctx_x11->im, XNQueryInputStyle, &swfw_ctx_x11->styles, NULL);
			if (arg) {
				XCloseIM(swfw_ctx_x11->im); 
				swfw_ctx_x11->im = NULL;
			}
		}
	}
	swfw_ctx_x11->context = XUniqueContext();
	return status;
}
#endif /* SWFW_X11 */

#ifdef SWFW_WAYLAND

#ifndef SWFW_WAYLAND_TMP_FILE_TEMPLATE
#define SWFW_WAYLAND_TMP_FILE_TEMPLATE "/swfwsoXXXXXX"
#endif

static int32_t swfw_wl_create_file(off_t size)
{
	static const char template[] = SWFW_WAYLAND_TMP_FILE_TEMPLATE;
	const char *path = NULL;
	char *tmp_name = NULL;
	int32_t fd = -1;
	path = getenv("XDG_RUNTIME_DIR");
	if (!path) {
		goto done;
	}
	tmp_name = calloc(strlen(path) + sizeof(template) + 1, sizeof(char));
	if (!tmp_name) {
		goto done;
	}
	strcpy(tmp_name, path);
	strcat(tmp_name, template);
#ifdef HAVE_MKOSTEMP
	fd = mkostemp(tmp_name, O_CLOEXEC | O_TMPFILE);
#else
	fd = mkstemp(tmp_name);
	if (fd >= 0) {
		long flags = fcntl(fd, F_GETFD);
		if (flags == -1) {
			close(fd);
			fd = -1;
		} else if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC)) {
			close(fd);
			fd = -1;
		}
	}
#endif
	if (fd < 0) {
		goto done;
	}
	if (unlink(tmp_name)) {
		close(fd);
		fd = -1;
		goto done;
	}
	if (ftruncate(fd, size) < 0) {
		close(fd);
		fd = -1;
	}
done:
	if (tmp_name) {
		free(tmp_name);
	}
	return fd;
}

static void shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
	struct swfw_window_wl *swfw_win_wl = data;
	struct wl_region *region = NULL;
	swfw_win_wl->width = width;
	swfw_win_wl->height = height;
	region = wl_compositor_create_region(swfw_win_wl->swfw_ctx_wl->compositor);
	wl_region_add(region, 0, 0, width, height);
	wl_surface_set_opaque_region(swfw_win_wl->surface, region);
	wl_surface_commit(swfw_win_wl->surface);
	wl_region_destroy(region);
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		wl_egl_window_resize(swfw_win_wl->egl_window, width, height, 0, 0);
	}
#endif
}

static const struct wl_shell_surface_listener shell_surface_listener = {
	shell_surface_ping,
	shell_surface_configure
};

static void swfw_wl_create_shell_surface(struct swfw_window_wl *swfw_win_wl)
{
	swfw_win_wl->shell_surface = wl_shell_get_shell_surface(swfw_win_wl->swfw_ctx_wl->shell, swfw_win_wl->surface);
	wl_shell_surface_add_listener(swfw_win_wl->shell_surface, &shell_surface_listener, swfw_win_wl);
	wl_shell_surface_set_toplevel(swfw_win_wl->shell_surface);
}

enum swfw_status swfw_drag_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	if (swfw_win_wl->shell_surface) {
		wl_shell_surface_move(swfw_win_wl->shell_surface, swfw_win_wl->swfw_ctx_wl->seat, swfw_win_wl->swfw_ctx_wl->pointer_serial);
	}
	return SWFW_OK;
}

enum swfw_status swfw_resize_window_wl(struct swfw_window_wl *swfw_win_wl, enum swfw_window_border window_border)
{
	enum swfw_status status = SWFW_OK;
	int32_t border = 0;
	if (window_border == SWFW_WINDOW_BORDER_LEFT) {
		border = WL_SHELL_SURFACE_RESIZE_LEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP) {
		border = WL_SHELL_SURFACE_RESIZE_TOP;
	} else if (window_border == SWFW_WINDOW_BORDER_RIGHT) {
		border = WL_SHELL_SURFACE_RESIZE_RIGHT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM) {
		border = WL_SHELL_SURFACE_RESIZE_BOTTOM;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP_LEFT) {
		border = WL_SHELL_SURFACE_RESIZE_TOP_LEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_TOP_RIGHT) {
		border = WL_SHELL_SURFACE_RESIZE_TOP_RIGHT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM_LEFT) {
		border = WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT;
	} else if (window_border == SWFW_WINDOW_BORDER_BOTTOM_RIGHT) {
		border = WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT;
	} else {
		status = SWFW_ERROR;
	}
	if (status == SWFW_OK && swfw_win_wl->shell_surface) {
		wl_shell_surface_resize(swfw_win_wl->shell_surface,
			swfw_win_wl->swfw_ctx_wl->seat,
			swfw_win_wl->swfw_ctx_wl->pointer_serial,
			border);
	}
	return status;
}

enum swfw_status swfw_hide_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	if (swfw_win_wl->surface) {
		wl_surface_attach(swfw_win_wl->surface, NULL, 0, 0);
		wl_surface_commit(swfw_win_wl->surface);
		wl_display_dispatch(swfw_win_wl->swfw_ctx_wl->display);
	}
	return SWFW_OK;
}

enum swfw_status swfw_show_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	if (swfw_win_wl->surface) {
		wl_surface_attach(swfw_win_wl->surface, swfw_win_wl->buffer, 0, 0);
	}
	wl_surface_commit(swfw_win_wl->surface);
	wl_display_dispatch(swfw_win_wl->swfw_ctx_wl->display);
	return SWFW_OK;
}

enum swfw_status swfw_get_window_work_area_wl(struct swfw_window_wl *swfw_win_wl, int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_position_wl(struct swfw_window_wl *swfw_win_wl, int32_t x, int32_t y)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_size_wl(struct swfw_window_wl *swfw_win_wl, int32_t width, int32_t height)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_size_limits_wl(struct swfw_window_wl *swfw_win_wl, int32_t min_width, int32_t min_height, int32_t max_width, int32_t max_height)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_resizable_wl(struct swfw_window_wl *swfw_win_wl, bool resizable)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_decorated_wl(struct swfw_window_wl *swfw_win_wl, bool decorated)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_set_window_title_wl(struct swfw_window_wl *swfw_win_wl, char *title)
{
	if (swfw_win_wl->shell_surface) {
		wl_shell_surface_set_title(swfw_win_wl->shell_surface, title);
	}
	return SWFW_OK;
}

enum swfw_status swfw_window_swap_interval_wl(struct swfw_window_wl *swfw_win_wl, int32_t interval)
{
	enum swfw_status status = SWFW_OK;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		status = swfw_egl_swap_interval(&swfw_win_wl->swfw_ctx_egl, interval);
	}
#endif
	return status;
}

enum swfw_status swfw_window_swap_buffers_wl(struct swfw_window_wl *swfw_win_wl)
{
	enum swfw_status status = SWFW_OK;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		status = swfw_egl_swap_buffers(&swfw_win_wl->swfw_ctx_egl);
	}
#endif
	return status;
}

enum swfw_status swfw_destroy_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	if (swfw_win_wl->buffer) {
		wl_surface_attach(swfw_win_wl->surface, NULL, 0, 0);
		wl_buffer_destroy(swfw_win_wl->buffer);
		wl_shm_pool_destroy(swfw_win_wl->shm_pool);
		close(swfw_win_wl->fd);
	}
	wl_surface_destroy(swfw_win_wl->surface);
	wl_shell_surface_destroy(swfw_win_wl->shell_surface);
	return SWFW_OK;
}

void surface_listener_enter(void *data, struct wl_surface *wl_surface, struct wl_output *output)
{
}

static void surface_listener_leave(void *data, struct wl_surface *wl_surface, struct wl_output *output)
{
}

static const struct wl_surface_listener surface_listener = {
	surface_listener_enter,
	surface_listener_leave
};

enum swfw_status swfw_make_window_wl(struct swfw_context_wl *swfw_ctx_wl, struct swfw_window_wl *swfw_win_wl, struct swfw_hints hints)
{
	enum swfw_status status = SWFW_OK;
	struct wl_region *region = NULL;
	swfw_win_wl->width = hints.width;
	swfw_win_wl->height = hints.height;
	/* Wayland surface */
	swfw_win_wl->swfw_ctx_wl = swfw_ctx_wl;
	swfw_win_wl->surface = wl_compositor_create_surface(swfw_ctx_wl->compositor);
	wl_surface_add_listener(swfw_win_wl->surface, &surface_listener, swfw_win_wl);
	swfw_wl_create_shell_surface(swfw_win_wl);
	/* Opaque region */
	region = wl_compositor_create_region(swfw_win_wl->swfw_ctx_wl->compositor);
	wl_region_add(region, 0, 0, hints.width, hints.height);
	wl_surface_set_opaque_region(swfw_win_wl->surface, region);
	wl_surface_commit(swfw_win_wl->surface);
	wl_region_destroy(region);
	/* Buffer */
	swfw_win_wl->fd = swfw_wl_create_file(swfw_win_wl->width * 4 * swfw_win_wl->height);
	if (swfw_win_wl->fd >= 0) {
		swfw_win_wl->shm_data = mmap(NULL, swfw_win_wl->width * 4 * swfw_win_wl->height, PROT_READ | PROT_WRITE, MAP_SHARED, swfw_win_wl->fd, 0);
		swfw_win_wl->shm_pool = wl_shm_create_pool(swfw_win_wl->swfw_ctx_wl->shm, swfw_win_wl->fd, swfw_win_wl->width * 4 * swfw_win_wl->height);
	}
	if (swfw_win_wl->shm_pool) {
		swfw_win_wl->buffer = wl_shm_pool_create_buffer(swfw_win_wl->shm_pool, 0,
			swfw_win_wl->width, swfw_win_wl->height, swfw_win_wl->width * 4, WL_SHM_FORMAT_XRGB8888);
	}
	/* Hardware acceleration */
	swfw_win_wl->use_hardware_acceleration = hints.use_hardware_acceleration;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		if (initialize_egl(&swfw_win_wl->swfw_ctx_egl, swfw_ctx_wl->display) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_get_config(&swfw_win_wl->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		swfw_win_wl->egl_window = wl_egl_window_create(swfw_win_wl->surface,
			hints.width, hints.height);
		if (!swfw_win_wl->egl_window) {
			abort();
		}
		if (swfw_egl_create_context(&swfw_win_wl->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_create_surface(&swfw_win_wl->swfw_ctx_egl, (EGLNativeWindowType)swfw_win_wl->egl_window) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_make_current(&swfw_win_wl->swfw_ctx_egl) != SWFW_OK) {
			abort();
		}
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glFlush();
		if (swfw_window_swap_buffers_wl(swfw_win_wl) != SWFW_OK) {
			abort();
		}
	}
#endif
	return status;
}

enum swfw_status swfw_get_screen_size_wl(struct swfw_context_wl *swfw_ctx_wl, int32_t i, int32_t *width, int32_t *height)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_get_screens_wl(struct swfw_context_wl *swfw_ctx_wl, int32_t *i)
{
	return SWFW_UNSUPPORTED;
}

bool swfw_poll_event_wl(struct swfw_context_wl *swfw_ctx_wl, struct swfw_event *event)
{
	bool has_event = false;
	if (wl_display_dispatch_pending(swfw_ctx_wl->display) > 0) {
		*event = swfw_ctx_wl->event;
		if (swfw_ctx_wl->event.type != 0) {
			has_event = true;
		}
	}
	return has_event;
}

enum swfw_status swfw_destroy_context_wl(struct swfw_context_wl *swfw_ctx_wl)
{
	return SWFW_OK;
}

static void pointer_listener_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	swfw_ctx_wl->pointer_serial = serial;
	e.type = SWFW_EVENT_CURSOR_ENTER;
	swfw_ctx_wl->event = e;
}

static void pointer_listener_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	swfw_ctx_wl->pointer_serial = serial;
	e.type = SWFW_EVENT_CURSOR_LEAVE;
	swfw_ctx_wl->event = e;
}

static void pointer_listener_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	e.type = SWFW_EVENT_CURSOR_MOTION;
	e.x = wl_fixed_to_double(x);
	e.y = wl_fixed_to_double(y);
	swfw_ctx_wl->event = e;
}

static void pointer_listener_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	swfw_ctx_wl->pointer_serial = serial;
	if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
		e.type = SWFW_EVENT_BUTTON_PRESS;
	} else {
		e.type = SWFW_EVENT_BUTTON_RELEASE;
	}
	e.button = button;
	swfw_ctx_wl->event = e;
}

static void pointer_listener_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	e.type = SWFW_EVENT_SCROLL;
	e.axis = axis;
	e.scroll = wl_fixed_to_double(value);
	swfw_ctx_wl->event = e;
}

static void keyboard_listener_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int32_t fd, uint32_t size)
{
}

static void keyboard_listener_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys)
{
}

static void keyboard_listener_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface)
{
}

static void keyboard_listener_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	struct swfw_event e = {0};
	if (state == 0) {
		e.type = SWFW_EVENT_KEY_RELEASE;
	} else {
		e.type = SWFW_EVENT_KEY_PRESS;
	}
	e.key_code = key;
	swfw_ctx_wl->event = e;
}

static void keyboard_listener_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
}

static struct wl_pointer_listener pointer_listener = {
	pointer_listener_enter,
	pointer_listener_leave,
	pointer_listener_motion,
	pointer_listener_button,
	pointer_listener_axis
};

static struct wl_keyboard_listener keyboard_listener = {
	keyboard_listener_keymap,
	keyboard_listener_enter,
	keyboard_listener_leave,
	keyboard_listener_key,
	keyboard_listener_modifiers
};

static void seat_listener_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability capabilities)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		swfw_ctx_wl->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(swfw_ctx_wl->pointer, &pointer_listener, swfw_ctx_wl);
	}
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		swfw_ctx_wl->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(swfw_ctx_wl->keyboard, &keyboard_listener, swfw_ctx_wl);
	}
}

static void shm_listener_format(void *data, struct wl_shm *wl_shm, uint32_t format)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	if (format == WL_SHM_FORMAT_XRGB8888) {
		swfw_ctx_wl->has_xrgb = true;
	}
}

static void output_listener_geometry(void *data, struct wl_output *output,
	int32_t x, int32_t y, int32_t width, int32_t height, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
}

static void output_listener_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
}

static void output_listener_done(void *data, struct wl_output *wl_output)
{
}

static void output_listener_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
}

static struct wl_seat_listener seat_listener = {
	seat_listener_capabilities
};

struct wl_shm_listener shm_listener = {
	shm_listener_format
};

struct wl_output_listener output_listener = {
	output_listener_geometry,
	output_listener_mode,
	output_listener_done,
	output_listener_scale
};

static void registry_listener_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct wl_output *output = NULL;
	struct swfw_context_wl *swfw_ctx_wl = data;
	if (!strcmp(interface, "wl_compositor")) {
		swfw_ctx_wl->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (!strcmp(interface, "wl_shell")) {
		swfw_ctx_wl->shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
	} else if (!strcmp(interface, "wl_seat")) {
		swfw_ctx_wl->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(swfw_ctx_wl->seat, &seat_listener, swfw_ctx_wl);
	} else if (!strcmp(interface, "wl_shm")) {
		swfw_ctx_wl->shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
		wl_shm_add_listener(swfw_ctx_wl->shm, &shm_listener, swfw_ctx_wl);
	} else if (!strcmp(interface, "wl_output")) {
		output = wl_registry_bind(registry, name, &wl_output_interface, 1);
		wl_output_add_listener(output, &output_listener, swfw_ctx_wl);
	}
}

static void registry_listener_global_remove(void *data, struct wl_registry *registry, uint32_t id)
{
}

static const struct wl_registry_listener registry_listener = {
	registry_listener_global,
	registry_listener_global_remove
};

enum swfw_status swfw_make_context_wl(struct swfw_context_wl *swfw_ctx_wl)
{
	enum swfw_status status = SWFW_OK;
	struct wl_registry *registry = NULL;
	swfw_ctx_wl->display = wl_display_connect(NULL);
	if (!swfw_ctx_wl->display) {
		status = SWFW_ERROR;
		goto done;
	}
	registry = wl_display_get_registry(swfw_ctx_wl->display);
	wl_registry_add_listener(registry, &registry_listener, swfw_ctx_wl);
	wl_display_dispatch(swfw_ctx_wl->display);
	wl_display_roundtrip(swfw_ctx_wl->display);
	if (!swfw_ctx_wl->compositor || !swfw_ctx_wl->shell || !swfw_ctx_wl->seat || !swfw_ctx_wl->shm) {
		status = SWFW_ERROR;
	}
done:
	return status;
}
#endif /* SWFW_WAYLAND */

/* SWFW API */

/* Window */
enum swfw_status swfw_drag_window(struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_drag_window_x11(&swfw_win->swfw_win_x11);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_drag_window_wl(&swfw_win->swfw_win_wl);
#endif
	}
	return status;
}

enum swfw_status swfw_resize_window(struct swfw_window *swfw_win, enum swfw_window_border window_border)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_resize_window_x11(&swfw_win->swfw_win_x11, window_border);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_resize_window_wl(&swfw_win->swfw_win_wl, window_border);
#endif
	}
	return status;
}

enum swfw_status swfw_hide_window(struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_hide_window_x11(&swfw_win->swfw_win_x11);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_hide_window_wl(&swfw_win->swfw_win_wl);
#endif
	}
	return status;
}

enum swfw_status swfw_show_window(struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_show_window_x11(&swfw_win->swfw_win_x11);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_show_window_wl(&swfw_win->swfw_win_wl);
#endif
	}
	return status;
}

enum swfw_status swfw_get_window_work_area(struct swfw_window *swfw_win, int32_t *x, int32_t *y, int32_t *width, int32_t *height)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_get_window_work_area_x11(&swfw_win->swfw_win_x11, x, y, width, height);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_get_window_work_area_wl(&swfw_win->swfw_win_wl, x, y, width, height);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_position(struct swfw_window *swfw_win, int32_t x, int32_t y)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_position_x11(&swfw_win->swfw_win_x11, x, y);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_position_wl(&swfw_win->swfw_win_wl, x, y);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_size(struct swfw_window *swfw_win, int32_t width, int32_t height)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_size_x11(&swfw_win->swfw_win_x11, width, height);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_size_wl(&swfw_win->swfw_win_wl, width, height);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_size_limits(struct swfw_window *swfw_win, int32_t min_width, int32_t min_height, int32_t max_width, int32_t max_height)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_size_limits_x11(&swfw_win->swfw_win_x11, min_width, min_height, max_width, max_height);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_size_limits_wl(&swfw_win->swfw_win_wl, min_width, min_height, max_width, max_height);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_resizable(struct swfw_window *swfw_win, bool resizable)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_resizable_x11(&swfw_win->swfw_win_x11, resizable);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_resizable_wl(&swfw_win->swfw_win_wl, resizable);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_decorated(struct swfw_window *swfw_win, bool decorated)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_decorated_x11(&swfw_win->swfw_win_x11, decorated);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_decorated_wl(&swfw_win->swfw_win_wl, decorated);
#endif
	}
	return status;
}

enum swfw_status swfw_set_window_title(struct swfw_window *swfw_win, char *title)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_set_window_title_x11(&swfw_win->swfw_win_x11, title);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_set_window_title_wl(&swfw_win->swfw_win_wl, title);
#endif
	}
	return status;
}

enum swfw_status swfw_window_swap_interval(struct swfw_window *swfw_win, int32_t interval)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_window_swap_interval_x11(&swfw_win->swfw_win_x11, interval);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_window_swap_interval_wl(&swfw_win->swfw_win_wl, interval);
#endif
	}
	return status;
}

enum swfw_status swfw_window_swap_buffers(struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_window_swap_buffers_x11(&swfw_win->swfw_win_x11);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_window_swap_buffers_wl(&swfw_win->swfw_win_wl);
#endif
	}
	return status;

}

enum swfw_status swfw_destroy_window(struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_destroy_window_x11(&swfw_win->swfw_win_x11);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_destroy_window_wl(&swfw_win->swfw_win_wl);
#endif
	}
	return status;
}

enum swfw_status swfw_make_window(struct swfw_context *swfw_ctx, struct swfw_window *swfw_win)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	swfw_win->swfw_ctx = swfw_ctx;
	if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_make_window_x11(&swfw_ctx->swfw_ctx_x11, &swfw_win->swfw_win_x11, swfw_ctx->hints);
#endif
	} else if (swfw_win->swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_make_window_wl(&swfw_ctx->swfw_ctx_wl, &swfw_win->swfw_win_wl, swfw_ctx->hints);
#endif
	}
	return status;
}

/* Context */
enum swfw_status swfw_hint_window_size(struct swfw_context *swfw_ctx, int32_t width, int32_t height)
{
	swfw_ctx->hints.width = width;
	swfw_ctx->hints.height = height;
	return SWFW_OK;
}

enum swfw_status swfw_hint_gl_version(struct swfw_context *swfw_ctx, int32_t major, int32_t minor)
{
	swfw_ctx->hints.gl.major = major;
	swfw_ctx->hints.gl.minor = minor;
	return SWFW_OK;
}

enum swfw_status swfw_hint_use_hardware_acceleration(struct swfw_context *swfw_ctx, bool use_hardware_acceleration)
{
	swfw_ctx->hints.use_hardware_acceleration = use_hardware_acceleration;
	return SWFW_OK;
}

enum swfw_status swfw_get_screen_size(struct swfw_context *swfw_ctx, int32_t i, int32_t *width, int32_t *height)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_get_screen_size_x11(&swfw_ctx->swfw_ctx_x11, i, width, height);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_get_screen_size_wl(&swfw_ctx->swfw_ctx_wl, i, width, height);
#endif
	}
	return status;
}

enum swfw_status swfw_get_screens(struct swfw_context *swfw_ctx, int32_t *i)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_get_screens_x11(&swfw_ctx->swfw_ctx_x11, i);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_get_screens_wl(&swfw_ctx->swfw_ctx_wl, i);
#endif
	}
	return status;
}

bool swfw_poll_event(struct swfw_context *swfw_ctx, struct swfw_event *event)
{
	bool has_event = false;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		has_event = swfw_poll_event_x11(&swfw_ctx->swfw_ctx_x11, event);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		has_event = swfw_poll_event_wl(&swfw_ctx->swfw_ctx_wl, event);
#endif
	}
	return has_event;
}

enum swfw_status swfw_destroy_context(struct swfw_context *swfw_ctx)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_destroy_context_x11(&swfw_ctx->swfw_ctx_x11);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_destroy_context_wl(&swfw_ctx->swfw_ctx_wl);
#endif
	}
	return status;
}

enum swfw_status swfw_make_context(struct swfw_context *swfw_ctx, enum swfw_backend backend)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	swfw_ctx->backend = backend;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_make_context_x11(&swfw_ctx->swfw_ctx_x11);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_make_context_wl(&swfw_ctx->swfw_ctx_wl);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_AUTOMATIC) {
#if defined(SWFW_X11) && !defined(SWFW_WAYLAND)
		swfw_ctx->backend = SWFW_BACKEND_X11;
		status = swfw_make_context_x11(&swfw_ctx->swfw_ctx_x11);
#endif
#if !defined(SWFW_X11) && defined(SWFW_WAYLAND)
		swfw_ctx->backend = SWFW_BACKEND_WAYLAND;
		status = swfw_make_context_wl(&swfw_ctx->swfw_ctx_wl);
#endif
#if defined(SWFW_X11) && defined(SWFW_WAYLAND)
		swfw_ctx->backend = SWFW_BACKEND_X11;
		status = swfw_make_context_x11(&swfw_ctx->swfw_ctx_x11);
#endif
	}
	swfw_ctx->hints.x = 0;
	swfw_ctx->hints.y = 0;
	swfw_ctx->hints.width = DEFAULT_HINT_SIZE_WIDTH;
	swfw_ctx->hints.height = DEFAULT_HINT_SIZE_HEIGHT;
	swfw_ctx->hints.use_hardware_acceleration = DEFAULT_HINT_USE_HARDWARE_ACCELERATION;
	return status;
}
