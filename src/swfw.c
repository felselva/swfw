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

#ifdef SWFW_GLX
#endif

#ifdef SWFW_EGL
static enum swfw_status swfw_egl_get_config(struct swfw_egl_context *swfw_egl_ctx)
{
	EGLConfig egl_conf = {0};
	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	EGLConfig *configs = NULL;
	EGLint egl_num_configs = 0;
	EGLint val = 0;
	EGLBoolean rc = false;
	EGLint i = 0;
	rc = eglGetConfigs(swfw_egl_ctx->display, NULL, 0, &egl_num_configs);
	if (rc != EGL_TRUE) {
		return false;
	}
	if (egl_num_configs == 0) {
		return false;
	}
	configs = malloc(egl_num_configs * sizeof(*configs));
	if (configs == NULL) {
		return false;
	}
	rc = eglGetConfigs(swfw_egl_ctx->display, configs, egl_num_configs, &egl_num_configs);
	if (rc != EGL_TRUE) {
		free(configs);
		return false;
	}
	for (i = 0; i < egl_num_configs; i++) {
		eglGetConfigAttrib(swfw_egl_ctx->display, configs[i], EGL_SURFACE_TYPE, &val);
		if (!(val & EGL_WINDOW_BIT)) {
			continue;
		}
		eglGetConfigAttrib(swfw_egl_ctx->display, configs[i], EGL_COLOR_BUFFER_TYPE, &val);
		if (!(val & EGL_RGB_BUFFER)) {
			continue;
		}
		eglGetConfigAttrib(swfw_egl_ctx->display, configs[i], EGL_RENDERABLE_TYPE, &val);
		if (!(val & EGL_OPENGL_ES2_BIT)) {
			continue;
		}
		eglGetConfigAttrib(swfw_egl_ctx->display, configs[i], EGL_DEPTH_SIZE, &val);
		if (val == 0) {
			continue;
		}
		egl_conf = configs[i];
		break;
	}
	free(configs);
	swfw_egl_ctx->config = egl_conf;
	return SWFW_OK;
}

enum swfw_status initialize_egl(struct swfw_egl_context *swfw_egl_ctx, void *native_display)
{
	enum swfw_status status = SWFW_OK;
	swfw_egl_ctx->display = eglGetDisplay(native_display);
	if (swfw_egl_ctx->display == EGL_NO_DISPLAY) {
		status = SWFW_ERROR;
	} else {
		if (eglInitialize(swfw_egl_ctx->display, &swfw_egl_ctx->major, &swfw_egl_ctx->minor) != EGL_TRUE) {
			status = SWFW_ERROR;
		}
	}
	return status;
}

enum swfw_status swfw_egl_create_context(struct swfw_egl_context *swfw_egl_ctx)
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
	swfw_egl_ctx->context = eglCreateContext(swfw_egl_ctx->display,
		swfw_egl_ctx->config,
		EGL_NO_CONTEXT,
		context_attribs);
	if (swfw_egl_ctx->context == EGL_NO_CONTEXT) {
		status = SWFW_ERROR;
	}
done:
	return status;
}

enum swfw_status swfw_egl_create_surface(struct swfw_egl_context *swfw_egl_ctx, EGLNativeWindowType native_window)
{
	enum swfw_status status = SWFW_OK;
	EGLint egl_surf_attr[] = {
		EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
		EGL_NONE
	};
	swfw_egl_ctx->surface = eglCreateWindowSurface(swfw_egl_ctx->display,
		swfw_egl_ctx->config,
		native_window,
		egl_surf_attr);
	if (swfw_egl_ctx->surface == EGL_NO_SURFACE) {
		status = SWFW_ERROR;
	}
	return status;
}

enum swfw_status swfw_egl_make_current(struct swfw_egl_context *swfw_egl_ctx)
{
	enum swfw_status status = SWFW_OK;
	if (eglMakeCurrent(swfw_egl_ctx->display, swfw_egl_ctx->surface, swfw_egl_ctx->surface, swfw_egl_ctx->context) == EGL_FALSE) {
		status = SWFW_ERROR;
	}
	return status;
}
#endif

#ifdef SWFW_X11
#ifndef _NET_WM_MOVERESIZE
#define _NET_WM_MOVERESIZE 8
#endif

struct x11_hints {
	uint64_t flags;
	uint64_t functions;
	uint64_t decorations;
	int64_t input_mode;
	uint64_t status;
};

/* Window */
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

enum swfw_status swfw_window_swap_buffers_x11(struct swfw_window_x11 *swfw_win_x11)
{
	enum swfw_status status = SWFW_OK;
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
	return status;
}

