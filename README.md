# SWFW - Simple Window Framework

This is a cross-platform simple window framework. This library contains only two files, `swfw.c` and `swfw.h`.

## Version

This library still hasn't reached its first major release, which means that the API might change and break backwards compatility.

## Planned Support

- Platforms:
  - Linux and FreeBSD `in-progress`
    - X11 `in-progress`
    - Wayland `in-progress`
  - Windows `not-started`
  - Mac OS X `not-started`
  - Web (using Emscripten) `not-started`
  - Mobile (Android and iOS) `not-started`

## Compiling

## Linux and FreeBSD

On Linux and FreeBSD, the compilation should specify the backend using the macros `SWFW_X11` and `SWFW_WAYLAND`. With GCC, this can be cone using the `-D` option:

```Makefile
gcc -DSWFW_X11 -DSWFW_WAYLAND -c ./swfw.c
```

The example above should compile to a object file (`swfw.o`) with X11 and Wayland support. This you can choose at run-time which one of the two backend the program will use to create a window. This advantage comes at the cost of the user having to have the two backends installed in their computer. You can also define only one of the macros, and the program will support only one of the backends, requiring the user to have only one of the two.

On Linux and FreeBSD, when including `swfw.h` in your file, two approaches can be used:

1. Add the same macros that you used for compilation before the inclusion of the header:

```
#define SWFW_X11
#define SWFW_WAYLAND
#include "swfw.h"
```

2. Or also compile your file adding the option `-D` with the macros specifying the supported backends:

```
gcc -DSWFW_X11 -DSWFW_WAYLAND -c swfw.c
gcc -DSWFW_X11 -DSWFW_WAYLAND -c main.c # Macros also defined in your file.
```

The example `simple.c` contains a minimal application using the second approach.

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
