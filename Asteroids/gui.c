#include "gui.h"

bool button_update(BitMap rb, Button* b, ivec2 mouse_pos, bool lmb_down, bool lmb_was_down, i32* current_focus) {
	irect screen_rect = { .bl = pos_to_screen(b->pos_rect.bl, 0.05f, rb.height, rb.width),
						  .tr = pos_to_screen(b->pos_rect.tr, 0.05f, rb.height, rb.width) };
	if (lmb_down && point_inside_irect(mouse_pos, screen_rect)) {
		*current_focus = b->id;
		return true;
	}
	return false;
}

bool textbox_update(BitMap rb, TextBox* t, ivec2 mouse_pos, bool lmb_down, i32* current_focus, char key_pressed) {
	irect screen_rect = { .bl = pos_to_screen(t->pos_rect.bl, 0.05f, rb.height, rb.width),
					      .tr = pos_to_screen(t->pos_rect.tr, 0.05f, rb.height, rb.width) };

	// enter character if focused
	if (*current_focus == t->id && key_pressed != -1) {
		// backspace
		if (key_pressed == 0x08) {
			if (t->input_text_len > 0 && t->cursor_pos > 0) {
				memmove(&t->input_text[t->cursor_pos - 1], &t->input_text[t->cursor_pos], t->input_text_len - t->cursor_pos);
				t->input_text[--t->input_text_len] = '\0';
				t->cursor_pos--;
			}
		}
		// arrow left
		else if (key_pressed == 0x25) { if(t->cursor_pos > 0) t->cursor_pos--; } 
		// arrow right
		else if (key_pressed == 0x27) { if(t->cursor_pos < t->input_text_len) t->cursor_pos++;}

		// normal ascii code
		else if(t->input_text_len < MAX_NAME_LENGTH) {
			memmove(&t->input_text[t->cursor_pos + 1], &t->input_text[t->cursor_pos], t->input_text_len - t->cursor_pos);
			t->input_text[t->cursor_pos++] = key_pressed;
			t->input_text_len++;
		}
	}
	// check mouse click
	if (lmb_down && point_inside_irect(mouse_pos, screen_rect)) {
		f32 charsize = (rb.width / 1280.0f) * 4.0f;
		i32 x_mouse_relative = mouse_pos.x - screen_rect.bl.x;
		i32 cursor_index = x_mouse_relative / (charsize * 8);
		if (cursor_index >= t->input_text_len) t->cursor_pos = t->input_text_len;
		else t->cursor_pos = cursor_index;
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

static u64 cursor_blinking_timer = 0;
void textbox_render(BitMap rb, BitMap font,TextBox t, bool mouse_clicked, i32 current_focus, u64 dt) {
	irect textbox_rect = { .bl = pos_to_screen(t.pos_rect.bl, 0.05f, rb.height, rb.width),
						  .tr = pos_to_screen(t.pos_rect.tr, 0.05f, rb.height, rb.width) };
	if (current_focus == t.id) draw_rect(rb, textbox_rect, 0x0000FFFF);
	else draw_rect(rb, textbox_rect, 0x00FFFFFF);

	// if empty then draw default text
	if (t.input_text_len == 0 && current_focus != t.id) {
		vec2 font_size = { (rb.width / 1280.0f) * 4.0f, (rb.height / 720.0f) * 4.0f };
		ivec2 pos = { textbox_rect.bl.x + font_size.x * 0.5f * 8, textbox_rect.bl.y + font_size.y * 0.5f * 8 };
		draw_string(rb, font, pos, font_size, t.default_text, 0x00BDBDBD, textbox_rect);
	}
	// draw already written text
	else {
		vec2 font_size = { (rb.width / 1280.0f) * 4.0f, (rb.height / 720.0f) * 4.0f };
		ivec2 pos = { textbox_rect.bl.x + font_size.x * 0.5f * 8, textbox_rect.bl.y + font_size.y * 0.5f * 8 };
		draw_string(rb, font, pos, font_size, t.input_text, 0x00FFFFFF, textbox_rect);
		if (current_focus == t.id) {
			// cursor should be visible for CURSOR_BLINKING_TIME time and then invisible for CURSOR_BLINKING_TIME time and then reset
			cursor_blinking_timer += dt; 
			while (cursor_blinking_timer > 2.0f * CURSOR_BLINKING_TIME) cursor_blinking_timer -= 2.0f * CURSOR_BLINKING_TIME;
			if (cursor_blinking_timer < CURSOR_BLINKING_TIME) {
				ivec2 cursor_pos = { pos.x + (t.cursor_pos - 0.4) * font_size.x * 8, (pos.y) };
				vec2 cursor_size = { font_size.x * 0.8f, font_size.y * 1.4f };
				// HACK!!! because pipe symbol doesnt have its own 8 pixel square
				textbox_rect.tr.x = MIN(textbox_rect.tr.x, cursor_pos.x + font_size.x * 4);
				draw_character(rb, font, cursor_pos, cursor_size, 's' + 64, 0x005E5E5E, textbox_rect);
			}
		} 
	}

	// draw header text of textbox 
	irect screen_rect = { (ivec2) { 0, 0 }, (ivec2) { rb.width, rb.height } };
	vec2 font_size = { (rb.width / 1280.0f) * 3.0f, (rb.height / 720.0f) * 3.0f };
	draw_string(rb, font, (ivec2) { textbox_rect.bl.x, textbox_rect.tr.y }, font_size, t.header_string, 0x00BDBDBD, screen_rect);
}