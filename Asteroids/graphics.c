#include "graphics.h"

void clear_screen(Render_Buffer* rb, u32 color) {
	for (size_t i = 0; i < rb->width; i++) {
		for (size_t j = 0; j < rb->height; j++) {
			rb->pixels[i + rb->width * j] = color;
		}
	}
}