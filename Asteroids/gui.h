#pragma once
#include "calc.h"
#include "graphic.h"

typedef struct TextBox_s {
	u32 id;
	rect pos_rect;
	bool focused;
	char default_text[50];
	char input_text[MAX_NAME_LENGTH];
	size_t input_text_len;
	i32 cursor_pos;
	i32 header_string[50];
} TextBox;

typedef struct Button_s {
	u32 id;
	rect pos_rect;
	bool focused;
	char default_text[50];
} Button;

bool button_update(BitMap rb, Button* b, ivec2 mouse_pos, bool lmb_down, bool lmb_was_down, i32* current_focus);
bool textbox_update(BitMap rb, TextBox* t, ivec2 mouse_pos, bool lmb_down, i32* current_focus, char key_pressed);
void button_render(BitMap rb, BitMap font, Button b, bool lmb_down, i32 current_focus);
void textbox_render(BitMap rb, BitMap font, TextBox b, bool lmb_down, i32 current_focus);