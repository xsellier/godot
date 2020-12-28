/*************************************************************************/
/*  button.cpp                                                           */
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

#include "button.h"
#include "print_string.h"
#include "servers/visual_server.h"
#include "translation.h"

void Button::set_autowrap(bool p_autowrap) {

	if (autowrap != p_autowrap) {
		autowrap = p_autowrap;
		update();
		minimum_size_changed();
	}
}
bool Button::has_autowrap() const {

	return autowrap;
}

void Button::set_icon_scale(bool p_icon_scale) {

	if (icon_scale != p_icon_scale) {
		icon_scale = p_icon_scale;
		update();
		minimum_size_changed();
	}
}

void Button::set_modulate(const Color &p_tex) {

	if (modulate != p_tex) {
		modulate = p_tex;
		update();
	}
}

Color Button::get_modulate() const {

	return modulate;
}

bool Button::is_icon_scaling() const {

	return icon_scale;
}

Size2 Button::get_minimum_size() const {

	Size2 minsize = get_font("font")->get_string_size(text);
	if (clip_text)
		minsize.width = 0;

	Ref<Texture> _icon;
	if (icon.is_null() && has_icon("icon"))
		_icon = Control::get_icon("icon");
	else
		_icon = icon;

	if (!_icon.is_null()) {
		if (is_icon_scaling()) {
			Ref<StyleBox> style = get_stylebox("normal");
			Size2 size = get_size() - style->get_minimum_size();
			float icon_scale = MIN(size.x / _icon->get_width(), size.y / _icon->get_height());

			minsize.height = MAX(minsize.height, MIN(24.0, _icon->get_height() * icon_scale));
			minsize.width += MIN(24.0, _icon->get_width() * icon_scale);
		} else {
			minsize.height = MAX(minsize.height, _icon->get_height());
			minsize.width += _icon->get_width();
		}

		if (text != "")
			minsize.width += get_constant("hseparation");
	}

	return get_stylebox("normal")->get_minimum_size() + minsize;
}

