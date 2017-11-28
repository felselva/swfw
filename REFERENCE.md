# SWFW Reference

## Types

```c
enum swfw_status {
	SWFW_OK,
	SWFW_ERROR,
	SWFW_INVALID_BACKEND,
	SWFW_UNSUPPORTED
};
```

The type used to indicate status.

- `SWFW_OK` indicates that the operation was successful.
- `SWFW_ERROR` indicates that the an error occurred.
- `SWFW_INVALID_BACKEND` indicates that the operation failed when trying to use an invalid backend. This value should never be returned, except if the value of `backend` inside the structure `swfw_context` was modified by the user.
- `SWFW_UNSUPPORTED` indicates that the operation is not supported by the backend used in the context.

```c
enum swfw_backend {
	SWFW_BACKEND_AUTOMATIC,
	SWFW_BACKEND_X11,
	SWFW_BACKEND_WAYLAND,
	SWFW_BACKEND_WINDOWS,
	SWFW_BACKEND_COCOA,
	SWFW_BACKEND_WEB
};
```

The type used to indicate the context backend.

On Windows, Mac OS X and Web:

- If you use `SWFW_BACKEND_AUTOMATIC`, the context will be created for their respective platform.
- Using `SWFW_BACKEND_WINDOWS`, `SWFW_BACKEND_COCOA` or `SWFW_BACKEND_WEB`, as long as in their respective platform, is also correct.

On Linux or FreeBSD:

- If you compiled only with X11 support, `SWFW_BACKEND_AUTOMATIC` and `SWFW_BACKEND_X11` will create a X11 context.
- If you compiled only with Wayland support, `SWFW_BACKEND_AUTOMATIC` and `SWFW_BACKEND_WAYLAND` will create a Wayland context.
- If you compiled with both X11 and Wayland support, `SWFW_BACKEND_AUTOMATIC` will create a X11 context. If you want to be specific, use `SWFW_BACKEND_X11` or `SWFW_BACKEND_WAYLAND`.

```c
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
```

The type used to indicate the window border.

```c
struct swfw_context {
	/* Internal structures */
};

struct swfw_window {
	/* Internal structures */
};
```

The structures `swfw_context` and `swfw_window` are used in the context initialization and creation of the window.

## Functions

```c
enum swfw_status swfw_drag_window(struct swfw_window *swfw_win);
```

Starts a dragging operation on the window. This function is supposed to work correctly only from a event of type `SWFW_EVENT_BUTTON_PRESS`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_resize_window(struct swfw_window *swfw_win, enum swfw_window_border window_border);
```

Starts a resize operation on one of the borders of the window. This function is supposed to work correctly only from a event of type `SWFW_EVENT_BUTTON_PRESS`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_hide_window(struct swfw_window *swfw_win);
```

Hides the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_show_window(struct swfw_window *swfw_win);
```

Shows the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_get_window_work_area(struct swfw_window *swfw_win, int32_t *x, int32_t *y, int32_t *width, int32_t *height);
```

Gets the display work area. The value of `x` and `y` will represent the top-left corner, and the value `width` and `height` the size of the work area.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_position(struct swfw_window *swfw_win, int32_t x, int32_t y);
```

Sets the window position to `x` and `y`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_size(struct swfw_window *swfw_win, int32_t width, int32_t height);
```

Sets the window size to `width` and `height`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_size_limits(struct swfw_window *swfw_win, int32_t min_width, int32_t min_height, int32_t max_width, int32_t max_height);
```

Sets the minimum size of the window to `min_width` and `min_height`, and the maximum size to `max_width` and `max_height`. If one of the values are negative, then the limit is unset.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_resizable(struct swfw_window *swfw_win, bool resizable);
```

Sets if the window can be resized.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_decorated(struct swfw_window *swfw_win, bool decorated);
```

Sets if the window is decorated.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_set_window_title(struct swfw_window *swfw_win, char *title);
```

Sets if the window title.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_window_swap_interval(struct swfw_window *swfw_win, int32_t interval);
```

Sets the swap interval of the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_window_swap_buffers(struct swfw_window *swfw_win);
```

Swaps the front and back buffers of the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_destroy_window(struct swfw_window *swfw_win);
```

Destroys the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_make_window(struct swfw_context *swfw_ctx, struct swfw_window *swfw_win);
```

The window is created for the context and passed to the structure `swfw_win`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_hint_window_size(struct swfw_context *swfw_ctx, int32_t width, int32_t height);
```

Sets the initial size used for the creation of the window.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_hint_gl_version(struct swfw_context *swfw_ctx, int32_t major, int32_t minor);
```

If using OpenGL for hardware acceleration, specify the OpenGL version.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_hint_use_hardware_acceleration(struct swfw_context *swfw_ctx, bool use_hardware_acceleration);
```

Specify if the context will use hardware acceleration.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
bool swfw_poll_event(struct swfw_context *swfw_ctx, struct swfw_event *event);
```

Poll event and returns to the structure `event`.

If there was an event, the function returns `true`, otherwise the function returns `false`.

```c
enum swfw_status swfw_destroy_context(struct swfw_context *swfw_ctx);
```

Destroys the context.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

```c
enum swfw_status swfw_make_context(struct swfw_context *swfw_ctx, enum swfw_backend backend);
```

The SWFW context is created using the backend defined by `backend` and passed to the structure `swfw_ctx`.

If successful, the function returns `SWFW_OK`, otherwise the function returns one of the error values.

## License

The source code of this project is licensed under the terms of the ZLIB license:

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
