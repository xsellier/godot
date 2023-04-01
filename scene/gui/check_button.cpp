/*************************************************************************/
/*  check_button.cpp                                                     */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

#include "check_button.h"

#include "print_string.h"
#include "servers/visual_server.h"

void CheckButton::_notification(int p_what) {

	if (p_what == NOTIFICATION_DRAW) {

		RID canvas_item = get_canvas_item();

		Ref<Texture> on_texture = Control::get_icon("on");
		Ref<Texture> off_texture = Control::get_icon("off");
		float style_maximum_size = get_stylebox("normal")->get_margin(MARGIN_LEFT);

		Vector2 offset;

		if (is_pressed()) {
			Size2 icon_size = on_texture->get_size();
			float icon_scale = MIN(1.0, MIN(get_size().height, style_maximum_size) / icon_size.height);

			offset.x = get_size().width - on_texture->get_width() * icon_scale;
			offset.y = int((get_size().height - on_texture->get_height() * icon_scale) / 2);

			on_texture->draw_rect(canvas_item, Rect2(offset, icon_size * icon_scale));

		} else {
			Size2 icon_size = off_texture->get_size();
			float icon_scale = MIN(1.0, MIN(get_size().height, style_maximum_size) / icon_size.height);

			offset.x = get_size().width - off_texture->get_width() * icon_scale;
			offset.y = int((get_size().height - off_texture->get_height() * icon_scale) / 2);

			off_texture->draw_rect(canvas_item, Rect2(offset, icon_size * icon_scale));
		}
	}
}

CheckButton::CheckButton() {

	set_toggle_mode(true);
	set_text_align(ALIGN_LEFT);
}

CheckButton::~CheckButton() {
}
