all:
	@meson compile -C build

setup:
	@meson setup build --cross cross-mingw-64.txt
