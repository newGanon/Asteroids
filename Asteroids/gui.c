#include "gui.h"

bool button_update(BitMap rb, Button* b, ivec2 mouse_pos, bool mouse_clicked, i32* current_focus) {
	irect screen_rect = { .bl = pos_to_screen(b->pos_rect.bl, 0.05f, rb.height, rb.width),
						  .tr = pos_to_screen(b->pos_rect.tr, 0.05f, rb.height, rb.width) };
	if (mouse_clicked && point_inside_irect(mouse_pos, screen_rect)) {
		*current_focus = b->id;
		return true;
	}
	return false;
}

bool textbox_update(BitMap rb, TextBox* t, ivec2 mouse_pos, bool mouse_clicked, i32* current_focus, char key_pressed) {
	irect screen_rect = { .bl = pos_to_screen(t->pos_rect.bl, 0.05f, rb.height, rb.width),
					      .tr = pos_to_screen(t->pos_rect.tr, 0.05f, rb.height, rb.width) };

	// enter character if focused
	if (*current_focus == t->id && key_pressed != -1) {
		// backspace
		if (key_pressed == 8) {
			if (t->input_text_len > 0) {
				t->input_text[--t->input_text_len] = '\0';
			}
		}
		// normal ascii code
		else if (t->input_text_len < MAX_NAME_LENGTH) {
			t->input_text[t->input_text_len++] = key_pressed;
		}
	}
	// check mouse click
	if (mouse_clicked && point_inside_irect(mouse_pos, screen_rect)) {
		*current_focus = t->id;
		return true;
	}
	return false;
}

void button_render(BitMap rb, BitMap font, Button b, bool mouse_clicked, i32 current_focus) {
	irect button_rect = { .bl = pos_to_screen(b.pos_rect.bl, 0.05f, rb.height, rb.width),
					      .tr = pos_to_screen(b.pos_rect.tr, 0.05f, rb.height, rb.width) };
	if(current_focus == b.id) draw_rect(rb, button_rect, 0x0000FFFF);
	else draw_rect(rb, button_rect, 0x00FFFFFF);

	//i32 text_length = strlen(b.default_text);
	//vec2 character_size = { (f32)(button_rect.tr.x - button_rect.bl.x) / text_length, (f32)(button_rect.tr.y - button_rect.bl.y)};
	//vec2 font_size = { (character_size.x / 8.0f), (character_size.y/ 8.0f) };
	vec2 font_size = { (rb.width / 1280.0f) * 4.0f, (rb.height / 720.0f) * 4.0f };
	i32 text_length = strlen(b.default_text);
	ivec2 pos = { button_rect.bl.x + (button_rect.tr.x - button_rect.bl.x) /2.0f - (font_size.x * 8 * text_length) / 2.0f, button_rect.bl.y + font_size.x * 0.5f * 8};
	draw_string(rb, font, pos, font_size, b.default_text, 0x00FFFFFF, button_rect);
}

void textbox_render(BitMap rb, BitMap font,TextBox t, bool mouse_clicked, i32 current_focus) {
	irect textbox_rect = { .bl = pos_to_screen(t.pos_rect.bl, 0.05f, rb.height, rb.width),
						  .tr = pos_to_screen(t.pos_rect.tr, 0.05f, rb.height, rb.width) };
	if (current_focus == t.id) draw_rect(rb, textbox_rect, 0x0000FFFF);
	else draw_rect(rb, textbox_rect, 0x00FFFFFF);

	// if empty then draw default text
	if (t.input_text_len == 0 && current_focus != t.id) {
		//i32 text_length = strlen(t.default_text);
		//vec2 character_size = { (f32)(textbox_rect.tr.x - textbox_rect.bl.x) / text_length, (f32)(textbox_rect.tr.y - textbox_rect.bl.y) };
		//vec2 font_size = { (character_size.x / 8.0f), (character_size.y / 8.0f) };
		vec2 font_size = { (rb.width / 1280.0f) * 4.0f, (rb.height / 720.0f) * 4.0f };
		ivec2 pos = { textbox_rect.bl.x + font_size.x * 0.5f * 8, textbox_rect.bl.y + font_size.x * 0.5f * 8 };
		draw_string(rb, font, pos, font_size, t.default_text, 0x00BDBDBD, textbox_rect);
	}
	// draw already written text
	else {
		//i32 text_length = strlen(t.input_text);
		//vec2 character_size = { (f32)(textbox_rect.tr.x - textbox_rect.bl.x) / text_length, (f32)(textbox_rect.tr.y - textbox_rect.bl.y) };
		//vec2 font_size = { (character_size.x / 8.0f), (character_size.y / 8.0f) };
		vec2 font_size = { (rb.width / 1280.0f) * 4.0f, (rb.height / 720.0f) * 4.0f };
		ivec2 pos = { textbox_rect.bl.x + font_size.x * 0.5f * 8, textbox_rect.bl.y + font_size.x * 0.5f * 8 };
		draw_string(rb, font, pos, font_size, t.input_text, 0x00FFFFFF, textbox_rect);
	}


	// draw header text of textbox 
	irect screen_rect = { (ivec2) { 0, 0 }, (ivec2) { rb.width, rb.height } };
	vec2 font_size = { (rb.width / 1280.0f) * 3.0f, (rb.height / 720.0f) * 3.0f };
	draw_string(rb, font, (ivec2) { textbox_rect.bl.x, textbox_rect.tr.y }, font_size, t.header_string, 0x00BDBDBD, screen_rect);
}