/* Context */
enum swfw_status swfw_pool_events_x11(struct swfw_context_x11 *swfw_ctx_x11, struct swfw_event *event)
{
	XEvent x11_event = {0};
	XNextEvent(swfw_ctx_x11->display, &x11_event);
	if (x11_event.type == Expose) {
		event->type = SWFW_EVENT_EXPOSE;
	} else if (x11_event.type == KeyPress) {
		event->type = SWFW_EVENT_KEY_PRESS;
		event->key_code = x11_event.xkey.keycode;
	} else if (x11_event.type == KeyRelease) {
		event->type = SWFW_EVENT_KEY_RELEASE;
		event->key_code = x11_event.xkey.keycode;
	} else if (x11_event.type == MotionNotify) {
		event->type = SWFW_EVENT_CURSOR_MOTION;
		event->x = x11_event.xmotion.x;
		event->y = x11_event.xmotion.y;
	} else if (x11_event.type == ButtonPress) {
		event->type = SWFW_EVENT_BUTTON_PRESS;
	} else if (x11_event.type == ButtonRelease) {
		event->type = SWFW_EVENT_BUTTON_RELEASE;
	} else if (x11_event.type == EnterNotify) {
		event->type = SWFW_EVENT_CURSOR_ENTER;
	} else if (x11_event.type == LeaveNotify) {
		event->type = SWFW_EVENT_CURSOR_LEAVE;
	} else if (x11_event.type == ConfigureNotify) {
		event->type = SWFW_EVENT_CONFIGURE;
	} else if (x11_event.type == MapNotify) {
		event->type = SWFW_EVENT_MAP;
	} else if (x11_event.type == UnmapNotify) {
		event->type = SWFW_EVENT_UNMAP;
	} else if (x11_event.type == DestroyNotify) {
		event->type = SWFW_EVENT_DESTROY;
	} else if (x11_event.type == ClientMessage) {
		if(x11_event.xclient.data.l[0] == swfw_ctx_x11->atom_WM_DELETE_WINDOW) {
			event->type = SWFW_EVENT_DESTROY;
		}
	}
	return SWFW_OK;
}

enum swfw_status swfw_destroy_context_x11(struct swfw_context_x11 *swfw_ctx_x11)
{
	XCloseDisplay(swfw_ctx_x11->display);
	return SWFW_OK;
}

enum swfw_status swfw_make_context_x11(struct swfw_context_x11 *swfw_ctx_x11)
{
	enum swfw_status status = SWFW_OK;
	swfw_ctx_x11->display = XOpenDisplay(NULL);
	swfw_ctx_x11->screen = XDefaultScreen(swfw_ctx_x11->display);
	swfw_ctx_x11->visual = XDefaultVisual(swfw_ctx_x11->display, swfw_ctx_x11->screen);
	swfw_ctx_x11->root = XRootWindow(swfw_ctx_x11->display, swfw_ctx_x11->screen);
	swfw_ctx_x11->depth = XDefaultDepth(swfw_ctx_x11->display, swfw_ctx_x11->screen);
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
	return status;
}
#endif /* SWFW_X11 */

#ifdef SWFW_WAYLAND
enum swfw_status swfw_drag_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_hide_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	return SWFW_UNSUPPORTED;
}

enum swfw_status swfw_show_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	return SWFW_UNSUPPORTED;
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
	wl_shell_surface_set_title(swfw_win_wl->shell_surface, title);
	return SWFW_OK;
}

enum swfw_status swfw_window_swap_buffers_wl(struct swfw_window_wl *swfw_win_wl)
{
	enum swfw_status status = SWFW_OK;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		eglSwapBuffers(swfw_win_wl->swfw_egl_ctx.display, swfw_win_wl->swfw_egl_ctx.surface);
		if (eglGetError() != EGL_SUCCESS) {
			status = SWFW_ERROR;
		}
	}
#endif
	return status;
}

enum swfw_status swfw_destroy_window_wl(struct swfw_window_wl *swfw_win_wl)
{
	return SWFW_OK;
}

static void shell_surface_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

static void shell_surface_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height)
{
	struct swfw_window_wl *swfw_win_wl = data;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		wl_egl_window_resize(swfw_win_wl->egl_window, width, height, 0, 0);
	}
#endif
}

static void shell_surface_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static void surface_enter(void *data, struct wl_surface *surface, struct wl_output *output)
{
	struct swfw_window_wl *swfw_win_wl = data;
	/* Surface enter */
}

static void surface_leave(void *data, struct wl_surface *surface, struct wl_output *output)
{
	struct swfw_window_wl *swfw_win_wl = data;
	/* Surface leave */
}

static const struct wl_shell_surface_listener shell_surface_listener = {
	shell_surface_ping,
	shell_surface_configure,
	shell_surface_popup_done
};

static const struct wl_surface_listener surface_listener = {
	surface_enter,
	surface_leave
};

