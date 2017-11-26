/*
Copyright (C) 2017 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In
no event will the authors be held liable for any damages arising from the use of
this software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim
     that you wrote the original software. If you use this software in a
     product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SWFW_H
#define SWFW_H

#include <stdint.h>
#include <stdbool.h>
#ifdef SWFW_X11
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#ifndef SWFW_NO_HARDWARE_ACCELERATION
		#define SWFW_EGL
	#endif
	#ifdef SWFW_EGL
		#include <EGL/egl.h>
		#include <GLES2/gl2.h>
	#endif
#endif
#ifdef SWFW_WAYLAND
	#include <wayland-server.h>
	#include <wayland-client.h>
	#ifndef SWFW_NO_HARDWARE_ACCELERATION
		#define SWFW_EGL
	#endif
	#ifdef SWFW_EGL
		#include <wayland-egl.h>
		#include <EGL/egl.h>
		#include <GLES2/gl2.h>
	#endif
#endif

#define DEFAULT_HINT_USE_HARDWARE_ACCELERATION true
#define DEFAULT_HINT_SIZE_WIDTH 256
#define DEFAULT_HINT_SIZE_HEIGHT 256

enum swfw_status {
	SWFW_OK,
	SWFW_ERROR,
	SWFW_INVALID_BACKEND,
	SWFW_UNSUPPORTED
};

enum swfw_backend {
	SWFW_BACKEND_AUTOMATIC,
	SWFW_BACKEND_X11,
	SWFW_BACKEND_WAYLAND,
	SWFW_BACKEND_WIN32,
	SWFW_BACKEND_COCOA,
	SWFW_BACKEND_WEB
};

enum swfw_window_border {
	SWFW_WINDOW_BORDER_LEFT,
	SWFW_WINDOW_BORDER_TOP,
	SWFW_WINDOW_BORDER_RIGHT,
	SWFW_WINDOW_BORDER_BOTTOM,
	SWFW_WINDOW_BORDER_TOP_LEFT,
	SWFW_WINDOW_BORDER_TOP_RIGHT,
	SWFW_WINDOW_BORDER_BOTTOM_LEFT,
	SWFW_WINDOW_BORDER_BOTTOM_RIGHT
};

struct swfw_gl_hints {
	int32_t major;
	int32_t minor;
};

struct swfw_hints {
	int32_t x;
	int32_t y;
	int32_t width;
	int32_t height;
	bool use_hardware_acceleration;
	struct swfw_gl_hints gl;
};

enum swfw_event_type {
	SWFW_EVENT_CONFIGURE,
	SWFW_EVENT_MAP,
	SWFW_EVENT_UNMAP,
	SWFW_EVENT_CLOSE,
	SWFW_EVENT_MAXIMIZE,
	SWFW_EVENT_ICONIFY,
	SWFW_EVENT_RESTORE,
	SWFW_EVENT_KEY_PRESS,
	SWFW_EVENT_KEY_RELEASE,
	SWFW_EVENT_CURSOR_ENTER,
	SWFW_EVENT_CURSOR_LEAVE,
	SWFW_EVENT_CURSOR_MOTION,
	SWFW_EVENT_BUTTON_PRESS,
	SWFW_EVENT_BUTTON_RELEASE,
	SWFW_EVENT_EXPOSE,
	SWFW_EVENT_DESTROY
};

struct swfw_event {
	enum swfw_event_type type;
	uint32_t key_code;
	double x;
	double y;
};

#ifdef SWFW_EGL
struct swfw_egl_context {
	EGLint major;
	EGLint minor;
	EGLDisplay display;
	EGLContext context;
	EGLConfig config;
	EGLSurface surface;
};
#endif

#ifdef SWFW_X11
struct swfw_context_x11 {
	Display *display;
	Visual *visual;
	int screen;
	int depth;
	XIM im;
	Window root;
	XSetWindowAttributes attributes;
	Colormap colormap;
	unsigned long mask;
	Atom atom_WM_PROTOCOLS;
	Atom atom_WM_STATE;
	Atom atom_WM_DELETE_WINDOW;
	Atom atom_NET_WM_NAME;
	Atom atom_NET_WM_ICON_NAME;
	Atom atom_NET_WM_ICON;
	Atom atom_NET_WM_PID;
	Atom atom_NET_WM_PING;
	Atom atom_NET_WM_WINDOW_TYPE;
	Atom atom_NET_WM_WINDOW_TYPE_NORMAL;
	Atom atom_NET_WM_STATE;
	Atom atom_NET_WM_STATE_ABOVE;
	Atom atom_NET_WM_STATE_FULLSCREEN;
	Atom atom_NET_WM_STATE_MAXIMIZED_VERT;
	Atom atom_NET_WM_STATE_MAXIMIZED_HORZ;
	Atom atom_NET_WM_STATE_DEMANDS_ATTENTION;
	Atom atom_NET_WM_BYPASS_COMPOSITOR;
	Atom atom_NET_WM_FULLSCREEN_MONITORS;
	Atom atom_NET_WM_MOVERESIZE;
	Atom atom_MOTIF_WM_HINTS;
	Atom atom_NET_ACTIVE_WINDOW;
	Atom atom_NET_FRAME_EXTENTS;
	Atom atom_NET_REQUEST_FRAME_EXTENTS;
	Atom atom_NET_WORKAREA;
};

struct swfw_window_x11 {
	Window window;
	XIC ic;
	double cursor_x;
	double cursor_y;
	int32_t window_x;
	int32_t window_y;
	struct swfw_context_x11 *swfw_ctx_x11;
	bool use_hardware_acceleration;
#ifdef SWFW_EGL
	struct swfw_egl_context swfw_egl_ctx;
#endif
};
#endif

#ifdef SWFW_WAYLAND
struct swfw_context_wl {
	struct wl_display *display;
	struct wl_compositor *compositor;
	struct wl_shell *shell;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
	uint32_t pointer_serial;
	struct swfw_event event;
};

struct swfw_window_wl {
	struct wl_surface *surface;
	struct wl_shell_surface *shell_surface;
	struct wl_region *region;
	struct swfw_context_wl *swfw_ctx_wl;
	bool use_hardware_acceleration;
#ifdef SWFW_EGL
	struct wl_egl_window *egl_window;
	struct swfw_egl_context swfw_egl_ctx;
#endif
};
#endif

/* SWFW */
struct swfw_context {
#ifdef SWFW_X11
	struct swfw_context_x11 swfw_ctx_x11;
#endif
#ifdef SWFW_WAYLAND
	struct swfw_context_wl swfw_ctx_wl;
#endif
	enum swfw_backend backend;
	struct swfw_hints hints;
};

