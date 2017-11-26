CC ?= gcc
CFLAGS ?= -g
CFLAGS_WARNINGS ?= -Wall

CCINCL ?= -I ../../src
CCLIBS_X11 ?= -lX11 -lEGL -lGL
CCLIBS_WAYLAND ?= -lwayland-client -lwayland-egl -lEGL -lGL
CCLIBS_X11_WAYLAND ?= -lX11 -lwayland-client -lwayland-egl -lEGL -lGL

target-x11:
	$(MAKE) clear
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_X11 -c ../../src/swfw.c
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_X11 -c ./main.c
	$(CC) $(CFLAGS) -o ./simple ./*.o $(CCLIBS_X11)
	$(MAKE) clear

target-wayland:
	$(MAKE) clear
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_WAYLAND -c ../../src/swfw.c
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_WAYLAND -c ./main.c
	$(CC) $(CFLAGS) -o ./simple ./*.o $(CCLIBS_WAYLAND)
	$(MAKE) clear

target-x11-wayland:
	$(MAKE) clear
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_X11 -DSWFW_WAYLAND -c ../../src/swfw.c
	$(CC) $(CFLAGS) $(CFLAGS_WARNINGS) $(CCINCL) -DSWFW_X11 -DSWFW_WAYLAND -c ./main.c
	$(CC) $(CFLAGS) -o ./simple ./*.o $(CCLIBS_X11_WAYLAND)
	$(MAKE) clear

clear:
	rm -f ./*.o