enum swfw_status swfw_make_window_wl(struct swfw_context_wl *swfw_ctx_wl, struct swfw_window_wl *swfw_win_wl, struct swfw_hints hints)
{
	enum swfw_status status = SWFW_OK;
	/* Wayland surface */
	swfw_win_wl->swfw_ctx_wl = swfw_ctx_wl;
	swfw_win_wl->surface = wl_compositor_create_surface(swfw_ctx_wl->compositor);
	wl_surface_add_listener(swfw_win_wl->surface, &surface_listener, swfw_win_wl);
	/* Wayland shell surface */
	swfw_win_wl->shell_surface = wl_shell_get_shell_surface(swfw_ctx_wl->shell, swfw_win_wl->surface);
	wl_shell_surface_add_listener(swfw_win_wl->shell_surface, &shell_surface_listener, swfw_win_wl);
	wl_shell_surface_set_toplevel(swfw_win_wl->shell_surface);
	/* Opaque region */
	swfw_win_wl->region = wl_compositor_create_region(swfw_ctx_wl->compositor);
	wl_region_add(swfw_win_wl->region, 0, 0, hints.width, hints.height);
	wl_surface_set_opaque_region(swfw_win_wl->surface, swfw_win_wl->region);
	wl_surface_commit(swfw_win_wl->surface);
	wl_region_destroy(swfw_win_wl->region);
	swfw_win_wl->use_hardware_acceleration = hints.use_hardware_acceleration;
#ifdef SWFW_EGL
	if (swfw_win_wl->use_hardware_acceleration) {
		if (initialize_egl(&swfw_win_wl->swfw_egl_ctx, swfw_ctx_wl->display) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_get_config(&swfw_win_wl->swfw_egl_ctx) != SWFW_OK) {
			abort();
		}
		swfw_win_wl->egl_window = wl_egl_window_create(swfw_win_wl->surface,
			hints.width, hints.height);
		if (!swfw_win_wl->egl_window) {
			abort();
		}
		if (swfw_egl_create_context(&swfw_win_wl->swfw_egl_ctx) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_create_surface(&swfw_win_wl->swfw_egl_ctx, (EGLNativeWindowType)swfw_win_wl->egl_window) != SWFW_OK) {
			abort();
		}
		if (swfw_egl_make_current(&swfw_win_wl->swfw_egl_ctx) != SWFW_OK) {
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

/* Context */
enum swfw_status swfw_pool_events_wl(struct swfw_context_wl *swfw_ctx_wl, struct swfw_event *event)
{
	wl_display_dispatch(swfw_ctx_wl->display);
	return SWFW_OK;
}

enum swfw_status swfw_destroy_context_wl(struct swfw_context_wl *swfw_ctx_wl)
{
	return SWFW_OK;
}

static void pointer_listener_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
}

static void pointer_listener_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
}

static void pointer_listener_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t x, wl_fixed_t y)
{
}

static void pointer_listener_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
}

static void pointer_listener_axis(void *data, struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value)
{
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
		struct wl_pointer *pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(pointer, &pointer_listener, swfw_ctx_wl);
	} else if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(keyboard, &keyboard_listener, swfw_ctx_wl);
	}
}

static struct wl_seat_listener seat_listener = {
	seat_listener_capabilities
};

static void registry_listener_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct swfw_context_wl *swfw_ctx_wl = data;
	if (!strcmp(interface, "wl_compositor")) {
		swfw_ctx_wl->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
	} else if (!strcmp(interface, "wl_shell")) {
		swfw_ctx_wl->shell = wl_registry_bind(registry, name, &wl_shell_interface, 1);
	} else if (!strcmp(interface, "wl_seat")) {
		swfw_ctx_wl->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
		wl_seat_add_listener(swfw_ctx_wl->seat, &seat_listener, swfw_ctx_wl);
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
		abort();
	}
	registry = wl_display_get_registry(swfw_ctx_wl->display);
	wl_registry_add_listener(registry, &registry_listener, swfw_ctx_wl);
	wl_display_dispatch(swfw_ctx_wl->display);
	wl_display_roundtrip(swfw_ctx_wl->display);
	wl_display_roundtrip(swfw_ctx_wl->display);
	if (!swfw_ctx_wl->compositor || !swfw_ctx_wl->shell) {
		status = SWFW_ERROR;
	}
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

enum swfw_status swfw_pool_events(struct swfw_context *swfw_ctx, struct swfw_event *event)
{
	enum swfw_status status = SWFW_INVALID_BACKEND;
	if (swfw_ctx->backend == SWFW_BACKEND_X11) {
#ifdef SWFW_X11
		status = swfw_pool_events_x11(&swfw_ctx->swfw_ctx_x11, event);
#endif
	} else if (swfw_ctx->backend == SWFW_BACKEND_WAYLAND) {
#ifdef SWFW_WAYLAND
		status = swfw_pool_events_wl(&swfw_ctx->swfw_ctx_wl, event);
#endif
	}
	return status;
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