void Button::_notification(int p_what) {

	if (p_what == NOTIFICATION_DRAW) {

		RID ci = get_canvas_item();
		Size2 size = get_size();
		Color color;
		Ref<Font> font = get_font("font");
		Size2 string_size = font->get_string_size(text);
		Ref<StyleBox> style = get_stylebox("normal");
		Size2 minimum_style_size = style->get_minimum_size();
		Ref<Texture> _icon;
		if (icon.is_null() && has_icon("icon"))
			_icon = Control::get_icon("icon");
		else
			_icon = icon;

		float icon_scale = 1.0;
		Point2 icon_ofs = Point2();

		if (!_icon.is_null()) {
			if (is_icon_scaling()) {
				icon_scale = MIN((size.width - minimum_style_size.x) / _icon->get_width(), (size.height - minimum_style_size.y) / _icon->get_height());
			}

			icon_ofs = Point2(_icon->get_width() * icon_scale + get_constant("hseparation"), 0);
		}

		Point2 stringPosition = size - minimum_style_size - icon_ofs - string_size;

		if (!clip_text && stringPosition.x < -0.1) {
			size.x += ABS(stringPosition.x);
			set_size(size);

			stringPosition = size - minimum_style_size - icon_ofs - string_size;
		}

		// Draw the background
		switch (get_draw_mode()) {

			case DRAW_NORMAL: {

				if (!flat)
					get_stylebox("normal")->draw(ci, Rect2(Point2(0, 0), size));
				color = get_color("font_color");
			} break;
			case DRAW_PRESSED: {

				get_stylebox("pressed")->draw(ci, Rect2(Point2(0, 0), size));
				if (has_color("font_color_pressed"))
					color = get_color("font_color_pressed");
				else
					color = get_color("font_color");

			} break;
			case DRAW_HOVER: {

				get_stylebox("hover")->draw(ci, Rect2(Point2(0, 0), size));
				color = get_color("font_color_hover");

			} break;
			case DRAW_DISABLED: {

				get_stylebox("disabled")->draw(ci, Rect2(Point2(0, 0), size));
				color = get_color("font_color_disabled");

			} break;
		}

		if (has_focus()) {

			Ref<StyleBox> focus_style = get_stylebox("focus");
			focus_style->draw(ci, Rect2(Point2(), size));
		}

		int text_clip = MAX(0, size.width - minimum_style_size.width - icon_ofs.width);

		if (autowrap) {
			float line_y_size = string_size.y;
			Point2 text_ofs = size - minimum_style_size - icon_ofs;
			Vector<String> splitted_text = text.split(" ");
			Size2 global_string_size = Size2(0.0, line_y_size);
			float text_y_size = line_y_size;
			Vector<String> lines;
			String current_line = "";
			String appended_string = "";

			for (int index = 0; index < splitted_text.size(); index++) {
				String current_string = appended_string + splitted_text[index];
				Size2 current_string_size = font->get_string_size(current_string);

				if (ceil(global_string_size.x + current_string_size.x) >= floor(text_ofs.x)) {
					global_string_size.x = 0;

					if (ceil(global_string_size.y + line_y_size) > floor(text_ofs.y)) {
						// Cannot fit the rest of the string
						lines.push_back(current_line + current_string);
						current_line = "";

						break;
					} else {
						lines.push_back(current_line);

						// Line spacing by default: 2 pixels.
						global_string_size.y += line_y_size + font->get_ascent();
						text_y_size += line_y_size;
						current_line = "" + splitted_text[index];
					}
				} else {
					current_line += current_string;
					global_string_size.x += current_string_size.x;

					appended_string = " ";
				}
			}

			// In order to be able to use it to compute the y offset
			global_string_size.x = 0;

			if (current_line.length() > 0) {
				lines.push_back(current_line);
			}

			// Reset the anchor offset point
			text_ofs = (size - minimum_style_size - icon_ofs) / 2.0;
			text_ofs.y -= (text_y_size - font->get_ascent()) / 2.0;
			float origin_x_offset = text_ofs.x;

			for (int index = 0; index < lines.size(); index++) {

				Size2 line_string_size = font->get_string_size(lines[index]);
				text_ofs -= line_string_size / 2.0;

				switch (align) {
					case ALIGN_LEFT: {
						text_ofs.x = style->get_margin(MARGIN_LEFT) + icon_ofs.x;
						text_ofs.y += style->get_offset().y;
					} break;
					case ALIGN_CENTER: {
						if (text_ofs.x < 0)
							text_ofs.x = 0;
						text_ofs += icon_ofs;
						text_ofs += style->get_offset();
					} break;
					case ALIGN_RIGHT: {
						text_ofs.x = MAX(icon_ofs.width + minimum_style_size.width, size.x - style->get_margin(MARGIN_RIGHT) - line_string_size.x);
						text_ofs.y += style->get_offset().y;
					} break;
				}

				text_ofs.y += font->get_ascent();
				font->draw(ci, text_ofs.floor(), lines[index], color, clip_text ? text_clip : -1);

				text_ofs.x = origin_x_offset;
				text_ofs.y += line_y_size;
			}

		} else {
			Point2 text_ofs = stringPosition / 2.0;

			switch (align) {
				case ALIGN_LEFT: {
					text_ofs.x = style->get_margin(MARGIN_LEFT) + icon_ofs.x;
					text_ofs.y += style->get_offset().y;
				} break;
				case ALIGN_CENTER: {
					if (text_ofs.x < 0)
						text_ofs.x = 0;
					text_ofs += icon_ofs;
					text_ofs += style->get_offset();
				} break;
				case ALIGN_RIGHT: {
					text_ofs.x = MAX(icon_ofs.width + minimum_style_size.width, size.x - style->get_margin(MARGIN_RIGHT) - string_size.x);
					text_ofs.y += style->get_offset().y;
				} break;
			}

			text_ofs.y += font->get_ascent();
			font->draw(ci, text_ofs.floor(), text, color, clip_text ? text_clip : -1);
		}

		if (!_icon.is_null()) {
			int valign = size.height - minimum_style_size.y;
			Color computed_modulation = modulate;

			if (is_disabled()) {
				computed_modulation.a = 0.4;
			}

			Size2 _icon_size = Size2(_icon->get_width() * icon_scale, _icon->get_height() * icon_scale);
			Point2 offset = style->get_offset() + Point2(0, Math::floor((valign - _icon_size.y) / 2.0));

			_icon->draw_rect(ci, Rect2(offset.x, offset.y, _icon_size.x, _icon_size.y), false, computed_modulation);
		}
	}
}