struct swfw_window {
#ifdef SWFW_X11
	struct swfw_window_x11 swfw_win_x11;
#endif
#ifdef SWFW_WAYLAND
	struct swfw_window_wl swfw_win_wl;
#endif
	struct swfw_context *swfw_ctx;
};

/* Window */
enum swfw_status swfw_get_cursor_position(struct swfw_window *swfw_win, double *cursor_x, double *cursor_y);
enum swfw_status swfw_get_window_position(struct swfw_window *swfw_win, int32_t *window_x, int32_t *window_y);
enum swfw_status swfw_drag_window(struct swfw_window *swfw_win);
enum swfw_status swfw_resize_window(struct swfw_window *swfw_win, enum swfw_window_border window_border);
enum swfw_status swfw_hide_window(struct swfw_window *swfw_win);
enum swfw_status swfw_show_window(struct swfw_window *swfw_win);
enum swfw_status swfw_get_window_work_area(struct swfw_window *swfw_win, int32_t *x, int32_t *y, int32_t *width, int32_t *height);
enum swfw_status swfw_set_window_position(struct swfw_window *swfw_win, int32_t x, int32_t y);
enum swfw_status swfw_set_window_size(struct swfw_window *swfw_win, int32_t width, int32_t height);
enum swfw_status swfw_set_window_size_limits(struct swfw_window *swfw_win, int32_t min_width, int32_t min_height, int32_t max_width, int32_t max_height);
enum swfw_status swfw_set_window_resizable(struct swfw_window *swfw_win, bool resizable);
enum swfw_status swfw_set_window_decorated(struct swfw_window *swfw_win, bool decorated);
enum swfw_status swfw_set_window_title(struct swfw_window *swfw_win, char *title);
enum swfw_status swfw_window_swap_buffers(struct swfw_window *swfw_win);
enum swfw_status swfw_destroy_window(struct swfw_window *swfw_win);
enum swfw_status swfw_make_window(struct swfw_context *swfw_ctx, struct swfw_window *swfw_win);

/* Context */
enum swfw_status swfw_hint_window_size(struct swfw_context *swfw_ctx, int32_t width, int32_t height);
enum swfw_status swfw_hint_gl_version(struct swfw_context *swfw_ctx, int32_t major, int32_t minor);
enum swfw_status swfw_hint_use_hardware_acceleration(struct swfw_context *swfw_ctx, bool use_hardware_acceleration);
int32_t swfw_poll_events(struct swfw_context *swfw_ctx, struct swfw_event *event);
enum swfw_status swfw_destroy_context(struct swfw_context *swfw_ctx);
enum swfw_status swfw_make_context(struct swfw_context *swfw_ctx, enum swfw_backend backend);

#endif
