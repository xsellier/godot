/**************************************************************************/
/*  check_box.cpp                                                         */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "check_box.h"

#include "scene/theme/theme_db.h"

Size2 CheckBox::get_icon_size() const {
	Size2 tex_size = Size2(0, 0);
	if (theme_cache.checked.is_valid()) {
		tex_size = theme_cache.checked->get_size();
	}
	if (theme_cache.unchecked.is_valid()) {
		tex_size = tex_size.max(theme_cache.unchecked->get_size());
	}
	if (theme_cache.radio_checked.is_valid()) {
		tex_size = tex_size.max(theme_cache.radio_checked->get_size());
	}
	if (theme_cache.radio_unchecked.is_valid()) {
		tex_size = tex_size.max(theme_cache.radio_unchecked->get_size());
	}
	if (theme_cache.checked_disabled.is_valid()) {
		tex_size = tex_size.max(theme_cache.checked_disabled->get_size());
	}
	if (theme_cache.unchecked_disabled.is_valid()) {
		tex_size = tex_size.max(theme_cache.unchecked_disabled->get_size());
	}
	if (theme_cache.radio_checked_disabled.is_valid()) {
		tex_size = tex_size.max(theme_cache.radio_checked_disabled->get_size());
	}
	if (theme_cache.radio_unchecked_disabled.is_valid()) {
		tex_size = tex_size.max(theme_cache.radio_unchecked_disabled->get_size());
	}

	float texture_scale = 1.0;
	
	if (!theme_cache.normal_style.is_null()) {
		float computed_checkbox_height = get_size().height - (theme_cache.normal_style->get_margin(SIDE_TOP) + theme_cache.normal_style->get_margin(SIDE_BOTTOM));
	
		texture_scale = (tex_size.height > 0) ? MIN(1.0, computed_checkbox_height / tex_size.height) : 1.0;
	}
	
	return _fit_icon_size(tex_size * texture_scale);
}

Size2 CheckBox::get_minimum_size() const {
	Size2 minsize = Button::get_minimum_size();
	const Size2 tex_size = get_icon_size();
	if (tex_size.width > 0 || tex_size.height > 0) {
		const Size2 padding = _get_largest_stylebox_size();
		Size2 content_size = minsize - padding;
		if (content_size.width > 0 && tex_size.width > 0) {
			content_size.width += MAX(0, theme_cache.h_separation);
		}
		content_size.width += tex_size.width;
		content_size.height = MAX(content_size.height, tex_size.height);

		minsize = content_size + padding;
	}

	return minsize;
}

void CheckBox::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_THEME_CHANGED:
		case NOTIFICATION_LAYOUT_DIRECTION_CHANGED:
		case NOTIFICATION_TRANSLATION_CHANGED: {
			if (is_layout_rtl()) {
				_set_internal_margin(SIDE_LEFT, 0.f);
				_set_internal_margin(SIDE_RIGHT, get_icon_size().width);
			} else {
				_set_internal_margin(SIDE_LEFT, get_icon_size().width);
				_set_internal_margin(SIDE_RIGHT, 0.f);
			}
		} break;

		case NOTIFICATION_DRAW: {
			RID ci = get_canvas_item();

			Ref<Texture2D> on_texture;
			Ref<Texture2D> off_texture;

			if (is_radio()) {
				if (is_disabled()) {
					on_texture = theme_cache.radio_checked_disabled;
					off_texture = theme_cache.radio_unchecked_disabled;
				} else {
					on_texture = theme_cache.radio_checked;
					off_texture = theme_cache.radio_unchecked;
				}
			} else {
				if (is_disabled()) {
					on_texture = theme_cache.checked_disabled;
					off_texture = theme_cache.unchecked_disabled;
				} else {
					on_texture = theme_cache.checked;
					off_texture = theme_cache.unchecked;
				}
			}

			Vector2 offset;
			if (is_layout_rtl()) {
				offset.x = get_size().x - theme_cache.normal_style->get_margin(SIDE_RIGHT) - get_icon_size().width;
			} else {
				offset.x = theme_cache.normal_style->get_margin(SIDE_LEFT);
			}

			float computed_checkbox_height = get_size().height - (theme_cache.normal_style->get_margin(SIDE_TOP) + theme_cache.normal_style->get_margin(SIDE_BOTTOM));

			if (is_pressed()) {
				Size2 texture_size = on_texture->get_size();
				float texture_scale = MIN(1.0, computed_checkbox_height / texture_size.height);

				offset.y = int((get_size().height - texture_scale * on_texture->get_height()) / 2);
				on_texture->draw_rect(ci, Rect2(offset, _fit_icon_size(texture_size * texture_scale)));
			} else {
				Size2 texture_size = off_texture->get_size();
				float texture_scale = MIN(1.0, computed_checkbox_height / texture_size.height);
				
				offset.y = int((get_size().height - texture_scale * off_texture->get_height()) / 2);
				off_texture->draw_rect(ci, Rect2(offset, _fit_icon_size(texture_size * texture_scale)));
			}
		} break;
	}
}

bool CheckBox::is_radio() {
	return get_button_group().is_valid();
}

void CheckBox::_bind_methods() {
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, CheckBox, h_separation);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, CheckBox, check_v_offset);
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, CheckBox, normal_style, "normal");

	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, checked);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, unchecked);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, radio_checked);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, radio_unchecked);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, checked_disabled);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, unchecked_disabled);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, radio_checked_disabled);
	BIND_THEME_ITEM(Theme::DATA_TYPE_ICON, CheckBox, radio_unchecked_disabled);
}

CheckBox::CheckBox(const String &p_text) :
		Button(p_text) {
	set_toggle_mode(true);

	set_text_alignment(HORIZONTAL_ALIGNMENT_LEFT);

	if (is_layout_rtl()) {
		_set_internal_margin(SIDE_RIGHT, get_icon_size().width);
	} else {
		_set_internal_margin(SIDE_LEFT, get_icon_size().width);
	}
}

CheckBox::~CheckBox() {
}