void Button::set_text(const String &p_text) {

	if (text == p_text)
		return;
	text = XL_MESSAGE(p_text);
	update();
	_change_notify("text");
	minimum_size_changed();
}
String Button::get_text() const {

	return text;
}

void Button::set_icon(const Ref<Texture> &p_icon) {

	if (icon == p_icon)
		return;
	icon = p_icon;
	update();
	_change_notify("icon");
	minimum_size_changed();
}

Ref<Texture> Button::get_icon() const {

	return icon;
}

void Button::set_flat(bool p_flat) {

	if (flat != p_flat) {
		flat = p_flat;
		update();
		_change_notify("flat");
	}
}

bool Button::is_flat() const {

	return flat;
}

void Button::set_clip_text(bool p_clip_text) {

	if (clip_text != p_clip_text) {
		clip_text = p_clip_text;
		update();
		minimum_size_changed();
	}
}

bool Button::get_clip_text() const {

	return clip_text;
}

void Button::set_text_align(TextAlign p_align) {

	if (align != p_align) {
		align = p_align;
		update();
	}
}

Button::TextAlign Button::get_text_align() const {

	return align;
}

void Button::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_text", "text"), &Button::set_text);
	ObjectTypeDB::bind_method(_MD("get_text"), &Button::get_text);
	ObjectTypeDB::bind_method(_MD("set_button_icon", "texture:Texture"), &Button::set_icon);
	ObjectTypeDB::bind_method(_MD("get_button_icon:Texture"), &Button::get_icon);
	ObjectTypeDB::bind_method(_MD("set_flat", "enabled"), &Button::set_flat);
	ObjectTypeDB::bind_method(_MD("set_icon_scale", "enable"), &Button::set_icon_scale);
	ObjectTypeDB::bind_method(_MD("is_icon_scaling"), &Button::is_icon_scaling);
	ObjectTypeDB::bind_method(_MD("set_icon_modulate", "modulate"), &Button::set_modulate);
	ObjectTypeDB::bind_method(_MD("get_icon_modulate"), &Button::get_modulate);
	ObjectTypeDB::bind_method(_MD("set_autowrap", "enable"), &Button::set_autowrap);
	ObjectTypeDB::bind_method(_MD("has_autowrap"), &Button::has_autowrap);
	ObjectTypeDB::bind_method(_MD("set_clip_text", "enabled"), &Button::set_clip_text);
	ObjectTypeDB::bind_method(_MD("get_clip_text"), &Button::get_clip_text);
	ObjectTypeDB::bind_method(_MD("set_text_align", "align"), &Button::set_text_align);
	ObjectTypeDB::bind_method(_MD("get_text_align"), &Button::get_text_align);
	ObjectTypeDB::bind_method(_MD("is_flat"), &Button::is_flat);

	BIND_CONSTANT(ALIGN_LEFT);
	BIND_CONSTANT(ALIGN_CENTER);
	BIND_CONSTANT(ALIGN_RIGHT);

	ADD_PROPERTYNZ(PropertyInfo(Variant::STRING, "text", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), _SCS("set_text"), _SCS("get_text"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture"), _SCS("set_button_icon"), _SCS("get_button_icon"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "icon_scale"), _SCS("set_icon_scale"), _SCS("is_icon_scaling"));
	ADD_PROPERTYNO(PropertyInfo(Variant::COLOR, "modulate"), _SCS("set_icon_modulate"), _SCS("get_icon_modulate"));
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat"), _SCS("set_flat"), _SCS("is_flat"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "autowrap"), _SCS("set_autowrap"), _SCS("has_autowrap"));
	ADD_PROPERTYNZ(PropertyInfo(Variant::BOOL, "clip_text"), _SCS("set_clip_text"), _SCS("get_clip_text"));
	ADD_PROPERTYNO(PropertyInfo(Variant::INT, "align", PROPERTY_HINT_ENUM, "Left,Center,Right"), _SCS("set_text_align"), _SCS("get_text_align"));
}

Button::Button(const String &p_text) {

	flat = false;
	clip_text = false;
	autowrap = false;
	icon_scale = false;
	modulate = Color(1, 1, 1, 1);
	set_stop_mouse(true);
	set_text(p_text);
	align = ALIGN_CENTER;
}

Button::~Button() {
}
