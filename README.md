# SWFW - Simple Window Framework

This library is a cross-platform simple window framework containing only two files, `swfw.c` and `swfw.h`.

## Version

This library still hasn't reached its first major release, which means that the API might change and break backward compatility.

## Planned Support

- Platforms:
  - Linux and BSD `in-progress`
    - X11 `in-progress`
    - Wayland `in-progress`
  - Windows `not-started`
  - Mac OS `not-started`
  - Web (using Emscripten) `not-started`
  - Mobile (Android and iOS) `not-started`

- Graphic APIs:
  - No hardware acceleration
  - GL
  - Vulkan

The graphic APIs will be supported depending on the platform.

## Building

This project doesn't include a build system because it's intended that you add the files `swfw.c` and `swfw.h` in the build system of your project.

### Linux and BSD

On Linux and BSD, the library can be compiled to support only X11, only Wayland, or both.

The macro `SWFW_X11` is used for supporting X11, and the macro `SWFW_WAYLAND` is used for supporting Wayland. With GCC, the macros can be defined using the `-D` option:

```Makefile
# When no macro is provided, `SWFW_X11` is automatically defined internally.
gcc -c ./swfw.c
```

```Makefile
gcc -DSWFW_X11 -c ./swfw.c
```

```Makefile
gcc -DSWFW_WAYLAND -c ./swfw.c
```

```Makefile
gcc -DSWFW_X11 -DSWFW_WAYLAND -c ./swfw.c
```

The last option above is for building with support for both X11 and Wayland backends.

On Linux and BSD, when including `swfw.h` in your file, the same macros for the supported backends should be defined before the inclusion of the header:

```c
#if defined(__bsdi__) || defined(__linux__)
	#define SWFW_X11
#endif
#include "swfw.h"
```

```c
#if defined(__bsdi__) || defined(__linux__)
	#define SWFW_WAYLAND
#endif
#include "swfw.h"
```

```c
#if defined(__bsdi__) || defined(__linux__)
	#define SWFW_X11
	#define SWFW_WAYLAND
#endif
#include "swfw.h"
```

### Windows, Mac OS and Web

On Windows, Mac OS and Web, there are only one backend for each of these platforms, then there's no need to define any backend macro, only include the files `swfw.c` and `swfw.h` in the build system.

## Contributing

Contributions are very welcome. The coding style is similar to Linux Kernel coding style, but some flexibiliy is allowed.

## Contact

You can use the e-mail in my profile to contact me.

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
