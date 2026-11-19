/*************************************************************************/
/*  editor_export_godot3.cpp                                             */
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

#include "editor_export_godot3.h"

#include "editor/editor_import_export.h"
#include "editor_node.h"
#include "io/resource_format_binary.h"
#include "io/resource_format_xml.h"
#include "scene/io/resource_format_image.h"
#include "scene/resources/packed_scene.h"
#include "scene/resources/scene_format_text.h"
#include "scene/resources/texture.h"

static const char *globals_renames[][2] = {
	/* [application] */
	{ "application/name", "application/config/name" },
	{ "application/auto_accept_quit", "application/config/auto_accept_quit" },
	{ "application/boot_splash", "application/boot_splash/image" },
	{ "application/boot_splash_fullsize", "application/boot_splash/fullsize" },
	{ "application/icon", "application/config/icon" },
	{ "application/main_scene", "application/run/main_scene" },
	{ "application/main_loop_type", "application/run/main_loop_type" },
	{ "application/disable_stdout", "application/run/disable_stdout" },
	{ "application/disable_stderr", "application/run/disable_stderr" },
	{ "application/frame_delay_msec", "application/run/frame_delay_msec" },

	/* [debug] */
	{ "debug/profiler_max_functions", "debug/settings/profiler/max_functions" },
	{ "debug/max_remote_stdout_chars_per_second", "network/limits/debugger_stdout/max_chars_per_second" },
	{ "debug/force_fps", "debug/settings/fps/force_fps" },
	{ "debug/verbose_stdout", "debug/settings/stdout/verbose_stdout" },
	//{ "debug/max_texture_size", "debug/" },
	//{ "debug/max_texture_size_alert", "debug/" },
	//{ "debug/image_load_times", "debug/" },
	{ "debug/script_max_call_stack", "debug/settings/gdscript/max_call_stack" },
	{ "debug/collision_shape_color", "debug/shapes/collision/shape_color" },
	{ "debug/collision_contact_color", "debug/shapes/collision/contact_color" },
	{ "debug/navigation_geometry_color", "debug/shapes/navigation/geometry_color" },
	{ "debug/navigation_disabled_geometry_color", "debug/shapes/navigation/disabled_geometry_color" },
	{ "debug/collision_max_contacts_displayed", "debug/shapes/collision/max_contacts_displayed" },
	//{ "debug/indicators_enabled", "debug/" },
	{ "debug/print_fps", "debug/settings/stdout/print_fps" },
	//{ "debug/print_metrics", "debug/" },

	/* [display] */
	{ "display/driver", "display/driver/name" },
	{ "display/width", "display/window/size/width" },
	{ "display/height", "display/window/size/height" },
	{ "display/allow_hidpi", "display/window/dpi/allow_hidpi" },
	{ "display/fullscreen", "display/window/size/fullscreen" },
	{ "display/resizable", "display/window/size/resizable" },
	{ "display/borderless_window", "display/window/size/borderless" },
	{ "display/use_vsync", "display/window/vsync/use_vsync" },
	{ "display/test_width", "display/window/size/test_width" },
	{ "display/test_height", "display/window/size/test_height" },
	{ "display/use_2d_pixel_snap", "rendering/quality/2d/use_pixel_snap" },
	{ "display/keep_screen_on", "display/window/energy_saving/keep_screen_on" },
	{ "display/orientation", "display/window/handheld/orientation" },
	{ "display/emulate_touchscreen", "display/window/handheld/emulate_touchscreen" },
	{ "display/use_hidpi_theme", "gui/theme/use_hidpi" },
	{ "display/custom_theme", "gui/theme/custom" },
	{ "display/custom_theme_font", "gui/theme/custom_font" },
	{ "display/swap_ok_cancel", "gui/common/swap_ok_cancel" },
	{ "display/tooltip_delay", "gui/timers/tooltip_delay_sec" },
	{ "display/text_edit_idle_detect_sec", "gui/timers/text_edit_idle_detect_sec" },
	{ "display/stretch_mode", "display/window/stretch/mode" },
	{ "display/stretch_aspect", "display/window/stretch/aspect" },

	/* [render] */
	{ "render/thread_model", "rendering/threads/thread_model" },
	//{ "render/mipmap_policy", "" },
	//{ "render/thread_textures_prealloc", "" },
	//{ "render/shadows_enabled", "" },
	//{ "render/aabb_random_points", "" },
	{ "render/default_clear_color", "rendering/environment/default_clear_color" },
	//{ "render/room_cull_enabled", "" },
	//{ "render/light_discard_enabled", "" },

	/* [audio] */
	// partly unchanged
	//{ "audio/mixer_interp", "" },
	//{ "audio/use_chorus_reverb", "" },
	//{ "audio/stream_volume_scale", "" },
	//{ "audio/fx_volume_scale", "" },
	//{ "audio/event_voice_volume_scale", "" },
	//{ "audio/stream_buffering_ms", "" },
	//{ "audio/video_delay_compensation_ms", "" },
	//{ "audio/mixer_latency", "" },

	/* [physics] */
	{ "physics/fixed_fps", "physics/common/physics_fps" },
	{ "physics/sleep_threshold_linear", "physics/3d/sleep_threshold_linear" },
	{ "physics/sleep_threshold_angular", "physics/3d/sleep_threshold_angular" },
	{ "physics/time_before_sleep", "physics/3d/time_before_sleep" },
	{ "physics/default_gravity", "physics/3d/default_gravity" },
	{ "physics/default_gravity_vector", "physics/3d/default_gravity_vector" },
	{ "physics/default_linear_damp", "physics/3d/default_linear_damp" },
	{ "physics/default_angular_damp", "physics/3d/default_angular_damp" },
	{ "physics/enable_object_picking", "physics/common/enable_object_picking" },

	/* [core] */
	//{ "core/message_queue_size_kb", "" },
	//{ "core/rid_pool_prealloc", "" },
	//{ "core/thread_rid_pool_prealloc", "" },
	{ "core/packet_stream_peer_max_buffer_po2", "network/limits/packet_peer_stream/max_buffer_po2" },

	/* [rasterizer.Android] */
	//{ "rasterizer.Android/use_fragment_lighting", "" },
	//{ "rasterizer.Android/fp16_framebuffer", "" },

	/* [display.Android] */
	//{ "display.Android/driver", "" },

	/* [rasterizer.iOS] */
	//{ "rasterizer.iOS/use_fragment_lighting", "" },
	//{ "rasterizer.iOS/fp16_framebuffer", "" },

	/* [display.iOS] */
	//{ "display.iOS/driver", "" },
	//{ "display.iOS/use_cadisplaylink", "" },

	/* [rasterizer] */
	// most don't have an equivalent or are not meaningful to port
	{ "rasterizer/anisotropic_filter_level", "rendering/quality/filter/anisotropic_filter_level" },

	/* [physics_2d] */
	{ "physics_2d/thread_model", "physics/2d/thread_model" },
	//{ "physics_2d/motion_fix_enabled", "" },
	{ "physics_2d/sleep_threashold_linear", "physics/2d/sleep_threshold_linear" },
	{ "physics_2d/sleep_threshold_angular", "physics/2d/sleep_threshold_angular" },
	{ "physics_2d/time_before_sleep", "physics/2d/time_before_sleep" },
	{ "physics_2d/bp_hash_table_size", "physics/2d/bp_hash_table_size" },
	{ "physics_2d/cell_size", "physics/2d/cell_size" },
	{ "physics_2d/large_object_surface_treshold_in_cells", "physics/2d/large_object_surface_threshold_in_cells" },
	{ "physics_2d/default_gravity", "physics/2d/default_gravity" },
	{ "physics_2d/default_gravity_vector", "physics/2d/default_gravity_vector" },
	{ "physics_2d/default_linear_damp", "physics/2d/default_linear_damp" },
	{ "physics_2d/default_angular_damp", "physics/2d/default_angular_damp" },

	/* [image_loader] */
	//{ "image_loader/filter", "" },
	//{ "image_loader/gen_mipmaps", "" },
	//{ "image_loader/repeat", "" },

	/* [ssl] */
	{ "ssl/certificates", "network/ssl/certificates" },
	{ "ssl/config", "network/ssl/config" },

	/* [locale] */
	// no change

	/* [global] */
	{ "editor_active", "editor/active" },

	/* [editor] */
	{ "editor/main_run_args", "editor/main_run_args" },
	//{ "editor/import_shared_textures", "" },

	/* [gui] */
	{ "gui/incr_search_max_interval_msec", "gui/timers/incremental_search_max_interval_msec" },

	{ NULL, NULL }
};

static const char *prop_renames[][2] = {
	{ "anchor/bottom", "anchor_bottom" }, // Control
	{ "anchor/left", "anchor_left" }, // Control
	{ "anchor/right", "anchor_right" }, // Control
	{ "anchor/top", "anchor_top" }, // Control
	{ "bbcode/bbcode", "bbcode_text" }, // RichTextLabel
	{ "bbcode/enabled", "bbcode_enabled" }, // RichTextLabel
	{ "bias/bias", "bias" }, // Joints2D
	{ "caret/block_caret", "caret_block_mode" }, // TextEdit
	{ "caret/caret_blink", "caret_blink" }, // LineEdit, TextEdit
	{ "caret/caret_blink_speed", "caret_blink_speed" }, // LineEdit, TextEdit
	{ "cell/center_x", "cell_center_x" }, // GridMap
	{ "cell/center_y", "cell_center_y" }, // GridMap
	{ "cell/center_z", "cell_center_z" }, // GridMap
	{ "cell/custom_transform", "cell_custom_transform" }, // TileMap
	{ "cell/half_offset", "cell_half_offset" }, // TileMap
	{ "cell/octant_size", "cell_octant_size" }, // GridMap
	{ "cell/quadrant_size", "cell_quadrant_size" }, // TileMap
	{ "cell/scale", "cell_scale" }, // GridMap
	{ "cell/size", "cell_size" }, // GridMap, TileMap
	{ "cell/tile_origin", "cell_tile_origin" }, // TileMap
	{ "cell/y_sort", "cell_y_sort" }, // TileMap
	{ "collision/bounce", "collision_bounce" }, // TileMap
	//{ "collision/exclude_nodes", "disable_collision" }, // Joint, Joint2D // Joint2D can be converted, not Joint, handle manually
	{ "collision/friction", "collision_friction" }, // TileMap
	{ "collision/layers", "collision_layer" }, // Area, Area2D, PhysicsBody, PhysicsBody2D, TileMap
	{ "collision/margin", "collision/safe_margin" }, // PhysicsBody, PhysicsBody2D
	{ "collision/mask", "collision_mask" }, // Area, Area2D, PhysicsBody, PhysicsBody2D, TileMap
	{ "collision/use_kinematic", "collision_use_kinematic" }, // TileMap
	{ "config/amount", "amount" }, // Particles2D
	{ "config/emitting", "emitting" }, // Particles2D
	{ "config/explosiveness", "explosiveness" }, // Particles2D
	{ "config/h_frames", "h_frames" }, // Particles2D
	{ "config/lifetime", "lifetime" }, // Particles2D
	{ "config/local_space", "local_coords" }, // Particles2D
	{ "config/preprocess", "preprocess" }, // Particles2D
	{ "config/texture", "texture" }, // Particles2D
	{ "config/time_scale", "speed_scale" }, // Particles2D
	{ "config/v_frames", "v_frames" }, // Particles2D
	{ "content_margin/bottom", "content_margin_bottom" }, // StyleBox
	{ "content_margin/left", "content_margin_left" }, // StyleBox
	{ "content_margin/right", "content_margin_right" }, // StyleBox
	{ "content_margin/top", "content_margin_top" }, // StyleBox
	{ "damping/compression", "damping_compression" }, // VehicleWheel
	{ "damping/relaxation", "damping_relaxation" }, // VehicleWheel
	{ "damp_override/angular", "angular_damp" }, // PhysicsBody, PhysicsBody2D
	{ "damp_override/linear", "linear_damp" }, // PhysicsBody, PhysicsBody2D
	{ "dialog/hide_on_ok", "dialog_hide_on_ok" }, // AcceptDialog
	{ "dialog/text", "dialog_text" }, // AcceptDialog
	{ "drag_margin/bottom", "drag_margin_bottom" }, // Camera2D
	{ "drag_margin/h_enabled", "drag_margin_h_enabled" }, // Camera2D
	{ "drag_margin/left", "drag_margin_left" }, // Camera2D
	{ "drag_margin/right", "drag_margin_right" }, // Camera2D
	{ "drag_margin/top", "drag_margin_top" }, // Camera2D
	{ "drag_margin/v_enabled", "drag_margin_v_enabled" }, // Camera2D
	{ "enabler/fixed_process_parent", "physics_process_parent" }, // VisibilityEnabler2D
	{ "enabler/freeze_bodies", "freeze_bodies" }, // VisibilityEnabler, VisibilityEnabler2D
	{ "enabler/pause_animated_sprites", "pause_animated_sprites" }, // VisibilityEnabler2D
	{ "enabler/pause_animations", "pause_animations" }, // VisibilityEnabler, VisibilityEnabler2D
	{ "enabler/pause_particles", "pause_particles" }, // VisibilityEnabler2D
	{ "enabler/process_parent", "process_parent" }, // VisibilityEnabler2D
	{ "expand_margin/bottom", "expand_margin_bottom" }, // StyleBox
	{ "expand_margin/left", "expand_margin_left" }, // StyleBox
	{ "expand_margin/right", "expand_margin_right" }, // StyleBox
	{ "expand_margin/top", "expand_margin_top" }, // StyleBox
	{ "extra_spacing/bottom", "extra_spacing_bottom" }, // DynamicFont
	{ "extra_spacing/char", "extra_spacing_char" }, // DynamicFont
	{ "extra_spacing/space", "extra_spacing_space" }, // DynamicFont
	{ "extra_spacing/top", "extra_spacing_top" }, // DynamicFont
	{ "flags/alpha_cut", "alpha_cut" }, // Sprite3D
	{ "flags/double_sided", "double_sided" }, // Sprite3D
	{ "flags/shaded", "shaded" }, // Sprite3D
	{ "flags/transparent", "transparent" }, // Sprite3D
	{ "focus_neighbour/bottom", "focus_neighbour_bottom" }, // Control
	{ "focus_neighbour/left", "focus_neighbour_left" }, // Control
	{ "focus_neighbour/right", "focus_neighbour_right" }, // Control
	{ "focus_neighbour/top", "focus_neighbour_top" }, // Control
	{ "font/font", "font_data" }, // DynamicFont
	{ "font/size", "size" }, // DynamicFont
	{ "font/use_filter", "use_filter" }, // DynamicFont
	{ "font/use_mipmaps", "use_mipmaps" }, // DynamicFont
	{ "geometry/cast_shadow", "cast_shadow" }, // GeometryInstance
	{ "geometry/extra_cull_margin", "extra_cull_margin" }, // GeometryInstance
	{ "geometry/material_override", "material_override" }, // GeometryInstance
	{ "geometry/use_baked_light", "use_in_baked_light" }, // GeometryInstance
	{ "hint/tooltip", "hint_tooltip" }, // Control
	{ "input/capture_on_drag", "input_capture_on_drag" }, // CollisionObject
	{ "input/pickable", "input_pickable" }, // CollisionObject2D
	{ "input/ray_pickable", "input_ray_pickable" }, // CollisionObject
	{ "invert/border", "invert_border" }, // Polygon2D
	{ "invert/enable", "invert_enable" }, // Polygon2D
	{ "is_pressed", "pressed" }, // BaseButton
	{ "limit/bottom", "limit_bottom" }, // Camera2D
	{ "limit/left", "limit_left" }, // Camera2D
	{ "limit/right", "limit_right" }, // Camera2D
	{ "limit/top", "limit_top" }, // Camera2D
	{ "margin/bottom", "margin_bottom" }, // Control, StyleBox
	{ "margin/left", "margin_left" }, // Control, StyleBox
	{ "margin/right", "margin_right" }, // Control, StyleBox
	{ "margin/top", "margin_top" }, // Control, StyleBox
	{ "material/material", "material" }, // CanvasItem
	{ "material/use_parent", "use_parent_material" }, // CanvasItem
	{ "mesh/mesh", "mesh" }, // MeshInstance
	{ "mesh/skeleton", "skeleton" }, // MeshInstance
	//{ "mode", "fill_mode" }, // TextureProgress & others // Would break TileMap and others, handle manually
	{ "motion/brake", "brake" }, // VehicleBody
	{ "motion/engine_force", "engine_force" }, // VehicleBody
	{ "motion/mirroring", "motion_mirroring" }, // ParallaxLayer
	{ "motion/offset", "motion_offset" }, // ParallaxLayer
	{ "motion/scale", "motion_scale" }, // ParallaxLayer
	{ "motion/steering", "steering" }, // VehicleBody
	{ "occluder/light_mask", "occluder_light_mask" }, // TileMap
	{ "params/attenuation/distance_exp", "attenuation_distance_exp" },
	{ "params/attenuation/max_distance", "attenuation_max_distance" },
	{ "params/attenuation/min_distance", "attenuation_min_distance" },
	{ "params/emission_cone/attenuation_db", "emission_cone_attenuation_db" },
	{ "params/emission_cone/degrees", "emission_cone_degrees" },
	{ "params/modulate", "self_modulate" },
	{ "params/pitch_scale", "pitch_scale" },
	{ "params/scale", "texture_scale" },
	{ "params/volume_db", "volume_db" },
	{ "patch_margin/bottom", "patch_margin_bottom" }, // Patch9Frame
	{ "patch_margin/left", "patch_margin_left" }, // Patch9Frame
	{ "patch_margin/right", "patch_margin_right" }, // Patch9Frame
	{ "patch_margin/top", "patch_margin_top" }, // Patch9Frame
	{ "percent/visible", "percent_visible" }, // ProgressBar
	{ "placeholder/alpha", "placeholder_alpha" }, // LineEdit
	{ "placeholder/text", "placeholder_text" }, // LineEdit
	//{ "playback/active", "playback_active" }, // AnimationPlayer, AnimationTreePlayer // properly renamed for AnimationPlayer, but not AnimationTreePlayer, handle manually
	{ "playback/default_blend_time", "playback_default_blend_time" }, // AnimationPlayer
	{ "playback/process_mode", "playback_process_mode" }, // AnimationPlayer, AnimationTreePlayer, Tween
	{ "playback/speed", "playback_speed" }, // AnimationPlayer, Tween
	{ "playback/repeat", "repeat" }, // Tween
	{ "playback/autoplay", "autoplay" }, // AnimationPlayer
	{ "playback/play", "current_animation" }, // AnimationPlayer
	{ "popup/exclusive", "popup_exclusive" }, // Popup
	{ "process/pause_mode", "pause_mode" }, // Node
	{ "radial_fill/center_offset", "radial_center_offset" }, // TextureProgress
	{ "radial_fill/fill_degrees", "radial_fill_degrees" }, // TextureProgress
	{ "radial_fill/initial_angle", "radial_initial_angle" }, // TextureProgress
	{ "range/exp_edit", "exp_edit" }, // Range
	{ "range/height", "range_height" }, // Light2D
	{ "range/item_mask", "range_item_cull_mask" }, // Light2D
	{ "range/layer_max", "range_layer_max" }, // Light2D
	{ "range/layer_min", "range_layer_min" }, // Light2D
	{ "range/max", "max_value" }, // Range
	{ "range/min", "min_value" }, // Range
	{ "range/page", "page" }, // Range
	{ "range/rounded", "rounded" }, // Range
	{ "range/step", "step" }, // Range
	{ "range/value", "value" }, // Range
	{ "range/z_max", "range_z_max" }, // Light2D
	{ "range/z_min", "range_z_min" }, // Light2D
	{ "rect/min_size", "rect_min_size" }, // Control
	{ "rect/pos", "rect_position" }, // Control
	{ "rect/rotation", "rect_rotation" }, // Control
	{ "rect/scale", "rect_scale" }, // Control
	{ "rect/size", "rect_size" }, // Control
	//{ "region", "region_enabled" }, // Sprite, Sprite3D // Not renamed for Texture, handle manually
	{ "resource/name", "resource_name" }, // Resource
	{ "resource/path", "resource_path" }, // Resource
	{ "root/root", "root_node" }, // AnimationPlayer
	{ "script/script", "script" }, // Object
	{ "scroll/base_offset", "scroll_base_offset" }, // ParallaxBackground
	{ "scroll/base_scale", "scroll_base_scale" }, // ParallaxBackground
	{ "scroll/horizontal", "scroll_horizontal_enabled" }, // ScrollContainer
	{ "scroll/ignore_camera_zoom", "scroll_ignore_camera_zoom" }, // ParallaxBackground
	{ "scroll/deadzone", "scroll_deadzone" }, // ScrollContainer
	{ "scroll/limit_begin", "scroll_limit_begin" }, // ParallaxBackground
	{ "scroll/limit_end", "scroll_limit_end" }, // ParallaxBackground
	{ "scroll/offset", "scroll_offset" }, // ParallaxBackground
	{ "scroll/vertical", "scroll_vertical_enabled" }, // ScrollContainer
	{ "shadow/buffer_size", "shadow_buffer_size" }, // Light2D
	{ "shadow/color", "shadow_color" }, // Light2D
	{ "shadow/enabled", "shadow_enabled" }, // Light2D
	{ "shadow/item_mask", "shadow_item_cull_mask" }, // Light2D
	{ "size_flags/horizontal", "size_flags_horizontal" }, // Control // Enum order got inverted Expand,Fill -> Fill,Expand, handle manually after rename
	{ "size_flags/stretch_ratio", "size_flags_stretch_ratio" }, // Control
	{ "size_flags/vertical", "size_flags_vertical" }, // Control // Enum order got inverted Expand,Fill -> Fill,Expand, handle manually after rename
	{ "smoothing/enable", "smoothing_enabled" }, // Camera2D
	{ "smoothing/speed", "smoothing_speed" }, // Camera2D
	{ "sort/enabled", "sort_enabled" }, // YSort
	{ "split/collapsed", "collapsed" }, // SplitContainer
	{ "split/dragger_visibility", "dragger_visibility" }, // SplitContainer
	{ "split/offset", "split_offset" }, // SplitContainer
	{ "stream/audio_track", "audio_track" }, // VideoPlayer
	{ "stream/autoplay", "autoplay" }, // VideoPlayer
	{ "stream/buffering_ms", "buffering_msec" }, // VideoPlayer
	{ "stream/loop", "loop" }, // Audio*
	{ "stream/loop_restart_time", "loop_offset" }, // Audio*
	{ "stream/paused", "paused" }, // VideoPlayer
	{ "stream/pitch_scale", "pitch_scale" }, // Audio*
	{ "stream/play", "playing" }, // Audio*
	{ "stream/stream", "stream" }, // VideoPlayer
	{ "stream/volume_db", "volume_db" }, // VideoPlayer
	{ "suspension/max_force", "suspension_max_force" }, // VehicleWheel
	{ "suspension/stiffness", "suspension_stiffness" }, // VehicleWheel
	{ "suspension/travel", "suspension_travel" }, // VehicleWheel
	{ "texture/offset", "texture_offset" }, // Polygon2D
	{ "texture/over", "texture_over" }, // TextureProgress
	{ "texture/progress", "texture_progress" }, // TextureProgress
	{ "texture/rotation", "texture_rotation_degrees" }, // Polygon2D
	{ "texture/scale", "texture_scale" }, // Polygon2D
	{ "textures/click_mask", "texture_click_mask" }, // TextureButton
	{ "textures/disabled", "texture_disabled" }, // TextureButton
	{ "textures/focused", "texture_focused" }, // TextureButton
	{ "textures/hover", "texture_hover" }, // TextureButton
	{ "textures/normal", "texture_normal" }, // TextureButton
	{ "textures/pressed", "texture_pressed" }, // TextureButton
	{ "texture/texture", "texture" }, // Polygon2D
	{ "texture/under", "texture_under" }, // TextureProgress
	{ "theme/theme", "theme" }, // Control
	{ "transform/local", "transform" }, // Spatial
	{ "transform/pos", "position" }, // Node2D
	{ "transform/rotation", "rotation_degrees" }, // Spatial
	{ "transform/rotation_rad", "rotation" }, // Spatial
	{ "transform/rot", "rotation_degrees" }, // Node2D
	{ "transform/scale", "scale" }, // Node2D, Spatial
	{ "transform/translation", "translation" }, // Spatial
	{ "type/steering", "use_as_steering" }, // VehicleWheel
	{ "type/traction", "use_as_traction" }, // VehicleWheel
	{ "vars/lifetime", "lifetime" }, // Particles
	{ "velocity/angular", "angular_velocity" }, // PhysicsBody, PhysicsBody2D
	{ "velocity/linear", "linear_velocity" }, // PhysicsBody, PhysicsBody2D
	{ "visibility", "visibility_aabb" }, // Particles
	{ "visibility/behind_parent", "show_behind_parent" }, // CanvasItem
	{ "visibility/light_mask", "light_mask" }, // CanvasItem
	{ "visibility/on_top", "show_on_top" }, // CanvasItem
	//{ "visibility/opacity", "modulate" }, // CanvasItem // Can't be converted this way, handle manually
	//{ "visibility/self_opacity", "self_modulate" }, // CanvasItem // Can't be converted this way, handle manually
	{ "visibility/visible", "visible" }, // CanvasItem, Spatial
	{ "wheel/friction_slip", "wheel_friction_slip" }, // VehicleWheel
	{ "wheel/radius", "wheel_radius" }, // VehicleWheel
	{ "wheel/rest_length", "wheel_rest_length" }, // VehicleWheel
	{ "wheel/roll_influence", "wheel_roll_influence" }, // VehicleWheel
	{ "window/title", "window_title" }, // Dialogs
	{ "z/relative", "z_as_relative" }, // Node2D
	{ "z/z", "z_index" }, // Node2D
	{ NULL, NULL }
};

static const char *type_renames[][2] = {
	{ "CanvasItemMaterial", "ShaderMaterial" },
	{ "CanvasItemShader", "Shader" },
	{ "ColorFrame", "ColorRect" },
	{ "ColorRamp", "Gradient" },
	{ "FixedMaterial", "SpatialMaterial" },
	{ "Patch9Frame", "NinePatchRect" },
	{ "ReferenceFrame", "ReferenceRect" },
	{ "SampleLibrary", "Resource" },
	{ "SamplePlayer2D", "AudioStreamPlayer2D" },
	{ "SamplePlayer", "Node" },
	{ "SoundPlayer2D", "Node2D" },
	{ "SpatialSamplePlayer", "AudioStreamPlayer3D" },
	{ "SpatialStreamPlayer", "AudioStreamPlayer3D" },
	{ "StreamPlayer", "AudioStreamPlayer" },
	{ "TestCube", "MeshInstance" },
	{ "TextureFrame", "TextureRect" },
	// Only for scripts
	{ "Matrix32", "Transform2D" },
	{ "Matrix3", "Basis" },
	{ "RawArray", "PoolByteArray" },
	{ "IntArray", "PoolIntArray" },
	{ "RealArray", "PoolRealArray" },
	{ "StringArray", "PoolStringArray" },
	{ "Vector2Array", "PoolVector2Array" },
	{ "Vector3Array", "PoolVector3Array" },
	{ "ColorArray", "PoolColorArray" },
	{ "InputEventJoystickButton", "InputEventJoypadButton" },
	{ "InputEventJoystickMotion", "InputEventJoypadMotion" },
	{ NULL, NULL }
};

static const char *signal_renames[][2] = {
	{ "area_enter", "area_entered" }, // Area, Area2D
	{ "area_enter_shape", "area_shape_entered" }, // Area, Area2D
	{ "area_exit", "area_exited" }, // Area, Area2D
	{ "area_exit_shape", "area_shape_exited" }, // Area, Area2D
	{ "body_enter", "body_entered" }, // Area, Area2D, PhysicsBody, PhysicsBody2D
	{ "body_enter_shape", "body_shape_entered" }, // Area, Area2D, PhysicsBody, PhysicsBody2D
	{ "body_exit", "body_exited" }, // Area, Area2D, PhysicsBody, PhysicsBody2D
	{ "body_exit_shape", "body_shape_exited" }, // Area, Area2D, PhysicsBody, PhysicsBody2D
	{ "enter_camera", "camera_entered" }, // VisibilityNotifier
	{ "enter_screen", "screen_entered" }, // VisibilityNotifier, VisibilityNotifier2D
	{ "enter_tree", "tree_entered" }, // Node
	{ "enter_viewport", "viewport_entered" }, // VisibilityNotifier2D
	{ "exit_camera", "camera_exited" }, // VisibilityNotifier
	{ "exit_screen", "screen_exited" }, // VisibilityNotifier, VisibilityNotifier2D
	{ "exit_tree", "tree_exiting" }, // Node // 2.1 emits it before leaving the tree, like tree_exiting in 3.x
	{ "exit_viewport", "viewport_exited" }, // VisibilityNotifier2D
	//{ "finished", "animation_finished" }, // AnimationPlayer, AnimatedSprite, but not StreamPlayer, handle manually
	{ "fixed_frame", "physics_frame" }, // SceneTree
	{ "focus_enter", "focus_entered" }, // Control
	{ "focus_exit", "focus_exited" }, // Control
	//{ "input_event", "gui_input" }, // Control, but not CollisionObject and CollisionObject2D, handle manually
	{ "item_pressed", "id_pressed" }, // PopupMenu
	{ "modal_close", "modal_closed" }, // Control
	{ "mouse_enter", "mouse_entered" }, // CollisionObject, CollisionObject2D, Control
	{ "mouse_exit", "mouse_exited" }, // CollisionObject, CollisionObject2D, Control
	{ "tween_start", "tween_started" }, // Tween
	{ "tween_complete", "tween_completed" }, // Tween
	{ NULL, NULL }
};

void EditorExportGodot3::_find_files(EditorFileSystemDirectory *p_dir, List<String> *r_files) {

	for (int i = 0; i < p_dir->get_subdir_count(); i++) {
		_find_files(p_dir->get_subdir(i), r_files);
	}

	for (int i = 0; i < p_dir->get_file_count(); i++) {

		r_files->push_back(p_dir->get_file_path(i));
	}
}

void EditorExportGodot3::_rename_properties(const String &p_type, List<ExportData::PropertyData> *p_props) {

	// We need specific hacks to fix compatibility breakage in the tracks of Animations
	bool fix_animation_tracks = (p_type == "Animation");
	String found_track_number = "";

	// Anchors/margins changed in 3.0 from always-positive to relative to their ratio anchor,
	// so we need to flip the sign of margins based on their anchor mode.
	int flip_margin_left = false;
	int flip_margin_right = false;
	int flip_margin_top = false;
	int flip_margin_bottom = false;

	for (List<ExportData::PropertyData>::Element *E = p_props->front(); E; E = E->next()) {

		/* Fixes for 2D rotations */

		// 2D rotations are now clockwise to match the downward Y base
		// Do this before the renaming, as afterwards we can't distinguish
		// between 2D and 3D rotations_degrees
		if (E->get().name == "transform/rot" || E->get().name == "rect/rotation") {
			E->get().value = (real_t)E->get().value * -1.0;
		}

		// CanvasLayer's "rotation" was in degrees and is now in radians, same sign change as above
		if (p_type == "CanvasLayer" && E->get().name == "rotation") {
			E->get().name = "rotation_degrees";
			E->get().value = (real_t)E->get().value * -1.0;
		}

		// To fix 2D rotations in the properties of Animation tracks (see below),
		// we need to locate stuff like this:
		// tracks/0/path = NodePath("Sprite:transform/rot")
		// And then modify the 'values' key of 'tracks/0/keys'.
		// This is going to be hacky.
		// We'll assume that we get properties in the correct order, so that the path will come before the keys
		// Otherwise we'd have to keep a stack of the track keys we found to later compare them to track paths
		// that match rotation_deg...
		if (fix_animation_tracks) {
			String prop_name = E->get().name;
			if (prop_name.begins_with("tracks/") && prop_name.ends_with("/path")) {
				String path_value = E->get().value;

				// Check if it's a rotation and save the track number to fix its assigned values
				if (path_value.ends_with(":transform/rot") || path_value.ends_with(":rect/rotation")) {
					// We found a track 'path' with a "transform/rot" NodePath, its 'keys' need to be fixed
					found_track_number = prop_name.get_slice("/", 1);
					print_line("Found Animation track with 2D rotations: " + prop_name + " = " + path_value);
				}

				// In animation tracks, NodePaths can refer to properties that need to be renamed
				int sep = path_value.find(":");
				if (sep != -1) {
					String track_nodepath = path_value.substr(0, sep);
					String track_prop = path_value.substr(sep + 1, path_value.length());
					if (prop_rename_map.has(track_prop)) {
						track_prop = prop_rename_map[track_prop];
					}

					// "[self_]opacity" was removed, and is replaced by the alpha component of "[self_]modulate"
					// "modulate" may already exist, but we posit that the "opacity" value is more important
					// Thankfully in NodePaths we can access the alpha property directly
					if (track_prop == "visibility/opacity") {
						track_prop = "modulate:a";
					} else if (track_prop == "visibility/self_opacity") {
						track_prop = "self_modulate:a";
					}

					E->get().value = NodePath(track_nodepath + ":" + track_prop);
				}
			} else if (found_track_number != "" && prop_name == "tracks/" + found_track_number + "/keys") {
				// Bingo! We found keys matching the track number we had spotted
				print_line("Fixing sign of 2D rotations in animation track " + found_track_number);
				Dictionary track_keys = E->get().value;
				if (track_keys.has("values")) {
					Array values = track_keys["values"];
					for (int i = 0; i < values.size(); i++) {
						values[i] = (real_t)values[i] * -1.0;
					}
					track_keys["values"] = values;
					E->get().value = track_keys;
					found_track_number = "";
				} else {
					print_line("Tried to change rotation in Animation tracks, but no value set found.");
				}
			}
		}

		/* Type specific renames, done before the generic renaming */

		String pname = E->get().name;

		// The own "modulate" of these nodes only affected the node itself, like "self_modulate" in 3.x
		if ((p_type == "Sprite" || p_type == "AnimatedSprite" || p_type == "TextureFrame" || p_type == "Patch9Frame" || p_type == "ViewportSprite") && pname == "modulate") {
			E->get().name = "self_modulate";
			continue;
		}

		// The icon modulate of Button does not exist in 3.x, keeping it would modulate the whole button
		if (p_type == "Button" && (pname == "modulate" || pname == "icon_scale" || pname == "autowrap")) {
			print_line("WARNING: Button property '" + pname + "' has no equivalent in Godot 3, it was removed.");
			E->get().name = "";
			continue;
		}

		if (p_type == "TextureButton") {
			if (pname == "params/resize_mode") {
				E->get().name = "expand";
				E->get().value = ((int)E->get().value) == 1; // RESIZE_STRETCH
				continue;
			} else if (pname == "params/stretch_mode") {
				// "Scale On Expand (Compat)" was removed, the other values moved down by one
				E->get().name = "stretch_mode";
				E->get().value = MAX(0, ((int)E->get().value) - 1);
				continue;
			} else if (pname == "params/scale") {
				print_line("WARNING: TextureButton property 'params/scale' has no equivalent in Godot 3, it was removed.");
				E->get().name = "";
				continue;
			}
		}

		if ((p_type == "RayCast" || p_type == "RayCast2D") && pname == "layer_mask") {
			E->get().name = "collision_mask";
			continue;
		}
		if ((p_type == "RayCast" || p_type == "RayCast2D") && pname == "type_mask") {
			E->get().name = "";
			continue;
		}

		if (p_type == "Tween" && pname == "playback/active") {
			E->get().name = "";
			continue;
		}

		if (p_type == "StyleBoxFlat") {
			if (pname == "border_size") {
				int bs = E->get().value;
				E->get().name = "border_width_left";
				E->get().value = bs;
				const char *sides[3] = { "border_width_top", "border_width_right", "border_width_bottom" };
				for (int k = 0; k < 3; k++) {
					ExportData::PropertyData pdata;
					pdata.name = sides[k];
					pdata.value = bs;
					p_props->push_back(pdata);
				}
				continue;
			} else if (pname == "light_color") {
				E->get().name = "border_color";
				continue;
			} else if (pname == "dark_color") {
				// 3.x only has one border color, "light_color" is kept when both are set
				bool has_light = false;
				for (List<ExportData::PropertyData>::Element *F = p_props->front(); F; F = F->next()) {
					if (F->get().name == "light_color" || (F != E && F->get().name == "border_color")) {
						has_light = true;
					}
				}
				E->get().name = has_light ? String() : String("border_color");
				continue;
			} else if (pname == "draw_bg") {
				E->get().name = "draw_center";
				continue;
			}
		}

		if (p_type == "Viewport") {
			if (pname == "rect") {
				E->get().name = "size";
				E->get().value = Rect2(E->get().value).size;
				continue;
			} else if (pname == "render_target/v_flip") {
				E->get().name = "render_target_v_flip";
				continue;
			} else if (pname == "render_target/update_mode") {
				E->get().name = "render_target_update_mode";
				continue;
			} else if (pname == "render_target/clear_on_new_frame") {
				E->get().name = "render_target_clear_mode";
				E->get().value = bool(E->get().value) ? 0 : 1; // CLEAR_MODE_ALWAYS : CLEAR_MODE_NEVER
				continue;
			} else if (pname == "audio_listener/enable_2d") {
				E->get().name = "audio_listener_enable_2d";
				continue;
			} else if (pname == "audio_listener/enable_3d") {
				E->get().name = "audio_listener_enable_3d";
				continue;
			} else if (pname == "gui/disable_input") {
				E->get().name = "gui_disable_input";
				continue;
			} else if (pname == "physics/object_picking") {
				E->get().name = "physics_object_picking";
				continue;
			} else if (pname.begins_with("render_target/")) {
				E->get().name = "";
				continue;
			}
		}

		if (p_type == "Sample") {
			// Sample became AudioStreamSample, its data dictionary is now split in properties
			if (pname == "data") {
				Dictionary d = E->get().value;
				String fmt = d.has("format") ? String(d["format"]) : String("pcm16");
				E->get().value = d.has("data") ? d["data"] : Variant(DVector<uint8_t>());
				ExportData::PropertyData pdata;
				pdata.name = "stereo";
				pdata.value = d.has("stereo") ? bool(d["stereo"]) : false;
				p_props->push_back(pdata);
				pdata.name = "format";
				pdata.value = (fmt == "pcm8") ? 0 : ((fmt == "ima_adpcm") ? 2 : 1); // FORMAT_8_BITS, FORMAT_IMA_ADPCM, FORMAT_16_BITS
				p_props->push_back(pdata);
				continue;
			} else if (pname == "loop_format") {
				E->get().name = "loop_mode";
				continue;
			}
		}

		/* Do the actual renaming */

		if (prop_rename_map.has(E->get().name)) {
			E->get().name = prop_rename_map[E->get().name];
		}

		/* Hardcoded fixups for properties that changed definition in 3.0 */

		// Anchors changed from Begin,End,Ratio,Center to only a ratio
		if (E->get().name.begins_with("anchor_")) {
			String side = E->get().name.substr(7, E->get().name.length() - 1);
			int prop_value = (int)E->get().value;
			switch (prop_value) {
				case 0: { // Begin
					E->get().value = 0.0;
				} break;
				case 1: { // End
					E->get().value = 1.0;
					// Flip corresponding margin's sign
					if (side == "left")
						flip_margin_left = true;
					else if (side == "right")
						flip_margin_right = true;
					else if (side == "top")
						flip_margin_top = true;
					else if (side == "bottom")
						flip_margin_bottom = true;
				} break;
				case 2: { // Ratio
					E->get().value = 0.0;
					print_line("WARNING: Property '" + E->get().name + "' with value 'Ratio' cannot be converted to the format used in Godot 3. Convert it to 'Begin' or 'End' to avoid losing the corresponding margin value.");
				} break;
				case 3: { // Center
					E->get().value = 0.5;
					// Flip corresponding margin's sign
					if (side == "left")
						flip_margin_left = true;
					else if (side == "right")
						flip_margin_right = true;
					else if (side == "top")
						flip_margin_top = true;
					else if (side == "bottom")
						flip_margin_bottom = true;
				} break;
			}
		}

		// Size flags enum changed ordering from "Expand,Fill" to "Fill,Expand,..."
		// So we swap 1 (Expand) and 2 (Fill), keep 0 (none) and 3 (Expand + Fill)
		if (E->get().name == "size_flags_horizontal" || E->get().name == "size_flags_vertical") {
			int prop_value = (int)E->get().value;
			switch (prop_value) {
				case 1: // Expand -> Fill
					E->get().value = 2;
					break;
				case 2: // Fill -> Expand
					E->get().value = 1;
					break;
				default: // none or both, keep
					break;
			}
		}

		// "[self_]opacity" was removed, and is replaced by the alpha component of "[self_]modulate"
		// "modulate" may already exist, but we posit that the "opacity" value is more important
		if (E->get().name == "visibility/opacity" || E->get().name == "visibility/self_opacity") {
			if (E->get().name == "visibility/self_opacity") {
				E->get().name = "self_modulate";
			} else {
				E->get().name = "modulate";
			}
			E->get().value = Color(1.0, 1.0, 1.0, (float)E->get().value);
		}

		// AnimationPlayer's "playback/active" was renamed to "playback_active", but not AnimationTreePlayer's
		if (p_type == "AnimationPlayer" && E->get().name == "playback/active") {
			E->get().name = "playback_active";
		}

		// Joint2D's "collision/exclude_nodes" was renamed to "disable_collision", but not Joint's
		if (p_type == "Joint2D" && E->get().name == "collision/exclude_nodes") {
			E->get().name = "disable_collision";
		}

		// TextureProgress' "mode" was renamed to "fill_mode", but not that of other nodes like TileMap
		if (p_type == "TextureProgress" && E->get().name == "mode") {
			E->get().name = "fill_mode";
		}

		// Sprite and Sprite3D's "region" was renamed to "region_enabled", but not Texture's
		if ((p_type == "Sprite" || p_type == "Sprite3D") && E->get().name == "region") {
			E->get().name = "region_enabled";
		}

		// Replaced by "mouse_filter" in _add_new_properties
		if (E->get().name == "focus/ignore_mouse" || E->get().name == "focus/stop_mouse") {
			E->get().name = "";
		}

		// "visibility/blend_mode" moved to CanvasItemMaterial, see _convert_blend_mode
		if (E->get().name == "visibility/blend_mode") {
			E->get().name = "";
		}

		// "click_on_pressed" was renamed to "action_mode" and is now a enum
		if (E->get().name == "click_on_press") {
			E->get().name = "action_mode";
			if (E->get().value) {
				E->get().value = 0; // ACTION_MODE_BUTTON_PRESS
			} else {
				E->get().value = 1; // ACTION_MODE_BUTTON_RELEASE
			}
		}
	}

	// Remove the properties which have no equivalent in Godot 3
	for (List<ExportData::PropertyData>::Element *E = p_props->front(); E;) {
		List<ExportData::PropertyData>::Element *N = E->next();
		if (E->get().name == String()) {
			p_props->erase(E);
		}
		E = N;
	}

	// Flip margins based on the previously fixed anchor modes
	if (flip_margin_left || flip_margin_right || flip_margin_top || flip_margin_bottom) {
		// Loop again and fix the margins
		for (List<ExportData::PropertyData>::Element *E = p_props->front(); E; E = E->next()) {
			if (!E->get().name.begins_with("margin_")) {
				continue;
			}
			if ((flip_margin_left && E->get().name == "margin_left") ||
					(flip_margin_right && E->get().name == "margin_right") ||
					(flip_margin_top && E->get().name == "margin_top") ||
					(flip_margin_bottom && E->get().name == "margin_bottom")) {
				E->get().value = (real_t)E->get().value * -1.0;
			}
		}
	}
}

void EditorExportGodot3::_add_new_properties(const String &p_type, List<ExportData::PropertyData> *p_props) {
	bool add_mouse_filter = false;

	// Default values of Control in 2.1
	bool ignore_mouse = false;
	bool stop_mouse = true;

	for (List<ExportData::PropertyData>::Element *E = p_props->front(); E; E = E->next()) {
		String prop_name = E->get().name;
		if (prop_name == "focus/ignore_mouse" || prop_name == "focus/stop_mouse") {
			add_mouse_filter = true;

			if (prop_name == "focus/ignore_mouse") {
				ignore_mouse = E->get().value;
			} else if (prop_name == "focus/stop_mouse") {
				stop_mouse = E->get().value;
			}
		}
	}

	if (add_mouse_filter) {
		ExportData::PropertyData pdata;
		pdata.name = "mouse_filter";

		// In 2.1 "ignore_mouse" wins over "stop_mouse": the event never reached the Control
		if (ignore_mouse) {
			pdata.value = 2; // MOUSE_FILTER_IGNORE
		} else if (stop_mouse) {
			pdata.value = 0; // MOUSE_FILTER_STOP
		} else {
			pdata.value = 1; // MOUSE_FILTER_PASS
		}

		p_props->push_back(pdata);
	}
}

int EditorExportGodot3::_get_next_resource_index(const ExportData &resource) const {

	int idx = 1;
	for (int i = 0; i < resource.resources.size(); i++) {
		idx = MAX(idx, resource.resources[i].index + 1);
	}
	return idx;
}

String EditorExportGodot3::_add_sub_resource(ExportData &resource, const String &p_type, const List<ExportData::PropertyData> &p_props) {

	ExportData::ResourceData rd;
	rd.type = p_type;
	rd.index = _get_next_resource_index(resource);
	rd.properties = p_props;

	if (resource.nodes.size() == 0 && resource.resources.size() > 0) {
		// The main resource of a resource file has to stay the last one
		resource.resources.insert(resource.resources.size() - 1, rd);
	} else {
		resource.resources.push_back(rd);
	}

	return "@RESLOCAL:" + itos(rd.index);
}

static void _push_prop(List<ExportData::PropertyData> &r_props, const String &p_name, const Variant &p_value) {

	ExportData::PropertyData pdata;
	pdata.name = p_name;
	pdata.value = p_value;
	r_props.push_back(pdata);
}

static bool _has_prop(const List<ExportData::PropertyData> &p_props, const String &p_name, Variant *r_value = NULL) {

	for (const List<ExportData::PropertyData>::Element *E = p_props.front(); E; E = E->next()) {
		if (E->get().name == p_name) {
			if (r_value) {
				*r_value = E->get().value;
			}
			return true;
		}
	}
	return false;
}

static void _set_prop(List<ExportData::PropertyData> &r_props, const String &p_name, const Variant &p_value) {

	for (List<ExportData::PropertyData>::Element *E = r_props.front(); E; E = E->next()) {
		if (E->get().name == p_name) {
			E->get().value = p_value;
			return;
		}
	}
	_push_prop(r_props, p_name, p_value);
}

bool EditorExportGodot3::_image_to_resource(const Image &p_image, ExportData::ResourceData &r_resource) {

	// Image is a Resource in Godot 3, it's stored as a sub-resource with a "data" dictionary
	Image img = p_image;
	if (img.empty()) {
		return false;
	}

	if (img.is_compressed() && img.decompress() != OK) {
		print_line("WARNING: A compressed image could not be decompressed, it was not converted.");
		return false;
	}

	String format;
	switch (img.get_format()) {
		case Image::FORMAT_GRAYSCALE: format = "Lum8"; break;
		case Image::FORMAT_GRAYSCALE_ALPHA: format = "LumAlpha8"; break;
		case Image::FORMAT_RGB: format = "RGB8"; break;
		case Image::FORMAT_RGBA: format = "RGBA8"; break;
		default: {
			img.convert(Image::FORMAT_RGBA);
			format = "RGBA8";
		} break;
	}

	// Godot 3 expects a full mipmap chain, the texture flags regenerate them anyway
	if (img.get_mipmaps() > 0) {
		img.clear_mipmaps();
	}

	Dictionary d;
	d["width"] = img.get_width();
	d["height"] = img.get_height();
	d["format"] = format;
	d["mipmaps"] = false;
	d["data"] = img.get_data();

	r_resource.type = "Image";
	r_resource.properties.clear();
	_push_prop(r_resource.properties, "data", d);
	return true;
}

void EditorExportGodot3::_convert_images(ExportData &resource) {

	int next_index = _get_next_resource_index(resource);
	Vector<ExportData::ResourceData> converted;

	for (int i = 0; i < resource.resources.size(); i++) {

		ExportData::ResourceData rd = resource.resources[i];
		for (List<ExportData::PropertyData>::Element *E = rd.properties.front(); E; E = E->next()) {

			if (E->get().value.get_type() != Variant::IMAGE) {
				continue;
			}

			ExportData::ResourceData image_rd;
			if (_image_to_resource(E->get().value, image_rd)) {
				image_rd.index = next_index++;
				converted.push_back(image_rd);
				E->get().value = "@RESLOCAL:" + itos(image_rd.index);
			} else {
				E->get().value = Variant();
			}
		}
		converted.push_back(rd);
	}

	for (int i = 0; i < resource.nodes.size(); i++) {

		for (List<ExportData::PropertyData>::Element *E = resource.nodes[i].properties.front(); E; E = E->next()) {

			if (E->get().value.get_type() != Variant::IMAGE) {
				continue;
			}

			ExportData::ResourceData image_rd;
			if (_image_to_resource(E->get().value, image_rd)) {
				image_rd.index = next_index++;
				converted.push_back(image_rd);
				E->get().value = "@RESLOCAL:" + itos(image_rd.index);
			} else {
				E->get().value = Variant();
			}
		}
	}

	resource.resources = converted;
}

String EditorExportGodot3::_get_instanced_type(const ExportData &resource, int p_node) {

	// The type of an instanced node is the type of the root node of its scene
	String instance = resource.nodes[p_node].instance;
	if (!instance.begins_with("@RESEXTERNAL:")) {
		return String();
	}

	int dep = instance.get_slice(":", 1).to_int();
	if (!resource.dependencies.has(dep)) {
		return String();
	}

	String path = resource.dependencies[dep].path;
	if (instanced_types.has(path)) {
		return instanced_types[path];
	}

	String type;
	Ref<PackedScene> scene = ResourceLoader::load(path);
	if (scene.is_valid() && scene->get_state()->get_node_count() > 0) {
		type = scene->get_state()->get_node_type(0);
	}

	instanced_types[path] = type;
	return type;
}

String EditorExportGodot3::_get_node_type(const ExportData &resource, int p_node) {

	if (p_node < 0 || p_node >= resource.nodes.size()) {
		return String();
	}

	if (resource.nodes[p_node].type != String()) {
		return resource.nodes[p_node].type;
	}

	return _get_instanced_type(resource, p_node);
}

void EditorExportGodot3::_get_node_parents(const ExportData &resource, Vector<int> &r_parents) {

	r_parents.resize(resource.nodes.size());

	Map<String, int> paths;
	for (int i = 0; i < resource.nodes.size(); i++) {

		const ExportData::NodeData &node = resource.nodes[i];
		r_parents[i] = -1;

		if (node.text_data) {
			String parent = node.parent;
			String path;
			if (parent == String()) {
				path = ".";
			} else {
				if (paths.has(parent)) {
					r_parents[i] = paths[parent];
				}
				path = (parent == ".") ? String(node.name) : parent + "/" + node.name;
			}
			paths[path] = i;
		} else {
			int parent = node.parent_int;
			if (parent >= 0 && parent < resource.nodes.size() && !(parent & SceneState::FLAG_ID_IS_PATH)) {
				r_parents[i] = parent;
			}
		}
	}
}

int EditorExportGodot3::_get_connection_source(const ExportData &resource, const ExportData::Connection &p_connection) {

	if (!p_connection.text_data) {
		int from = p_connection.from_int;
		if (from & SceneState::FLAG_ID_IS_PATH) {
			return -1;
		}
		return from;
	}

	// NodePath "from" is relative to the root node
	String from = p_connection.from;

	for (int i = 0; i < resource.nodes.size(); i++) {

		String parent = resource.nodes[i].parent;
		String path;
		if (parent == String()) {
			path = ".";
		} else if (parent == ".") {
			path = resource.nodes[i].name;
		} else {
			path = parent + "/" + resource.nodes[i].name;
		}

		if (path == from) {
			return i;
		}
	}

	return -1;
}

void EditorExportGodot3::_convert_connections(ExportData &resource) {

	for (int i = 0; i < resource.connections.size(); i++) {

		String signal = resource.connections[i].signal;

		if (signal_rename_map.has(signal)) {
			resource.connections[i].signal = signal_rename_map[signal];
			continue;
		}

		/* Signals which need to be conditionally renamed based on their Node's type */

		if (signal != "finished" && signal != "input_event") {
			continue;
		}

		String type = _get_node_type(resource, _get_connection_source(resource, resource.connections[i]));
		if (type == String()) {
			print_line("WARNING: Unable to find the type of the node emitting '" + signal + "' (" + String(resource.connections[i].from) + "), the signal was not renamed.");
			continue;
		}

		// AnimationPlayer and AnimatedSprite's "finished" signal was renamed to "animation_finished",
		// but not that of StreamPlayer.
		if (signal == "finished" && (ObjectTypeDB::is_type(type, "AnimationPlayer") || ObjectTypeDB::is_type(type, "AnimatedSprite"))) {
			resource.connections[i].signal = "animation_finished";
		}

		// Control's "input_event" was renamed to "gui_input", but not that of CollisionObject[2D]
		if (signal == "input_event" && ObjectTypeDB::is_type(type, "Control")) {
			resource.connections[i].signal = "gui_input";
		}
	}
}

void EditorExportGodot3::_convert_particles_2d(ExportData &resource, int p_node) {

	// Particles2D became GPU based in Godot 3, CPUParticles2D is the node matching the 2.1 behavior
	List<ExportData::PropertyData> props;
	Map<String, Variant> old;

	for (List<ExportData::PropertyData>::Element *E = resource.nodes[p_node].properties.front(); E; E = E->next()) {
		String name = E->get().name;
		if (name.begins_with("config/") || name.begins_with("params/") || name.begins_with("randomness/") || name.begins_with("color/") || name.begins_with("color_phases/") || name.begins_with("phase_") || name == "emission_points") {
			old[name] = E->get().value;
		} else {
			props.push_back(E->get());
		}
	}

#define OLD_REAL(m_name, m_default) (old.has(m_name) ? (real_t)old[m_name] : (real_t)(m_default))

	_push_prop(props, "amount", old.has("config/amount") ? old["config/amount"] : Variant(32));
	_push_prop(props, "lifetime", OLD_REAL("config/lifetime", 2.0));
	_push_prop(props, "emitting", old.has("config/emitting") ? old["config/emitting"] : Variant(true));
	_push_prop(props, "local_coords", old.has("config/local_space") ? old["config/local_space"] : Variant(true));
	// 2.1 explosiveness: 1 means evenly spread, 0 means all at once. It's the opposite in 3.x
	_push_prop(props, "explosiveness", 1.0 - OLD_REAL("config/explosiveness", 1.0));

	if (old.has("config/time_scale")) {
		_push_prop(props, "speed_scale", old["config/time_scale"]);
	}
	if (old.has("config/preprocess")) {
		_push_prop(props, "preprocess", old["config/preprocess"]);
	}
	if (old.has("config/texture")) {
		_push_prop(props, "texture", old["config/texture"]);
	}

	// Directions were angles in degrees where 0 pointed down: Vector2(sin, cos)
	real_t direction = Math::deg2rad(OLD_REAL("params/direction", 0.0));
	_push_prop(props, "direction", Vector2(Math::stepify(Math::sin(direction), 0.0001), Math::stepify(Math::cos(direction), 0.0001)));
	_push_prop(props, "spread", OLD_REAL("params/spread", 10.0));

	real_t gravity_dir = Math::deg2rad(OLD_REAL("params/gravity_direction", 0.0));
	_push_prop(props, "gravity", Vector2(Math::stepify(Math::sin(gravity_dir), 0.0001), Math::stepify(Math::cos(gravity_dir), 0.0001)) * OLD_REAL("params/gravity_strength", 9.8));

	_push_prop(props, "initial_velocity", OLD_REAL("params/linear_velocity", 20.0));
	// Spin was in radians per second, 2D rotations are now clockwise
	_push_prop(props, "angular_velocity", -Math::rad2deg(OLD_REAL("params/spin_velocity", 0.0)));
	// Orbit was in radians per second, it's now in rotations per second
	_push_prop(props, "orbit_velocity", -OLD_REAL("params/orbit_velocity", 0.0) / (Math_PI * 2.0));
	_push_prop(props, "radial_accel", OLD_REAL("params/radial_accel", 0.0));
	// The tangent vector got inverted
	_push_prop(props, "tangential_accel", -OLD_REAL("params/tangential_accel", 0.0));
	_push_prop(props, "damping", OLD_REAL("params/damping", 0.0));
	_push_prop(props, "angle", -OLD_REAL("params/initial_angle", 0.0));
	_push_prop(props, "hue_variation", OLD_REAL("params/hue_variation", 0.0));
	_push_prop(props, "anim_speed", OLD_REAL("params/anim_speed_scale", 1.0));
	_push_prop(props, "anim_offset", OLD_REAL("params/anim_initial_pos", 0.0));

	real_t initial_size = OLD_REAL("params/initial_size", 1.0);
	real_t final_size = OLD_REAL("params/final_size", 1.0);
	_push_prop(props, "scale_amount", initial_size);
	if (initial_size != 0.0 && final_size != initial_size) {
		// The final size is now expressed with a curve multiplying "scale_amount"
		real_t ratio = final_size / initial_size;
		Array points;
		points.push_back(Vector2(0, 1));
		points.push_back(0.0);
		points.push_back(ratio - 1.0);
		points.push_back(0);
		points.push_back(1); // TANGENT_LINEAR
		points.push_back(Vector2(1, ratio));
		points.push_back(ratio - 1.0);
		points.push_back(0.0);
		points.push_back(1); // TANGENT_LINEAR
		points.push_back(0);

		List<ExportData::PropertyData> curve_props;
		_push_prop(curve_props, "min_value", MIN(0.0, ratio));
		_push_prop(curve_props, "max_value", MAX(1.0, ratio));
		_push_prop(curve_props, "_data", points);
		_push_prop(props, "scale_amount_curve", _add_sub_resource(resource, "Curve", curve_props));
	}

	static const char *randomness_renames[][2] = {
		{ "randomness/linear_velocity", "initial_velocity_random" },
		{ "randomness/spin_velocity", "angular_velocity_random" },
		{ "randomness/orbit_velocity", "orbit_velocity_random" },
		{ "randomness/radial_accel", "radial_accel_random" },
		{ "randomness/tangential_accel", "tangential_accel_random" },
		{ "randomness/damping", "damping_random" },
		{ "randomness/initial_angle", "angle_random" },
		{ "randomness/initial_size", "scale_amount_random" },
		{ "randomness/hue_variation", "hue_variation_random" },
		{ "randomness/anim_speed_scale", "anim_speed_random" },
		{ "randomness/anim_initial_pos", "anim_offset_random" },
		{ NULL, NULL }
	};

	for (int i = 0; randomness_renames[i][0]; i++) {
		if (OLD_REAL(randomness_renames[i][0], 0.0) != 0.0) {
			_push_prop(props, randomness_renames[i][1], CLAMP(Math::abs(OLD_REAL(randomness_renames[i][0], 0.0)), 0.0, 1.0));
		}
	}

	if (old.has("color/color")) {
		_push_prop(props, "color", old["color/color"]);
	}
	if (old.has("color/color_ramp")) {
		_push_prop(props, "color_ramp", old["color/color_ramp"]);
	}

	Vector2 extents = old.has("config/half_extents") ? Vector2(old["config/half_extents"]) : Vector2();
	Vector2 offset = old.has("config/offset") ? Vector2(old["config/offset"]) : Vector2();
	DVector<Vector2> points = old.has("emission_points") ? DVector<Vector2>(old["emission_points"]) : DVector<Vector2>();

	if (points.size()) {
		// Emission points were normalized and multiplied by the extents
		DVector<Vector2> new_points;
		for (int i = 0; i < points.size(); i++) {
			new_points.push_back(offset + points.get(i) * extents);
		}
		_push_prop(props, "emission_shape", 3); // EMISSION_SHAPE_POINTS
		_push_prop(props, "emission_points", new_points);
	} else if (extents != Vector2()) {
		_push_prop(props, "emission_shape", 2); // EMISSION_SHAPE_RECTANGLE
		_push_prop(props, "emission_rect_extents", extents);
	}

	if (offset != Vector2() && !points.size()) {
		print_line("WARNING: Particles2D '" + resource.nodes[p_node].name + "': the emission offset has no equivalent in CPUParticles2D, move the node instead.");
	}

	int h_frames = old.has("config/h_frames") ? int(old["config/h_frames"]) : 1;
	int v_frames = old.has("config/v_frames") ? int(old["config/v_frames"]) : 1;
	if (h_frames * v_frames > 1) {
		if (_has_prop(props, "material/material")) {
			print_line("WARNING: Particles2D '" + resource.nodes[p_node].name + "' already has a material, set the particles animation frames in it manually.");
		} else {
			List<ExportData::PropertyData> mat_props;
			_push_prop(mat_props, "particles_animation", true);
			_push_prop(mat_props, "particles_anim_h_frames", h_frames);
			_push_prop(mat_props, "particles_anim_v_frames", v_frames);
			_push_prop(mat_props, "particles_anim_loop", true);
			_push_prop(props, "material", _add_sub_resource(resource, "CanvasItemMaterial", mat_props));
		}
	}

	if (old.has("config/flip_h") || old.has("config/flip_v") || old.has("config/emit_timeout") || OLD_REAL("randomness/direction", 0.0) != 0.0 || OLD_REAL("randomness/gravity_direction", 0.0) != 0.0) {
		print_line("WARNING: Particles2D '" + resource.nodes[p_node].name + "': flip, emit timeout and direction randomness have no equivalent in CPUParticles2D.");
	}

#undef OLD_REAL

	resource.nodes[p_node].type = "CPUParticles2D";
	resource.nodes[p_node].properties = props;
}

void EditorExportGodot3::_convert_blend_mode(ExportData &resource, int p_node) {

	// "visibility/blend_mode" moved from CanvasItem to CanvasItemMaterial, with the same values
	Variant blend_mode;
	if (!_has_prop(resource.nodes[p_node].properties, "visibility/blend_mode", &blend_mode) || int(blend_mode) == 0) {
		return;
	}

	Variant material;
	if (_has_prop(resource.nodes[p_node].properties, "material", &material)) {
		// Material created during this conversion (Particles2D)
		String ref = material;
		int index = ref.get_slice(":", 1).to_int();
		for (int i = 0; i < resource.resources.size(); i++) {
			if (resource.resources[i].index == index) {
				_set_prop(resource.resources[i].properties, "blend_mode", blend_mode);
			}
		}
	} else if (_has_prop(resource.nodes[p_node].properties, "material/material")) {
		print_line("WARNING: Node '" + resource.nodes[p_node].name + "' has a material and a blend mode, the blend mode must be set manually in its material.");
	} else {
		List<ExportData::PropertyData> mat_props;
		_push_prop(mat_props, "blend_mode", blend_mode);
		_push_prop(resource.nodes[p_node].properties, "material", _add_sub_resource(resource, "CanvasItemMaterial", mat_props));
	}
}

void EditorExportGodot3::_convert_button_groups(ExportData &resource) {

	// ButtonGroup was a BoxContainer node in 2.1, it's a Resource assigned to the buttons in 3.x
	Vector<int> parents;
	_get_node_parents(resource, parents);

	Map<int, String> groups;
	for (int i = 0; i < resource.nodes.size(); i++) {
		if (resource.nodes[i].type == "ButtonGroup") {
			groups[i] = _add_sub_resource(resource, "ButtonGroup", List<ExportData::PropertyData>());
			resource.nodes[i].type = "VBoxContainer";
		}
	}

	if (groups.empty()) {
		return;
	}

	for (int i = 0; i < resource.nodes.size(); i++) {

		String type = _get_node_type(resource, i);
		if (groups.has(i) || type == String() || !ObjectTypeDB::is_type(type, "BaseButton")) {
			continue;
		}

		// The closest ButtonGroup ancestor wins
		int parent = parents[i];
		while (parent != -1 && !groups.has(parent)) {
			parent = parents[parent];
		}
		if (parent == -1) {
			continue;
		}

		_set_prop(resource.nodes[i].properties, "group", groups[parent]);
		// The group only works with toggle buttons in 3.x
		_set_prop(resource.nodes[i].properties, "toggle_mode", true);
	}
}

void EditorExportGodot3::_convert_dependencies(ExportData &resource) {

	for (Map<int, ExportData::Dependency>::Element *E = resource.dependencies.front(); E; E = E->next()) {

		String type = E->get().type;
		String path = _replace_resource(E->get().path);
		String ext = path.extension().to_lower();

		if (type_rename_map.has(type)) {
			type = type_rename_map[type];
		}

		// Images are imported as StreamTexture, which is not an ImageTexture
		if (type == "ImageTexture" && (ext == "png" || ext == "jpg" || ext == "jpeg" || ext == "webp" || ext == "bmp" || ext == "tga" || ext == "svg" || ext == "hdr" || ext == "exr")) {
			type = "Texture";
		}

		if (type == "Sample") {
			type = "AudioStreamSample";
		}

		E->get().type = type;
	}
}

void EditorExportGodot3::_convert_resources(ExportData &resource) {

	_convert_images(resource);

	for (int i = 0; i < resource.resources.size(); i++) {

		if (resource.resources[i].type == "GDScript" && export_convert_scripts) {
			// Built-in script
			for (List<ExportData::PropertyData>::Element *E = resource.resources[i].properties.front(); E; E = E->next()) {
				if (E->get().name != "script/source") {
					continue;
				}
				Vector<String> lines = String(E->get().value).split("\n");
				Vector<String> converted = _convert_script_lines(current_path + "::" + itos(resource.resources[i].index), lines, export_mark_lines);
				String source;
				for (int j = 0; j < converted.size(); j++) {
					source += (j ? "\n" : "") + converted[j];
				}
				E->get().value = source;
			}
		}

		_add_new_properties(resource.resources[i].type, &resource.resources[i].properties);
		_rename_properties(resource.resources[i].type, &resource.resources[i].properties);

		if (resource.resources[i].type == "Sample") {
			resource.resources[i].type = "AudioStreamSample";
		} else if (resource.resources[i].type == "Font") {
			// Font was the bitmap font of 2.1, Font is an abstract class in Godot 3
			resource.resources[i].type = "BitmapFont";
		} else if (type_rename_map.has(resource.resources[i].type)) {
			resource.resources[i].type = type_rename_map[resource.resources[i].type];
		}
	}

	// Done before renaming the node types, as they are needed to rename signals
	_convert_connections(resource);
	_convert_button_groups(resource);

	for (int i = 0; i < resource.nodes.size(); i++) {

		// Exported script members which were renamed
		Variant script;
		if (_has_prop(resource.nodes[i].properties, "script/script", &script) && String(script).begins_with("@RESEXTERNAL:")) {
			int dep = String(script).get_slice(":", 1).to_int();
			if (resource.dependencies.has(dep) && script_member_renames.has(resource.dependencies[dep].path)) {
				const Map<String, String> &renames = script_member_renames[resource.dependencies[dep].path];
				for (List<ExportData::PropertyData>::Element *E = resource.nodes[i].properties.front(); E; E = E->next()) {
					if (renames.has(E->get().name)) {
						E->get().name = renames[E->get().name];
					}
				}
			}
		}

		if (resource.nodes[i].type == "Particles2D") {
			_convert_particles_2d(resource, i);
		}
		_convert_blend_mode(resource, i);

		_add_new_properties(resource.nodes[i].type, &resource.nodes[i].properties);
		_rename_properties(resource.nodes[i].type, &resource.nodes[i].properties);

		if (type_rename_map.has(resource.nodes[i].type)) {
			resource.nodes[i].type = type_rename_map[resource.nodes[i].type];
		}
	}

	_convert_dependencies(resource);
}

void EditorExportGodot3::_unpack_packed_scene(ExportData &resource) {

	Dictionary d;
	for (List<ExportData::PropertyData>::Element *E = resource.resources[resource.resources.size() - 1].properties.front(); E; E = E->next()) {
		if (E->get().name == "_bundled") {
			d = E->get().value;
		}
	}

	ERR_FAIL_COND(d.empty());

	ERR_FAIL_COND(!d.has("names"));
	ERR_FAIL_COND(!d.has("variants"));
	ERR_FAIL_COND(!d.has("node_count"));
	ERR_FAIL_COND(!d.has("nodes"));
	ERR_FAIL_COND(!d.has("conn_count"));
	ERR_FAIL_COND(!d.has("conns"));

	Vector<String> names;

	DVector<String> snames = d["names"];
	if (snames.size()) {

		int namecount = snames.size();
		names.resize(namecount);
		DVector<String>::Read r = snames.read();
		for (int i = 0; i < names.size(); i++)
			names[i] = r[i];
	}

	Array variants = d["variants"];

	resource.nodes.resize(d["node_count"]);

	int nc = resource.nodes.size();
	if (nc) {
		DVector<int> snodes = d["nodes"];
		DVector<int>::Read r = snodes.read();
		int idx = 0;
		for (int i = 0; i < nc; i++) {

			int parent = r[idx++];
			int owner = r[idx++];
			int type = r[idx++];
			int name = r[idx++];
			int instance = r[idx++];

			ExportData::NodeData &node_data = resource.nodes[i];

			node_data.text_data = false;
			node_data.name = names[name];
			if (type == 0x7FFFFFFF) {
				node_data.instanced = true;
			} else {
				node_data.instanced = false;
				node_data.type = names[type];
			}

			node_data.parent_int = parent;
			node_data.owner_int = owner;
			if (instance >= 0) {
				node_data.instance_is_placeholder = instance & SceneState::FLAG_INSTANCE_IS_PLACEHOLDER;
				node_data.instance = variants[instance & SceneState::FLAG_MASK];
			}

			int prop_count = r[idx++];

			for (int j = 0; j < prop_count; j++) {

				int prop_name = r[idx++];
				int prop_value = r[idx++];

				ExportData::PropertyData pdata;
				pdata.name = names[prop_name];
				pdata.value = variants[prop_value];
				node_data.properties.push_back(pdata);
			}

			int group_count = r[idx++];
			for (int j = 0; j < group_count; j++) {

				int group_name = r[idx++];
				node_data.groups.push_back(names[group_name]);
			}
		}
	}

	int cc = d["conn_count"];

	if (cc) {

		DVector<int> sconns = d["conns"];
		DVector<int>::Read r = sconns.read();
		int idx = 0;
		for (int i = 0; i < cc; i++) {

			ExportData::Connection conn;

			conn.text_data = false;
			conn.from_int = r[idx++];
			conn.to_int = r[idx++];
			conn.signal = names[r[idx++]];
			conn.method = names[r[idx++]];
			conn.flags = r[idx++];
			int bindcount = r[idx++];

			for (int j = 0; j < bindcount; j++) {

				conn.binds.push_back(variants[r[idx++]]);
			}

			resource.connections.push_back(conn);
		}
	}

	Array np;
	if (d.has("node_paths")) {
		np = d["node_paths"];
	}

	for (int i = 0; i < np.size(); i++) {
		resource.node_paths.push_back(np[i]);
	}

	Array ei;
	if (d.has("editable_instances")) {
		ei = d["editable_instances"];
		for (int i = 0; i < ei.size(); i++) {
			resource.editables.push_back(ei[i]);
		}
	}

	if (d.has("base_scene")) {
		resource.base_scene = variants[d["base_scene"]];
	}

	resource.resources.resize(resource.resources.size() - 1); // erase packed
}

void EditorExportGodot3::_packed_to_text(ExportData &resource) {

	// Text scenes (XML in 2.1) are saved with the node format of .tscn, instead of a packed dictionary.
	// Nodes and connections refer to nodes with paths.
	Vector<String> paths;
	paths.resize(resource.nodes.size());

	for (int i = 0; i < resource.nodes.size(); i++) {

		ExportData::NodeData &node = resource.nodes[i];
		int parent = node.parent_int;
		String parent_path;

		if (parent < 0 || parent == 0x7FFFFFFF) {
			paths[i] = ".";
		} else {
			if (parent & SceneState::FLAG_ID_IS_PATH) {
				parent_path = String(resource.node_paths[parent & SceneState::FLAG_MASK].operator NodePath());
			} else if (parent < i) {
				parent_path = paths[parent];
			}
			paths[i] = (parent_path == ".") ? node.name : parent_path + "/" + node.name;
		}

		node.text_data = true;
		node.parent = parent_path == String() ? NodePath() : NodePath(parent_path);
		node.owner = NodePath();

		if (i == 0 && resource.base_scene.get_type() != Variant::NIL) {
			// Inherited scene
			node.instance = resource.base_scene;
		}
	}

	for (int i = 0; i < resource.connections.size(); i++) {

		ExportData::Connection &conn = resource.connections[i];
		int ids[2] = { conn.from_int, conn.to_int };
		NodePath node_paths[2];

		for (int j = 0; j < 2; j++) {
			if (ids[j] & SceneState::FLAG_ID_IS_PATH) {
				node_paths[j] = resource.node_paths[ids[j] & SceneState::FLAG_MASK].operator NodePath();
			} else if (ids[j] >= 0 && ids[j] < paths.size()) {
				node_paths[j] = NodePath(paths[ids[j]]);
			}
		}

		conn.from = node_paths[0];
		conn.to = node_paths[1];
		conn.text_data = true;
	}
}

void EditorExportGodot3::_pack_packed_scene(ExportData &resource) {

	pack_names.clear();
	pack_values.clear();

	Dictionary d;

	d["node_count"] = resource.nodes.size();

	Vector<int> node_data;

	for (int i = 0; i < resource.nodes.size(); i++) {

		const ExportData::NodeData &node = resource.nodes[i];

		node_data.push_back(node.parent_int);
		node_data.push_back(node.owner_int);
		if (node.instanced) {
			node_data.push_back(0x7FFFFFFF);
		} else {
			int name = _pack_name(node.type);
			node_data.push_back(name);
		}

		node_data.push_back(_pack_name(node.name));
		int instance = -1;
		if (node.instance != String()) {
			instance = _pack_value(node.instance);
			if (node.instance_is_placeholder) {
				instance |= SceneState::FLAG_INSTANCE_IS_PLACEHOLDER;
			}
		}
		node_data.push_back(instance);

		node_data.push_back(node.properties.size());

		for (int j = 0; j < node.properties.size(); j++) {
			node_data.push_back(_pack_name(node.properties[j].name));
			node_data.push_back(_pack_value(node.properties[j].value));
		}

		node_data.push_back(node.groups.size());

		for (int j = 0; j < node.groups.size(); j++) {

			node_data.push_back(_pack_name(node.groups[j]));
		}
	}

	d["nodes"] = node_data;

	d["conn_count"] = resource.connections.size();

	Vector<int> connections;

	for (int i = 0; i < resource.connections.size(); i++) {
		const ExportData::Connection &conn = resource.connections[i];

		connections.push_back(conn.from_int);
		connections.push_back(conn.to_int);
		connections.push_back(_pack_name(conn.signal));
		connections.push_back(_pack_name(conn.method));
		connections.push_back(conn.flags);
		connections.push_back(conn.binds.size());
		for (int j = 0; j < conn.binds.size(); j++) {
			connections.push_back(_pack_value(conn.binds[j]));
		}
	}

	d["conns"] = connections;

	Array np;
	for (int i = 0; i < resource.node_paths.size(); i++) {
		np.push_back(resource.node_paths[i]);
	}

	d["node_paths"] = np;

	Array ei;
	for (int i = 0; i < resource.editables.size(); i++) {
		ei.push_back(resource.editables[i]);
	}

	d["editable_instances"] = ei;

	if (resource.base_scene.get_type()) {

		d["base_scene"] = _pack_value(resource.base_scene);
	}

	DVector<String> names;
	names.resize(pack_names.size());
	{
		DVector<String>::Write w = names.write();
		for (Map<String, int>::Element *E = pack_names.front(); E; E = E->next()) {
			w[E->get()] = E->key();
		}
	}

	d["names"] = names;

	Array values;
	values.resize(pack_values.size());

	const Variant *K = NULL;
	while ((K = pack_values.next(K))) {

		int index = pack_values[*K];
		values[index] = *K;
	}

	d["variants"] = values;

	ExportData::ResourceData packed_scene;
	packed_scene.type = "PackedScene";
	packed_scene.index = -1;
	ExportData::PropertyData pd;
	pd.name = "_bundled";
	pd.value = d;
	packed_scene.properties.push_back(pd);

	resource.resources.push_back(packed_scene);
	resource.nodes.clear();
	resource.connections.clear();
	resource.editables.clear();
	resource.node_paths.clear();
	;
	resource.base_scene = Variant();
}

static String rtosfix(double p_value) {

	if (p_value == 0.0)
		return "0"; // avoid negative zero (-0) being written, which may annoy git, svn, etc. for changes when they don't exist.
	else
		return rtoss(p_value);
}

Error EditorExportGodot3::_get_property_as_text(const Variant &p_variant, String &p_string) {

	switch (p_variant.get_type()) {

		case Variant::NIL: {
			p_string += ("null");
		} break;
		case Variant::BOOL: {

			p_string += (p_variant.operator bool() ? "true" : "false");
		} break;
		case Variant::INT: {

			p_string += (itos(p_variant.operator int64_t()));
		} break;
		case Variant::REAL: {

			String s = rtosfix(p_variant.operator real_t());
			if (s.find(".") == -1 && s.find("e") == -1)
				s += ".0";
			p_string += (s);
		} break;
		case Variant::STRING: {

			String str = p_variant;
			if (str.begins_with("@RESLOCAL:")) {
				p_string += "SubResource( " + str.get_slice(":", 1) + " )";
			} else if (str.begins_with("@RESEXTERNAL:")) {
				p_string += "ExtResource( " + str.get_slice(":", 1) + " )";
			} else {

				// Call _replace_resource in case it's a path to a scene/resource
				str = "\"" + _replace_resource(str).c_escape_multiline() + "\"";
				p_string += (str);
			}

		} break;
		case Variant::VECTOR2: {

			Vector2 v = p_variant;
			p_string += ("Vector2( " + rtosfix(v.x) + ", " + rtosfix(v.y) + " )");
		} break;
		case Variant::RECT2: {

			Rect2 aabb = p_variant;
			p_string += ("Rect2( " + rtosfix(aabb.pos.x) + ", " + rtosfix(aabb.pos.y) + ", " + rtosfix(aabb.size.x) + ", " + rtosfix(aabb.size.y) + " )");

		} break;
		case Variant::VECTOR3: {

			Vector3 v = p_variant;
			p_string += ("Vector3( " + rtosfix(v.x) + ", " + rtosfix(v.y) + ", " + rtosfix(v.z) + " )");
		} break;
		case Variant::PLANE: {

			Plane p = p_variant;
			p_string += ("Plane( " + rtosfix(p.normal.x) + ", " + rtosfix(p.normal.y) + ", " + rtosfix(p.normal.z) + ", " + rtosfix(p.d) + " )");

		} break;
		case Variant::_AABB: {

			Rect3 aabb = p_variant;
			p_string += ("Rect3( " + rtosfix(aabb.pos.x) + ", " + rtosfix(aabb.pos.y) + ", " + rtosfix(aabb.pos.z) + ", " + rtosfix(aabb.size.x) + ", " + rtosfix(aabb.size.y) + ", " + rtosfix(aabb.size.z) + " )");

		} break;
		case Variant::QUAT: {

			Quat quat = p_variant;
			p_string += ("Quat( " + rtosfix(quat.x) + ", " + rtosfix(quat.y) + ", " + rtosfix(quat.z) + ", " + rtosfix(quat.w) + " )");

		} break;
		case Variant::MATRIX32: {

			String s = "Transform2D( ";
			Matrix32 m3 = p_variant;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 2; j++) {

					if (i != 0 || j != 0)
						s += ", ";
					s += rtosfix(m3.elements[i][j]);
				}
			}

			p_string += (s + " )");

		} break;
		case Variant::MATRIX3: {

			String s = "Basis( ";
			Matrix3 m3 = p_variant;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {

					if (i != 0 || j != 0)
						s += ", ";
					s += rtosfix(m3.elements[i][j]);
				}
			}

			p_string += (s + " )");

		} break;
		case Variant::TRANSFORM: {

			String s = "Transform( ";
			Transform t = p_variant;
			Matrix3 &m3 = t.basis;
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {

					if (i != 0 || j != 0)
						s += ", ";
					s += rtosfix(m3.elements[i][j]);
				}
			}

			s = s + ", " + rtosfix(t.origin.x) + ", " + rtosfix(t.origin.y) + ", " + rtosfix(t.origin.z);

			p_string += (s + " )");
		} break;

		// misc types
		case Variant::COLOR: {

			Color c = p_variant;
			p_string += ("Color( " + rtosfix(c.r) + ", " + rtosfix(c.g) + ", " + rtosfix(c.b) + ", " + rtosfix(c.a) + " )");

		} break;
		case Variant::IMAGE: {

			Image img = p_variant;

			if (img.empty()) {
				p_string += ("Image()");
				break;
			}

			String imgstr = "Image()";
			p_string += imgstr; // do not convert this for now

			/*imgstr+=itos(img.get_width());
			imgstr+=", "+itos(img.get_height());
			imgstr+=", "+String(img.get_mipmaps()?"true":"false");
			imgstr+=", "+Image::get_format_name(img.get_format());

			String s;

			DVector<uint8_t> data = img.get_data();
			int len = data.size();
			DVector<uint8_t>::Read r = data.read();
			const uint8_t *ptr=r.ptr();
			for (int i=0;i<len;i++) {

				if (i>0)
					s+=", ";
				s+=itos(ptr[i]);
			}

			imgstr+=", ";
			p_string+=(imgstr);
			p_string+=(s);
			p_string+=(" )");*/
		} break;
		case Variant::NODE_PATH: {

			String str = p_variant;

			str = "NodePath(\"" + str.c_escape() + "\")";
			p_string += (str);

		} break;

		case Variant::OBJECT: {

			// should never arrive here!
			ERR_FAIL_V(ERR_BUG);
		} break;
		case Variant::INPUT_EVENT: {

			String str = "InputEvent(";

			InputEvent ev = p_variant;
			switch (ev.type) {
				case InputEvent::KEY: {

					str += "KEY," + itos(ev.key.scancode);
					String mod;
					if (ev.key.mod.alt)
						mod += "A";
					if (ev.key.mod.shift)
						mod += "S";
					if (ev.key.mod.control)
						mod += "C";
					if (ev.key.mod.meta)
						mod += "M";

					if (mod != String())
						str += "," + mod;
				} break;
				case InputEvent::MOUSE_BUTTON: {

					str += "MBUTTON," + itos(ev.mouse_button.button_index);
				} break;
				case InputEvent::JOYSTICK_BUTTON: {
					str += "JBUTTON," + itos(ev.joy_button.button_index);

				} break;
				case InputEvent::JOYSTICK_MOTION: {
					str += "JAXIS," + itos(ev.joy_motion.axis) + "," + itos(ev.joy_motion.axis_value);
				} break;
				case InputEvent::NONE: {
					str += "NONE";
				} break;
				default: {
				}
			}

			str += ")";

			p_string += (str); // will be added later

		} break;
		case Variant::DICTIONARY: {

			Dictionary dict = p_variant;

			List<Variant> keys;
			dict.get_key_list(&keys);
			keys.sort();

			p_string += ("{\n");
			for (List<Variant>::Element *E = keys.front(); E; E = E->next()) {

				/*
				if (!_check_type(dict[E->get()]))
					continue;
				*/
				_get_property_as_text(E->get(), p_string);
				p_string += (": ");
				_get_property_as_text(dict[E->get()], p_string);
				if (E->next())
					p_string += (",\n");
			}

			p_string += ("\n}");

		} break;
		case Variant::ARRAY: {

			p_string += ("[ ");
			Array array = p_variant;
			int len = array.size();
			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");
				_get_property_as_text(array[i], p_string);
			}
			p_string += (" ]");

		} break;

		case Variant::RAW_ARRAY: {

			p_string += ("PoolByteArray( ");
			String s;
			DVector<uint8_t> data = p_variant;
			int len = data.size();
			DVector<uint8_t>::Read r = data.read();
			const uint8_t *ptr = r.ptr();
			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");

				p_string += (itos(ptr[i]));
			}

			p_string += (" )");

		} break;
		case Variant::INT_ARRAY: {

			p_string += ("PoolIntArray( ");
			DVector<int> data = p_variant;
			int len = data.size();
			DVector<int>::Read r = data.read();
			const int *ptr = r.ptr();

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");

				p_string += (itos(ptr[i]));
			}

			p_string += (" )");

		} break;
		case Variant::REAL_ARRAY: {

			p_string += ("PoolRealArray( ");
			DVector<real_t> data = p_variant;
			int len = data.size();
			DVector<real_t>::Read r = data.read();
			const real_t *ptr = r.ptr();

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");
				p_string += (rtosfix(ptr[i]));
			}

			p_string += (" )");

		} break;
		case Variant::STRING_ARRAY: {

			p_string += ("PoolStringArray( ");
			DVector<String> data = p_variant;
			int len = data.size();
			DVector<String>::Read r = data.read();
			const String *ptr = r.ptr();
			String s;
			// write_string("\n");

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");
				String str = ptr[i];
				p_string += ("\"" + str.c_escape() + "\"");
			}

			p_string += (" )");

		} break;
		case Variant::VECTOR2_ARRAY: {

			p_string += ("PoolVector2Array( ");
			DVector<Vector2> data = p_variant;
			int len = data.size();
			DVector<Vector2>::Read r = data.read();
			const Vector2 *ptr = r.ptr();

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");
				p_string += (rtosfix(ptr[i].x) + ", " + rtosfix(ptr[i].y));
			}

			p_string += (" )");

		} break;
		case Variant::VECTOR3_ARRAY: {

			p_string += ("PoolVector3Array( ");
			DVector<Vector3> data = p_variant;
			int len = data.size();
			DVector<Vector3>::Read r = data.read();
			const Vector3 *ptr = r.ptr();

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");
				p_string += (rtosfix(ptr[i].x) + ", " + rtosfix(ptr[i].y) + ", " + rtosfix(ptr[i].z));
			}

			p_string += (" )");

		} break;
		case Variant::COLOR_ARRAY: {

			p_string += ("PoolColorArray( ");

			DVector<Color> data = p_variant;
			int len = data.size();
			DVector<Color>::Read r = data.read();
			const Color *ptr = r.ptr();

			for (int i = 0; i < len; i++) {

				if (i > 0)
					p_string += (", ");

				p_string += (rtosfix(ptr[i].r) + ", " + rtosfix(ptr[i].g) + ", " + rtosfix(ptr[i].b) + ", " + rtosfix(ptr[i].a));
			}
			p_string += (" )");

		} break;
		default: {
		}
	}

	return OK;
}

static String _valprop(const String &p_name) {

	// Escape and quote strings with extended ASCII or further Unicode characters
	// as well as '"', '=' or ' ' (32)
	const CharType *cstr = p_name.c_str();
	for (int i = 0; cstr[i]; i++) {
		if (cstr[i] == '=' || cstr[i] == '"' || cstr[i] < 33 || cstr[i] > 126) {
			return "\"" + p_name.c_escape_multiline() + "\"";
		}
	}
	// Keep as is
	return p_name;
}

void EditorExportGodot3::_save_text(const String &p_path, ExportData &resource) {

	FileAccessRef f = FileAccess::open(p_path, FileAccess::WRITE);

	if (resource.nodes.size()) {
		f->store_line("[gd_scene load_steps=" + itos(resource.nodes.size() + resource.resources.size()) + " format=2]\n");
	} else {
		f->store_line("[gd_resource type=\"" + resource.resources[resource.resources.size() - 1].type + "\" load_steps=" + itos(resource.resources.size()) + " format=2]\n");
	}

	for (Map<int, ExportData::Dependency>::Element *E = resource.dependencies.front(); E; E = E->next()) {

		f->store_line("[ext_resource path=\"" + resource_replace_map[E->get().path] + "\" type=\"" + E->get().type + "\" id=" + itos(E->key()) + "]");
	}

	for (int i = 0; i < resource.resources.size(); i++) {

		if (resource.nodes.size() || i < resource.resources.size() - 1) {

			f->store_line("\n[sub_resource type=\"" + resource.resources[i].type + "\" id=" + itos(resource.resources[i].index) + "]\n");
		} else {
			f->store_line("\n[resource]\n");
		}

		for (List<ExportData::PropertyData>::Element *E = resource.resources[i].properties.front(); E; E = E->next()) {

			String prop;
			_get_property_as_text(E->get().value, prop);
			f->store_line(_valprop(E->get().name) + " = " + prop);
		}
	}

	for (int i = 0; i < resource.nodes.size(); i++) {

		String node_txt = "\n[node";

		if (resource.nodes[i].name != String()) {
			node_txt += " name=\"" + String(resource.nodes[i].name).c_escape() + "\"";
		}

		if (resource.nodes[i].owner != NodePath()) {
			node_txt += " owner=\"" + String(resource.nodes[i].owner).c_escape() + "\"";
		}

		if (resource.nodes[i].type != String()) {
			node_txt += " type=\"" + resource.nodes[i].type + "\"";
		}

		if (resource.nodes[i].parent != NodePath()) {
			node_txt += " parent=\"" + String(resource.nodes[i].parent).c_escape() + "\"";
		}

		if (resource.nodes[i].instance != String()) {
			String prop;
			_get_property_as_text(resource.nodes[i].instance, prop);
			node_txt += " instance=" + prop + "";
		}

		if (!resource.nodes[i].groups.empty()) {
			node_txt += " groups=[\n";
			for (int j = 0; j < resource.nodes[i].groups.size(); j++) {
				node_txt += "\"" + resource.nodes[i].groups[j] + "\",\n";
			}
			node_txt += "]";
		}

		node_txt += "]\n";
		f->store_line(node_txt);

		for (List<ExportData::PropertyData>::Element *E = resource.nodes[i].properties.front(); E; E = E->next()) {

			String prop;
			_get_property_as_text(E->get().value, prop);
			f->store_line(_valprop(E->get().name) + " = " + prop);
		}
	}

	for (int i = 0; i < resource.connections.size(); i++) {

		String binds_array;
		_get_property_as_text(resource.connections[i].binds, binds_array);

		// CONNECT_PERSIST is the default
		String flags = (resource.connections[i].flags != Object::CONNECT_PERSIST) ? " flags=" + itos(resource.connections[i].flags) : String();

		f->store_line("\n[connection signal=\"" + resource.connections[i].signal + "\" from=\"" + String(resource.connections[i].from).c_escape() + "\" to=\"" + String(resource.connections[i].to).c_escape() + "\" method=\"" + resource.connections[i].method + "\" binds=" + binds_array + flags + "]");
	}

	for (int i = 0; i < resource.editables.size(); i++) {

		f->store_line("[editable path=\"" + String(resource.editables[i]).c_escape() + "\"]");
	}
}

enum {

	// numbering must be different from variant, in case new variant types are added (variant must be always contiguous for jumptable optimization)
	VARIANT_NIL = 1,
	VARIANT_BOOL = 2,
	VARIANT_INT = 3,
	VARIANT_REAL = 4,
	VARIANT_STRING = 5,
	VARIANT_VECTOR2 = 10,
	VARIANT_RECT2 = 11,
	VARIANT_VECTOR3 = 12,
	VARIANT_PLANE = 13,
	VARIANT_QUAT = 14,
	VARIANT_AABB = 15,
	VARIANT_MATRIX3 = 16,
	VARIANT_TRANSFORM = 17,
	VARIANT_MATRIX32 = 18,
	VARIANT_COLOR = 20,
	VARIANT_IMAGE = 21,
	VARIANT_NODE_PATH = 22,
	VARIANT_RID = 23,
	VARIANT_OBJECT = 24,
	VARIANT_INPUT_EVENT = 25,
	VARIANT_DICTIONARY = 26,
	VARIANT_ARRAY = 30,
	VARIANT_RAW_ARRAY = 31,
	VARIANT_INT_ARRAY = 32,
	VARIANT_REAL_ARRAY = 33,
	VARIANT_STRING_ARRAY = 34,
	VARIANT_VECTOR3_ARRAY = 35,
	VARIANT_COLOR_ARRAY = 36,
	VARIANT_VECTOR2_ARRAY = 37,
	VARIANT_INT64 = 40,
	VARIANT_DOUBLE = 41,

	IMAGE_ENCODING_EMPTY = 0,
	IMAGE_ENCODING_RAW = 1,
	IMAGE_ENCODING_LOSSLESS = 2,
	IMAGE_ENCODING_LOSSY = 3,

	OBJECT_EMPTY = 0,
	OBJECT_EXTERNAL_RESOURCE = 1,
	OBJECT_INTERNAL_RESOURCE = 2,
	OBJECT_EXTERNAL_RESOURCE_INDEX = 3,
	// version 2: added 64 bits support for float and int
	FORMAT_VERSION = 2,
	FORMAT_VERSION_CAN_RENAME_DEPS = 1

};

enum {
	IMAGE_FORMAT_L8, // luminance
	IMAGE_FORMAT_LA8, // luminance-alpha
	IMAGE_FORMAT_R8,
	IMAGE_FORMAT_RG8,
	IMAGE_FORMAT_RGB8,
	IMAGE_FORMAT_RGBA8,
	IMAGE_FORMAT_RGB565, // 16 bit
	IMAGE_FORMAT_RGBA4444,
	IMAGE_FORMAT_RGBA5551,
	IMAGE_FORMAT_RF, // float
	IMAGE_FORMAT_RGF,
	IMAGE_FORMAT_RGBF,
	IMAGE_FORMAT_RGBAF,
	IMAGE_FORMAT_RH, // half float
	IMAGE_FORMAT_RGH,
	IMAGE_FORMAT_RGBH,
	IMAGE_FORMAT_RGBAH,
	IMAGE_FORMAT_DXT1, // s3tc bc1
	IMAGE_FORMAT_DXT3, // bc2
	IMAGE_FORMAT_DXT5, // bc3
	IMAGE_FORMAT_ATI1, // bc4
	IMAGE_FORMAT_ATI2, // bc5
	IMAGE_FORMAT_BPTC_RGBA, // btpc bc6h
	IMAGE_FORMAT_BPTC_RGBF, // float /
	IMAGE_FORMAT_BPTC_RGBFU, // unsigned float
	IMAGE_FORMAT_PVRTC2, // pvrtc
	IMAGE_FORMAT_PVRTC2A,
	IMAGE_FORMAT_PVRTC4,
	IMAGE_FORMAT_PVRTC4A,
	IMAGE_FORMAT_ETC, // etc1
	IMAGE_FORMAT_ETC2_R11, // etc2
	IMAGE_FORMAT_ETC2_R11S, // signed, NOT srgb.
	IMAGE_FORMAT_ETC2_RG11,
	IMAGE_FORMAT_ETC2_RG11S,
	IMAGE_FORMAT_ETC2_RGB8,
	IMAGE_FORMAT_ETC2_RGBA8,
	IMAGE_FORMAT_ETC2_RGB8A1,

};

static void _pad_buffer(int p_bytes, FileAccess *f) {

	int extra = 4 - (p_bytes % 4);
	if (extra < 4) {
		for (int i = 0; i < extra; i++)
			f->store_8(0); // pad to 32
	}
}

static void save_unicode_string(const String &p_string, FileAccess *f, bool p_hi_bit = false) {

	CharString utf8 = p_string.utf8();
	f->store_32(uint32_t(utf8.length() + 1) | (p_hi_bit ? 0x80000000 : 0));
	f->store_buffer((const uint8_t *)utf8.get_data(), utf8.length() + 1);
}

void EditorExportGodot3::_save_binary_property(const Variant &p_property, FileAccess *f) {

	switch (p_property.get_type()) {

		case Variant::NIL: {

			f->store_32(VARIANT_NIL);
			// don't store anything
		} break;
		case Variant::BOOL: {

			f->store_32(VARIANT_BOOL);
			bool val = p_property;
			f->store_32(val);
		} break;
		case Variant::INT: {

			f->store_32(VARIANT_INT);
			int val = p_property;
			f->store_32(int32_t(val));

		} break;
		case Variant::REAL: {

			f->store_32(VARIANT_REAL);
			f->store_real(p_property);

		} break;
		case Variant::STRING: {

			String str = p_property;
			if (str.begins_with("@RESLOCAL:")) {
				f->store_32(VARIANT_OBJECT);
				f->store_32(OBJECT_INTERNAL_RESOURCE);
				f->store_32(str.get_slice(":", 1).to_int());
			} else if (str.begins_with("@RESEXTERNAL:")) {
				f->store_32(VARIANT_OBJECT);
				f->store_32(OBJECT_EXTERNAL_RESOURCE_INDEX);
				f->store_32(str.get_slice(":", 1).to_int());
			} else {

				f->store_32(VARIANT_STRING);
				save_unicode_string(str, f);
			}
		} break;
		case Variant::VECTOR2: {

			f->store_32(VARIANT_VECTOR2);
			Vector2 val = p_property;
			f->store_real(val.x);
			f->store_real(val.y);

		} break;
		case Variant::RECT2: {

			f->store_32(VARIANT_RECT2);
			Rect2 val = p_property;
			f->store_real(val.pos.x);
			f->store_real(val.pos.y);
			f->store_real(val.size.x);
			f->store_real(val.size.y);

		} break;
		case Variant::VECTOR3: {

			f->store_32(VARIANT_VECTOR3);
			Vector3 val = p_property;
			f->store_real(val.x);
			f->store_real(val.y);
			f->store_real(val.z);

		} break;
		case Variant::PLANE: {

			f->store_32(VARIANT_PLANE);
			Plane val = p_property;
			f->store_real(val.normal.x);
			f->store_real(val.normal.y);
			f->store_real(val.normal.z);
			f->store_real(val.d);

		} break;
		case Variant::QUAT: {

			f->store_32(VARIANT_QUAT);
			Quat val = p_property;
			f->store_real(val.x);
			f->store_real(val.y);
			f->store_real(val.z);
			f->store_real(val.w);

		} break;
		case Variant::_AABB: {

			f->store_32(VARIANT_AABB);
			Rect3 val = p_property;
			f->store_real(val.pos.x);
			f->store_real(val.pos.y);
			f->store_real(val.pos.z);
			f->store_real(val.size.x);
			f->store_real(val.size.y);
			f->store_real(val.size.z);

		} break;
		case Variant::MATRIX32: {

			f->store_32(VARIANT_MATRIX32);
			Matrix32 val = p_property;
			f->store_real(val.elements[0].x);
			f->store_real(val.elements[0].y);
			f->store_real(val.elements[1].x);
			f->store_real(val.elements[1].y);
			f->store_real(val.elements[2].x);
			f->store_real(val.elements[2].y);

		} break;
		case Variant::MATRIX3: {

			f->store_32(VARIANT_MATRIX3);
			Matrix3 val = p_property;
			f->store_real(val.elements[0].x);
			f->store_real(val.elements[0].y);
			f->store_real(val.elements[0].z);
			f->store_real(val.elements[1].x);
			f->store_real(val.elements[1].y);
			f->store_real(val.elements[1].z);
			f->store_real(val.elements[2].x);
			f->store_real(val.elements[2].y);
			f->store_real(val.elements[2].z);

		} break;
		case Variant::TRANSFORM: {

			f->store_32(VARIANT_TRANSFORM);
			Transform val = p_property;
			f->store_real(val.basis.elements[0].x);
			f->store_real(val.basis.elements[0].y);
			f->store_real(val.basis.elements[0].z);
			f->store_real(val.basis.elements[1].x);
			f->store_real(val.basis.elements[1].y);
			f->store_real(val.basis.elements[1].z);
			f->store_real(val.basis.elements[2].x);
			f->store_real(val.basis.elements[2].y);
			f->store_real(val.basis.elements[2].z);
			f->store_real(val.origin.x);
			f->store_real(val.origin.y);
			f->store_real(val.origin.z);

		} break;
		case Variant::COLOR: {

			f->store_32(VARIANT_COLOR);
			Color val = p_property;
			f->store_real(val.r);
			f->store_real(val.g);
			f->store_real(val.b);
			f->store_real(val.a);

		} break;
		case Variant::IMAGE: {

			f->store_32(VARIANT_IMAGE);
			Image val = p_property;
			if (val.empty()) {
				f->store_32(IMAGE_ENCODING_EMPTY);
				break;
			}

			f->store_32(IMAGE_ENCODING_RAW);

			f->store_32(val.get_width());
			f->store_32(val.get_height());
			f->store_32(val.get_mipmaps() ? 1 : 0);
			switch (val.get_format()) {
				case Image::FORMAT_GRAYSCALE:
					f->store_32(IMAGE_FORMAT_L8);
					break; ///< one byte per pixel: f->store_32(IMAGE_FORMAT_ ); break; 0-255
				case Image::FORMAT_INTENSITY:
					f->store_32(IMAGE_FORMAT_L8);
					break; ///< one byte per pixel: f->store_32(IMAGE_FORMAT_ ); break; 0-255
				case Image::FORMAT_GRAYSCALE_ALPHA:
					f->store_32(IMAGE_FORMAT_LA8);
					break; ///< two bytes per pixel: f->store_32(IMAGE_FORMAT_ ); break; 0-255. alpha 0-255
				case Image::FORMAT_RGB:
					f->store_32(IMAGE_FORMAT_RGB8);
					break; ///< one byte R: f->store_32(IMAGE_FORMAT_ ); break; one byte G: f->store_32(IMAGE_FORMAT_ ); break; one byte B
				case Image::FORMAT_RGBA:
					f->store_32(IMAGE_FORMAT_RGBA8);
					break; ///< one byte R: f->store_32(IMAGE_FORMAT_ ); break; one byte G: f->store_32(IMAGE_FORMAT_ ); break; one byte B: f->store_32(IMAGE_FORMAT_ ); break; one byte A
				case Image::FORMAT_BC1:
					f->store_32(IMAGE_FORMAT_DXT1);
					break; // DXT1
				case Image::FORMAT_BC2:
					f->store_32(IMAGE_FORMAT_DXT3);
					break; // DXT3
				case Image::FORMAT_BC3:
					f->store_32(IMAGE_FORMAT_DXT5);
					break; // DXT5
				case Image::FORMAT_BC4:
					f->store_32(IMAGE_FORMAT_ATI1);
					break; // ATI1
				case Image::FORMAT_BC5:
					f->store_32(IMAGE_FORMAT_ATI2);
					break; // ATI2
				case Image::FORMAT_PVRTC2: f->store_32(IMAGE_FORMAT_PVRTC2); break;
				case Image::FORMAT_PVRTC2_ALPHA: f->store_32(IMAGE_FORMAT_PVRTC2A); break;
				case Image::FORMAT_PVRTC4: f->store_32(IMAGE_FORMAT_PVRTC4); break;
				case Image::FORMAT_PVRTC4_ALPHA: f->store_32(IMAGE_FORMAT_PVRTC4A); break;
				case Image::FORMAT_ETC:
					f->store_32(IMAGE_FORMAT_ETC);
					break; // regular ETC: f->store_32(IMAGE_FORMAT_ ); break; no transparency
				default: f->store_32(IMAGE_FORMAT_L8); break;
			}

			int dlen = val.get_data().size();
			f->store_32(dlen);
			DVector<uint8_t>::Read r = val.get_data().read();
			f->store_buffer(r.ptr(), dlen);
			_pad_buffer(dlen, f);

		} break;
		case Variant::NODE_PATH: {
			f->store_32(VARIANT_NODE_PATH);
			NodePath np = p_property;
			f->store_16(np.get_name_count());
			uint16_t snc = np.get_subname_count();
			if (np.is_absolute())
				snc |= 0x8000;
			f->store_16(snc);
			for (int i = 0; i < np.get_name_count(); i++) {
				save_unicode_string(np.get_name(i), f, true);
			}
			for (int i = 0; i < np.get_subname_count(); i++) {
				save_unicode_string(np.get_subname(i), f, true);
			}

			save_unicode_string(np.get_property(), f, true);

		} break;
		case Variant::_RID: {

			f->store_32(VARIANT_RID);
			WARN_PRINT("Can't save RIDs");
			RID val = p_property;
			f->store_32(val.get_id());
		} break;
		case Variant::OBJECT: {

			ERR_FAIL();

		} break;
		case Variant::INPUT_EVENT: {

			f->store_32(VARIANT_INPUT_EVENT);
			// InputEvent event = p_property;
			f->store_32(0); // event type none, nothing else supported for now.

		} break;
		case Variant::DICTIONARY: {

			f->store_32(VARIANT_DICTIONARY);
			Dictionary d = p_property;
			f->store_32(uint32_t(d.size()));

			List<Variant> keys;
			d.get_key_list(&keys);

			for (List<Variant>::Element *E = keys.front(); E; E = E->next()) {

				/*
				if (!_check_type(dict[E->get()]))
					continue;
				*/

				_save_binary_property(E->get(), f);
				_save_binary_property(d[E->get()], f);
			}

		} break;
		case Variant::ARRAY: {

			f->store_32(VARIANT_ARRAY);
			Array a = p_property;
			f->store_32(uint32_t(a.size()));
			for (int i = 0; i < a.size(); i++) {

				_save_binary_property(a[i], f);
			}

		} break;
		case Variant::RAW_ARRAY: {

			f->store_32(VARIANT_RAW_ARRAY);
			DVector<uint8_t> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<uint8_t>::Read r = arr.read();
			f->store_buffer(r.ptr(), len);
			_pad_buffer(len, f);

		} break;
		case Variant::INT_ARRAY: {

			f->store_32(VARIANT_INT_ARRAY);
			DVector<int> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<int>::Read r = arr.read();
			for (int i = 0; i < len; i++)
				f->store_32(r[i]);

		} break;
		case Variant::REAL_ARRAY: {

			f->store_32(VARIANT_REAL_ARRAY);
			DVector<real_t> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<real_t>::Read r = arr.read();
			for (int i = 0; i < len; i++) {
				f->store_real(r[i]);
			}

		} break;
		case Variant::STRING_ARRAY: {

			f->store_32(VARIANT_STRING_ARRAY);
			DVector<String> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<String>::Read r = arr.read();
			for (int i = 0; i < len; i++) {
				save_unicode_string(r[i], f);
			}

		} break;
		case Variant::VECTOR3_ARRAY: {

			f->store_32(VARIANT_VECTOR3_ARRAY);
			DVector<Vector3> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<Vector3>::Read r = arr.read();
			for (int i = 0; i < len; i++) {
				f->store_real(r[i].x);
				f->store_real(r[i].y);
				f->store_real(r[i].z);
			}

		} break;
		case Variant::VECTOR2_ARRAY: {

			f->store_32(VARIANT_VECTOR2_ARRAY);
			DVector<Vector2> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<Vector2>::Read r = arr.read();
			for (int i = 0; i < len; i++) {
				f->store_real(r[i].x);
				f->store_real(r[i].y);
			}

		} break;
		case Variant::COLOR_ARRAY: {

			f->store_32(VARIANT_COLOR_ARRAY);
			DVector<Color> arr = p_property;
			int len = arr.size();
			f->store_32(len);
			DVector<Color>::Read r = arr.read();
			for (int i = 0; i < len; i++) {
				f->store_real(r[i].r);
				f->store_real(r[i].g);
				f->store_real(r[i].b);
				f->store_real(r[i].a);
			}

		} break;
		default: {

			ERR_EXPLAIN("Invalid variant");
			ERR_FAIL();
		}
	}
}

void EditorExportGodot3::_save_binary(const String &p_path, ExportData &resource) {

	FileAccessRef f = FileAccess::open(p_path, FileAccess::WRITE);
	ERR_FAIL_COND(!f.operator->());

	// save header compressed
	static const uint8_t header[4] = { 'R', 'S', 'R', 'C' };
	f->store_buffer(header, 4);
	f->store_32(0);

	f->store_32(0); // 64 bits file, false for now
	f->store_32(3); // major
	f->store_32(0); // minor
	f->store_32(2); // format version (2 is for 3.0)

	// f->store_32(saved_resources.size()+external_resources.size()); // load steps -not needed
	save_unicode_string(resource.resources[resource.resources.size() - 1].type, f.operator->());
	for (int i = 0; i < 16; i++)
		f->store_32(0); // unused

	f->store_32(0); // no names saved

	f->store_32(resource.dependencies.size()); // amount of external resources

	for (Map<int, ExportData::Dependency>::Element *E = resource.dependencies.front(); E; E = E->next()) {

		save_unicode_string(E->get().type, f.operator->());
		save_unicode_string(resource_replace_map[E->get().path], f.operator->());
	}

	// save internal resource table
	Vector<uint64_t> ofs_pos;
	f->store_32(resource.resources.size()); // amount of internal resources
	for (int i = 0; i < resource.resources.size(); i++) {

		save_unicode_string("local://" + itos(resource.resources[i].index), f.operator->());
		ofs_pos.push_back(f->get_pos());
		f->store_64(0);
	}

	Vector<uint64_t> ofs_table;
	//	int saved_idx=0;
	// now actually save the resources

	for (int i = 0; i < resource.resources.size(); i++) {

		ofs_table.push_back(f->get_pos());
		save_unicode_string(resource.resources[i].type, f.operator->());
		f->store_32(resource.resources[i].properties.size());

		for (List<ExportData::PropertyData>::Element *E = resource.resources[i].properties.front(); E; E = E->next()) {

			save_unicode_string(E->get().name, f.operator->(), true);
			_save_binary_property(E->get().value, f.operator->());
		}
	}

	for (int i = 0; i < ofs_table.size(); i++) {
		f->seek(ofs_pos[i]);
		f->store_64(ofs_table[i]);
	}

	f->seek_end();

	f->store_buffer((const uint8_t *)"RSRC", 4); // magic at end

	ERR_FAIL_COND(f->get_error() != OK);
}

void EditorExportGodot3::_save_config(const String &p_path) {

	// Parse existing config, convert persisting properties and store in ConfigFile
	ConfigFile new_cfg = ConfigFile();

	List<PropertyInfo> props;
	Globals::get_singleton()->get_property_list(&props);

	for (List<PropertyInfo>::Element *E = props.front(); E; E = E->next()) {

		if (!Globals::get_singleton()->has(E->get().name))
			continue;

		if (Globals::get_singleton()->is_persisting(E->get().name) && !E->get().name.begins_with("godot3_export/")) {
			String newname;
			if (globals_rename_map.has(E->get().name)) {
				newname = globals_rename_map[E->get().name];
			} else {
				newname = E->get().name;
			}

			int sep = newname.find("/");
			String section = newname.substr(0, sep);
			String subname = newname.substr(sep + 1, newname.length());
			Variant v = Globals::get_singleton()->get(E->get().name);
			if (v.get_type() == Variant::STRING) {
				// Paths to scenes and resources may have been renamed (main scene, autoloads, icon...)
				String str = v;
				String prefix = str.begins_with("*") ? "*" : "";
				str = str.substr(prefix.length(), str.length());
				if (str.begins_with("res://")) {
					v = prefix + _replace_resource(str);
				}
			}

			String value;
			_get_property_as_text(v, value);
			new_cfg.set_value(section, subname, value);
		}
	}

	String str = "{\n\"flags/filter\": " + String(GLOBAL_DEF("image_loader/filter", true) ? "true" : "false");
	str += ",\n\"flags/mipmaps\": " + String(GLOBAL_DEF("image_loader/gen_mipmaps", true) ? "true" : "false");
	str += ",\n\"flags/repeat\": " + String(GLOBAL_DEF("image_loader/repeat", false) ? "1" : "0");
	str += "\n}";
	new_cfg.set_value("importer_defaults", "texture", str);

	// Write the collected ConfigFile manually - we need to use _get_property_as_text()
	// above, so we can't rely on ConfigFile.save() to properly store the raw strings.
	FileAccessRef f = FileAccess::open(p_path.plus_file("project.godot"), FileAccess::WRITE);

	// Godot 3 converts the input actions to its newer format based on this version
	f->store_line("config_version=3\n");

	if (class_names.size()) {
		// Global classes, so they're known before the editor scans the project
		String classes = "_global_script_classes=[ ";
		String icons = "_global_script_class_icons={\n";
		bool first = true;
		for (Map<String, ClassNameData>::Element *E = class_names.front(); E; E = E->next()) {
			String path = _replace_resource(E->key());
			String icon = E->get().extra.replace(",", "").strip_edges().replace("\"", "").replace("'", "");
			classes += String(first ? "" : ", ") + "{\n\"base\": \"" + E->get().base + "\",\n\"class\": \"" + E->get().name + "\",\n\"language\": \"GDScript\",\n\"path\": \"" + path + "\"\n}";
			icons += String(first ? "" : ",\n") + "\"" + E->get().name + "\": \"" + icon + "\"";
			first = false;
		}
		f->store_line(classes + " ]");
		f->store_line(icons + "\n}\n");
	}

	List<String> sections;
	new_cfg.get_sections(&sections);

	for (List<String>::Element *E = sections.front(); E; E = E->next()) {

		f->store_line("[" + E->get() + "]\n");

		List<String> keys;
		new_cfg.get_section_keys(E->get(), &keys);

		for (List<String>::Element *F = keys.front(); F; F = F->next()) {

			f->store_line(F->get() + " = " + new_cfg.get_value(E->get(), F->get()));
		}
		f->store_line("");
	}

	f->close();
}

/* GDScript conversion */

// Strings are replaced by single placeholder characters while converting the code,
// so that the conversions never alter them and can treat them as atoms.
#define STRING_PLACEHOLDER_BASE 0xE000

static bool _is_ident_char(CharType c) {

	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static bool _is_placeholder(CharType c) {

	return c >= STRING_PLACEHOLDER_BASE && c < STRING_PLACEHOLDER_BASE + 0x1000;
}

static void _split_script_line(const String &p_line, String &r_code, Vector<String> &r_strings, String &r_comment, String &r_multiline) {

	r_code = "";
	r_comment = "";
	r_strings.clear();

	int len = p_line.length();
	int i = 0;
	while (i < len) {

		if (r_multiline != "") {
			// Inside a multiline string started on a previous line
			int end = p_line.find(r_multiline, i);
			int stop = (end == -1) ? len : end + 3;
			r_code += CharType(STRING_PLACEHOLDER_BASE + r_strings.size());
			r_strings.push_back(p_line.substr(i, stop - i));
			if (end != -1) {
				r_multiline = "";
			}
			i = stop;
			continue;
		}

		CharType c = p_line[i];

		if (c == '#') {
			r_comment = p_line.substr(i, len - i);
			break;
		}

		if (c == '"' || c == '\'') {
			String quote = String::chr(c);
			String triple = quote + quote + quote;
			if (p_line.substr(i, 3) == triple) {
				r_multiline = triple;
				int end = p_line.find(triple, i + 3);
				int stop = (end == -1) ? len : end + 3;
				r_code += CharType(STRING_PLACEHOLDER_BASE + r_strings.size());
				r_strings.push_back(p_line.substr(i, stop - i));
				if (end != -1) {
					r_multiline = "";
				}
				i = stop;
				continue;
			}

			int j = i + 1;
			while (j < len && p_line[j] != c) {
				if (p_line[j] == '\\') {
					j++;
				}
				j++;
			}
			int stop = MIN(j + 1, len);
			r_code += CharType(STRING_PLACEHOLDER_BASE + r_strings.size());
			r_strings.push_back(p_line.substr(i, stop - i));
			i = stop;
			continue;
		}

		r_code += c;
		i++;
	}
}

static String _join_script_line(const String &p_code, const Vector<String> &p_strings, const String &p_comment) {

	String line;
	for (int i = 0; i < p_code.length(); i++) {
		CharType c = p_code[i];
		if (_is_placeholder(c) && c - STRING_PLACEHOLDER_BASE < p_strings.size()) {
			line += p_strings[c - STRING_PLACEHOLDER_BASE];
		} else {
			line += c;
		}
	}
	return line + p_comment;
}

static Vector<String> _read_lines(const String &p_path) {

	Vector<String> lines;
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	while (f && !f->eof_reached()) {
		lines.push_back(f->get_line());
	}
	return lines;
}

static String _rename_identifier(const String &p_line, const String &p_from, const String &p_to) {

	// Rename the identifier p_from, but not in strings and comments, nor when accessed with a dot
	String code;
	String comment;
	String multiline;
	Vector<String> strings;
	_split_script_line(p_line, code, strings, comment, multiline);

	String out;
	int i = 0;
	while (i < code.length()) {
		if (code.substr(i, p_from.length()) == p_from && (i == 0 || (!_is_ident_char(code[i - 1]) && code[i - 1] != '.')) && (i + p_from.length() >= code.length() || !_is_ident_char(code[i + p_from.length()]))) {
			out += p_to;
			i += p_from.length();
		} else {
			out += code[i];
			i++;
		}
	}

	return _join_script_line(out, strings, comment);
}

static int _find_closing(const String &p_code, int p_open) {

	int depth = 0;
	for (int i = p_open; i < p_code.length(); i++) {
		CharType c = p_code[i];
		if (c == '(' || c == '[' || c == '{') {
			depth++;
		} else if (c == ')' || c == ']' || c == '}') {
			depth--;
			if (depth == 0) {
				return i;
			}
		}
	}
	return -1;
}

static Vector<String> _split_args(const String &p_args) {

	Vector<String> args;
	if (p_args.strip_edges() == "") {
		return args;
	}

	int depth = 0;
	int from = 0;
	for (int i = 0; i < p_args.length(); i++) {
		CharType c = p_args[i];
		if (c == '(' || c == '[' || c == '{') {
			depth++;
		} else if (c == ')' || c == ']' || c == '}') {
			depth--;
		} else if (c == ',' && depth == 0) {
			args.push_back(p_args.substr(from, i - from).strip_edges());
			from = i + 1;
		}
	}
	args.push_back(p_args.substr(from, p_args.length() - from).strip_edges());
	return args;
}

static String _join_args(const Vector<String> &p_args, int p_from = 0) {

	String str;
	for (int i = p_from; i < p_args.size(); i++) {
		if (i > p_from) {
			str += ", ";
		}
		str += p_args[i];
	}
	return str;
}

// Start of the expression ending right before p_end (exclusive), e.g. "get_node("A").b" in "x = get_node("A").b"
static int _get_receiver_start(const String &p_code, int p_end) {

	int i = p_end - 1;
	while (i >= 0) {
		CharType c = p_code[i];
		if (c == ')' || c == ']') {
			int depth = 0;
			while (i >= 0) {
				if (p_code[i] == ')' || p_code[i] == ']') {
					depth++;
				} else if (p_code[i] == '(' || p_code[i] == '[') {
					depth--;
					if (depth == 0) {
						break;
					}
				}
				i--;
			}
			i--;
		} else if (_is_ident_char(c) || _is_placeholder(c) || c == '.') {
			i--;
		} else {
			break;
		}
	}
	return MAX(0, i + 1);
}

// An expression which can be negated or used as receiver without parentheses
static bool _is_simple_expression(const String &p_expr) {

	if (p_expr == "") {
		return false;
	}

	int depth = 0;
	for (int i = 0; i < p_expr.length(); i++) {
		CharType c = p_expr[i];
		if (c == '(' || c == '[') {
			depth++;
		} else if (c == ')' || c == ']') {
			depth--;
		} else if (depth == 0 && !_is_ident_char(c) && !_is_placeholder(c) && c != '.') {
			return false;
		}
	}
	return true;
}

static String _negate_expression(const String &p_expr) {

	String expr = p_expr.strip_edges();
	if (expr.begins_with("-") && _is_simple_expression(expr.substr(1, expr.length() - 1))) {
		return expr.substr(1, expr.length() - 1);
	}
	if (_is_simple_expression(expr)) {
		return "-" + expr;
	}
	return "-(" + expr + ")";
}

static String _not_expression(const String &p_expr) {

	String expr = p_expr.strip_edges();
	if (expr == "true") {
		return "false";
	} else if (expr == "false") {
		return "true";
	} else if (expr.begins_with("not ") && _is_simple_expression(expr.substr(4, expr.length()).strip_edges())) {
		return expr.substr(4, expr.length()).strip_edges();
	} else if (_is_simple_expression(expr)) {
		return "not " + expr;
	}
	return "not (" + expr + ")";
}

static String _choose(const String &p_condition, const String &p_if_true, const String &p_if_false) {

	String cond = p_condition.strip_edges();
	if (cond == "true") {
		return p_if_true;
	} else if (cond == "false") {
		return p_if_false;
	}
	return p_if_true + " if " + cond + " else " + p_if_false;
}

static String _with_receiver(const String &p_receiver, const String &p_member) {

	return p_receiver == "" ? p_member : p_receiver + "." + p_member;
}

// Wrap an expression starting with a unary operator when it follows another operator,
// Godot 3 refuses two consecutive operators ("a == not b", "a * -b()")
static String _wrap_unary(const String &p_before, const String &p_expr) {

	String before = p_before.strip_edges();
	if (before == String()) {
		return p_expr;
	}

	CharType last = before[before.length() - 1];
	bool after_operator = false;
	if (last == '=') {
		// Not after an assignment
		CharType prev = before.length() > 1 ? before[before.length() - 2] : CharType(' ');
		after_operator = prev == '=' || prev == '!' || prev == '<' || prev == '>';
	} else if (last == '+' || last == '-' || last == '*' || last == '/' || last == '%' || last == '<' || last == '>' || last == '!' || last == '&' || last == '|' || last == '^' || last == '~') {
		after_operator = true;
	} else if (before == "not" || before.ends_with(" not") || before.ends_with("(not") || before.ends_with("\tnot")) {
		after_operator = true;
	}

	return after_operator ? "(" + p_expr + ")" : p_expr;
}

static const char *script_method_renames[][2] = {
	{ "get_instance_ID", "get_instance_id" },
	{ "is_type", "is_class" },
	{ "set_fixed_process", "set_physics_process" },
	{ "is_fixed_processing", "is_physics_processing" },
	{ "get_fixed_process_delta_time", "get_physics_process_delta_time" },
	{ "get_item_rect", "get_rect" },
	{ "make_canvas_pos_local", "make_canvas_position_local" },
	{ "get_z", "get_z_index" },
	{ "set_z", "set_z_index" },
	{ "add_style_override", "add_stylebox_override" },
	{ "get_connected_joysticks", "get_connected_joypads" },
	{ "get_mouse_speed", "get_last_mouse_speed" },
	{ "get_item_ID", "get_item_id" },
	{ "set_item_ID", "set_item_id" },
	{ "get_selected_ID", "get_selected_id" },
	{ "get_unique_ID", "get_unique_id" },
	{ "get_process_ID", "get_process_id" },
	{ "set_enable_monitoring", "set_monitoring" },
	{ "is_monitoring_enabled", "is_monitoring" },
	{ "get_val", "get_value" },
	{ "set_val", "set_value" },
	{ "get_unit_value", "get_as_ratio" },
	{ "set_unit_value", "set_as_ratio" },
	{ "is_rounded_values", "is_using_rounded_values" },
	{ "set_rounded_values", "set_use_rounded_values" },
	{ "set_exp_unit_value", "set_exp_ratio" },
	{ "is_unit_value_exp", "is_ratio_exp" },
	{ "get_border_size", "get_border_width_min" },
	{ "set_border_size", "set_border_width_all" },
	{ "set_light_color", "set_border_color" },
	{ "set_dark_color", "set_border_color" },
	{ "get_light_color", "get_border_color" },
	{ "get_dark_color", "get_border_color" },
	{ "get_draw_center", "is_draw_center_enabled" },
	{ "stop_all", "stop" },
	{ "get_render_target_texture", "get_texture" },
	{ "set_render_target_vflip", "set_vflip" },
	{ "get_render_target_vflip", "get_vflip" },
	{ "set_render_target_update_mode", "set_update_mode" },
	{ "get_render_target_update_mode", "get_update_mode" },
	{ "tesselate", "tessellate" },
	{ "to_32", "to_argb32" },
	{ "to_ARGB32", "to_argb32" },
	{ "floorf", "floor" },
	{ "get_aspect", "aspect" },
	{ "look_at_from_pos", "look_at_from_position" },
	{ "get_collision_pos", "get_collision_point" },
	{ "get_speed", "get_speed_scale" },
	{ "set_speed", "set_speed_scale" },
	{ "set_wrap", "set_wrap_enabled" }, // TextEdit
	{ "is_move_and_slide_on_floor", "is_on_floor" },
	{ "is_move_and_slide_on_ceiling", "is_on_ceiling" },
	{ "is_move_and_slide_on_wall", "is_on_wall" },
	{ NULL, NULL }
};

// Methods which moved from OS to Engine
static const char *script_engine_methods[] = {
	"get_frames_drawn",
	"get_frames_per_second",
	"get_iterations_per_second",
	"set_iterations_per_second",
	"get_target_fps",
	"set_target_fps",
	"get_time_scale",
	"set_time_scale",
	"get_main_loop",
	NULL
};

// 2D rotation setters and getters: the name in 3.x, the angles have to be negated
static const char *script_rotation_setters[][2] = {
	{ "set_rot", "set_rotation" },
	{ "set_rotd", "set_rotation_degrees" },
	{ "set_global_rot", "set_global_rotation" },
	{ "set_global_rotd", "set_global_rotation_degrees" },
	{ "set_rotation_deg", "set_rotation_degrees" }, // Control
	{ "set_rotationd", "set_rotation_degrees" }, // CanvasLayer
	{ "set_rotation", "set_rotation" }, // Control, CanvasLayer
	{ NULL, NULL }
};

static const char *script_rotation_getters[][2] = {
	{ "get_rot", "get_rotation" },
	{ "get_rotd", "get_rotation_degrees" },
	{ "get_global_rot", "get_global_rotation" },
	{ "get_global_rotd", "get_global_rotation_degrees" },
	{ "get_rotation_deg", "get_rotation_degrees" },
	{ "get_rotationd", "get_rotation_degrees" },
	{ "get_rotation", "get_rotation" },
	{ NULL, NULL }
};

static const char *script_identifier_renames[][2] = {
	{ "Globals", "ProjectSettings" },
	{ "PS2D", "Physics2DServer" },
	{ "PS", "PhysicsServer" },
	{ "AS", "AudioServer" },
	{ "VS", "VisualServer" },
	{ "TS", "TranslationServer" },
	{ "KEY_ENTER", "KEY_KP_ENTER" },
	{ "KEY_RETURN", "KEY_ENTER" },
	{ "ERR_WTF", "ERR_BUG" },
	{ "ERR_CANT_AQUIRE_RESOURCE", "ERR_CANT_ACQUIRE_RESOURCE" },
	{ "JOY_ANALOG_0_X", "JOY_ANALOG_LX" },
	{ "JOY_ANALOG_0_Y", "JOY_ANALOG_LY" },
	{ "JOY_ANALOG_1_X", "JOY_ANALOG_RX" },
	{ "JOY_ANALOG_1_Y", "JOY_ANALOG_RY" },
	{ "JOY_ANALOG_2_X", "JOY_ANALOG_L2" },
	{ "JOY_ANALOG_2_Y", "JOY_ANALOG_R2" },
	{ "JOY_SNES_A", "JOY_DS_A" },
	{ "JOY_SNES_B", "JOY_DS_B" },
	{ "JOY_SNES_X", "JOY_DS_X" },
	{ "JOY_SNES_Y", "JOY_DS_Y" },
	{ "JOY_SEGA_A", "JOY_XBOX_A" },
	{ "JOY_SEGA_B", "JOY_XBOX_B" },
	{ "JOY_SEGA_X", "JOY_XBOX_X" },
	{ "JOY_SEGA_Y", "JOY_XBOX_Y" },
	{ "TYPE_MATRIX32", "TYPE_TRANSFORM2D" },
	{ "TYPE_MATRIX3", "TYPE_BASIS" },
	{ "TYPE_IMAGE", "TYPE_OBJECT" },
	{ "TYPE_INPUT_EVENT", "TYPE_OBJECT" },
	{ "NOTIFICATION_FIXED_PROCESS", "NOTIFICATION_PHYSICS_PROCESS" },
	{ NULL, NULL }
};

// Members accessed with a dot (InputEvent, Rect2...)
static const char *script_dot_member_renames[][2] = {
	{ "pos", "position" },
	{ "global_pos", "global_position" },
	{ "relative_pos", "relative" },
	{ "relative_x", "relative.x" },
	{ "relative_y", "relative.y" },
	{ "speed_x", "speed.x" },
	{ "speed_y", "speed.y" },
	{ NULL, NULL }
};

static const char *script_image_formats[][2] = {
	{ "FORMAT_GRAYSCALE", "FORMAT_L8" },
	{ "FORMAT_INTENSITY", "FORMAT_L8" },
	{ "FORMAT_GRAYSCALE_ALPHA", "FORMAT_LA8" },
	{ "FORMAT_RGB", "FORMAT_RGB8" },
	{ "FORMAT_RGBA", "FORMAT_RGBA8" },
	{ "FORMAT_INDEXED", "FORMAT_RGB8" },
	{ "FORMAT_INDEXED_ALPHA", "FORMAT_RGBA8" },
	{ "FORMAT_BC1", "FORMAT_DXT1" },
	{ "FORMAT_BC2", "FORMAT_DXT3" },
	{ "FORMAT_BC3", "FORMAT_DXT5" },
	{ "FORMAT_BC4", "FORMAT_RGTC_R" },
	{ "FORMAT_BC5", "FORMAT_RGTC_RG" },
	{ NULL, NULL }
};

static const char *script_input_event_types[][2] = {
	{ "KEY", "InputEventKey" },
	{ "MOUSE_MOTION", "InputEventMouseMotion" },
	{ "MOUSE_BUTTON", "InputEventMouseButton" },
	{ "JOYSTICK_MOTION", "InputEventJoypadMotion" },
	{ "JOYSTICK_BUTTON", "InputEventJoypadButton" },
	{ "SCREEN_TOUCH", "InputEventScreenTouch" },
	{ "SCREEN_DRAG", "InputEventScreenDrag" },
	{ "ACTION", "InputEventAction" },
	{ NULL, NULL }
};

// Properties of Godot 3 classes (own ones, not inherited), which can't be redeclared by scripts
static const char *godot3_members[][2] = {
	{ "Node", "custom_multiplayer filename multiplayer name owner pause_mode physics_interpolation_mode process_priority unique_name_in_owner" },
	{ "CanvasItem", "light_mask material modulate self_modulate show_behind_parent show_on_top use_parent_material visible" },
	{ "Node2D", "global_position global_rotation global_rotation_degrees global_scale global_transform position rotation rotation_degrees scale transform z_as_relative z_index" },
	{ "Control", "anchor_bottom anchor_left anchor_right anchor_top focus_mode focus_neighbour_bottom focus_neighbour_left focus_neighbour_right focus_neighbour_top focus_next focus_previous grow_horizontal grow_vertical hint_tooltip input_pass_on_modal_close_click margin_bottom margin_left margin_right margin_top mouse_default_cursor_shape mouse_filter physics_interpolation_mode rect_clip_content rect_global_position rect_min_size rect_pivot_offset rect_position rect_rotation rect_scale rect_size size_flags_horizontal size_flags_stretch_ratio size_flags_vertical theme theme_type_variation" },
	{ "Spatial", "gizmo global_position global_rotation global_transform global_translation lod_range merging_mode position rotation rotation_degrees scale transform translation visible" },
	{ "Sprite", "centered flip_h flip_v frame frame_coords hframes normal_map offset region_enabled region_filter_clip region_rect texture vframes" },
	{ "AnimatedSprite", "animation centered flip_h flip_v frame frames offset playing speed_scale" },
	{ "Label", "align autowrap clip_text lines_skipped max_lines_visible mouse_filter percent_visible size_flags_vertical text uppercase valign visible_characters" },
	{ "Button", "align clip_text expand_icon flat icon icon_align text" },
	{ "BaseButton", "action_mode button_mask disabled enabled_focus_mode focus_mode group keep_pressed_outside pressed shortcut shortcut_in_tooltip toggle_mode" },
	{ "LineEdit", "align caret_blink caret_blink_speed caret_position clear_button_enabled context_menu_enabled deselect_on_focus_loss_enabled editable expand_to_text_length focus_mode max_length middle_mouse_paste_enabled mouse_default_cursor_shape placeholder_alpha placeholder_text right_icon secret secret_character selecting_enabled shortcut_keys_enabled text virtual_keyboard_enabled virtual_keyboard_type" },
	{ "TextEdit", "bookmark_gutter breakpoint_gutter caret_blink caret_blink_speed caret_block_mode caret_moving_by_right_click context_menu_enabled deselect_on_focus_loss_enabled drag_and_drop_selection_enabled draw_spaces draw_tabs focus_mode fold_gutter hiding_enabled highlight_all_occurrences highlight_current_line middle_mouse_paste_enabled minimap_draw minimap_width mouse_default_cursor_shape override_selected_font_color readonly scroll_horizontal scroll_vertical selecting_enabled shortcut_keys_enabled show_line_numbers smooth_scrolling syntax_highlighting text v_scroll_speed virtual_keyboard_enabled wrap_enabled" },
	{ "RichTextLabel", "bbcode_enabled bbcode_text custom_effects deselect_on_focus_loss_enabled fit_content_height meta_underlined override_selected_font_color percent_visible rect_clip_content scroll_active scroll_following selection_enabled tab_size text visible_characters" },
	{ "Camera2D", "anchor_mode current custom_viewport drag_margin_bottom drag_margin_h_enabled drag_margin_left drag_margin_right drag_margin_top drag_margin_v_enabled editor_draw_drag_margin editor_draw_limits editor_draw_screen limit_bottom limit_left limit_right limit_smoothed limit_top offset offset_h offset_v process_mode rotating smoothing_enabled smoothing_speed zoom" },
	{ "Area2D", "angular_damp audio_bus_name audio_bus_override gravity gravity_distance_scale gravity_point gravity_vec linear_damp monitorable monitoring priority space_override" },
	{ "KinematicBody2D", "moving_platform_apply_velocity_on_leave" },
	{ "RigidBody2D", "angular_damp angular_velocity applied_force applied_torque bounce can_sleep contact_monitor contacts_reported continuous_cd custom_integrator friction gravity_scale inertia linear_damp linear_velocity mass mode physics_material_override sleeping weight" },
	{ "PhysicsBody2D", "input_pickable layers" },
	{ "CollisionObject2D", "collision_layer collision_mask input_pickable" },
	{ "Light2D", "color editor_only enabled energy mode offset range_height range_item_cull_mask range_layer_max range_layer_min range_z_max range_z_min shadow_buffer_size shadow_color shadow_enabled shadow_filter shadow_filter_smooth shadow_gradient_length shadow_item_cull_mask texture texture_scale" },
	{ "Polygon2D", "antialiased bones color internal_vertex_count invert_border invert_enable offset polygon polygons skeleton texture texture_offset texture_rotation texture_rotation_degrees texture_scale uv vertex_colors" },
	{ "Line2D", "antialiased begin_cap_mode default_color end_cap_mode gradient joint_mode points round_precision sharp_limit texture texture_mode width width_curve" },
	{ "TextureRect", "expand flip_h flip_v mouse_filter stretch_mode texture" },
	{ "NinePatchRect", "axis_stretch_horizontal axis_stretch_vertical draw_center mouse_filter patch_margin_bottom patch_margin_left patch_margin_right patch_margin_top region_rect texture" },
	{ "TextureButton", "expand flip_h flip_v stretch_mode texture_click_mask texture_disabled texture_focused texture_hover texture_normal texture_pressed" },
	{ "TextureProgress", "fill_mode mouse_filter nine_patch_stretch radial_center_offset radial_fill_degrees radial_initial_angle stretch_margin_bottom stretch_margin_left stretch_margin_right stretch_margin_top texture_over texture_progress texture_progress_offset texture_under tint_over tint_progress tint_under" },
	{ "Range", "allow_greater allow_lesser exp_edit max_value min_value page ratio rounded step value" },
	{ "ProgressBar", "percent_visible size_flags_vertical step" },
	{ "Timer", "autostart one_shot paused process_mode time_left wait_time" },
	{ "Tween", "playback_process_mode playback_speed repeat" },
	{ "AnimationPlayer", "assigned_animation autoplay current_animation current_animation_length current_animation_position method_call_mode playback_active playback_default_blend_time playback_process_mode playback_speed reset_on_save root_node" },
	{ "AudioStreamPlayer", "autoplay bus mix_target pitch_scale playing stream stream_paused volume_db" },
	{ "AudioStreamPlayer2D", "area_mask attenuation autoplay bus max_distance panning_strength pitch_scale playing stream stream_paused volume_db" },
	{ "Path2D", "curve self_modulate" },
	{ "PathFollow2D", "cubic_interp h_offset lookahead loop offset rotate unit_offset v_offset" },
	{ "TileMap", "bake_navigation cell_clip_uv cell_custom_transform cell_half_offset cell_quadrant_size cell_size cell_tile_origin cell_y_sort centered_textures collision_bounce collision_friction collision_layer collision_mask collision_use_kinematic collision_use_parent compatibility_mode mode navigation_layers occluder_light_mask show_collision tile_set" },
	{ "Viewport", "arvr audio_listener_enable_2d audio_listener_enable_3d canvas_transform debanding debug_draw disable_3d fxaa global_canvas_transform gui_disable_input gui_snap_controls_to_pixels handle_input_locally hdr keep_3d_linear msaa own_world physics_interpolation_mode physics_object_picking render_direct_to_screen render_target_clear_mode render_target_update_mode render_target_v_flip shadow_atlas_quad_0 shadow_atlas_quad_1 shadow_atlas_quad_2 shadow_atlas_quad_3 shadow_atlas_size sharpen_intensity size size_override_stretch transparent_bg usage use_32_bpc_depth world world_2d" },
	{ "CanvasLayer", "custom_viewport follow_viewport_enable follow_viewport_scale layer offset rotation rotation_degrees scale transform visible" },
	{ "ParallaxBackground", "layer scroll_base_offset scroll_base_scale scroll_ignore_camera_zoom scroll_limit_begin scroll_limit_end scroll_offset" },
	{ "ParallaxLayer", "motion_mirroring motion_offset motion_scale physics_interpolation_mode" },
	{ "Popup", "popup_exclusive visible" },
	{ "WindowDialog", "resizable window_title" },
	{ "AcceptDialog", "dialog_autowrap dialog_hide_on_ok dialog_text window_title" },
	{ "PopupMenu", "allow_search focus_mode hide_on_checkable_item_selection hide_on_item_selection hide_on_state_item_selection max_height submenu_popup_delay" },
	{ "OptionButton", "action_mode align selected toggle_mode" },
	{ "ItemList", "allow_reselect allow_rmb_select allow_search auto_height fixed_column_width fixed_icon_size focus_mode icon_mode icon_scale max_columns max_text_lines rect_clip_content same_column_width select_mode" },
	{ "Tree", "allow_reselect allow_rmb_select allow_search column_titles_visible columns drop_mode_flags focus_mode hide_folding hide_root rect_clip_content select_mode" },
	{ "ScrollContainer", "follow_focus rect_clip_content scroll_deadzone scroll_horizontal scroll_horizontal_enabled scroll_vertical scroll_vertical_enabled" },
	{ "BoxContainer", "alignment mouse_filter" },
	{ "GridContainer", "columns mouse_filter" },
	{ "TabContainer", "all_tabs_in_front current_tab drag_to_rearrange_enabled tab_align tabs_visible use_hidden_tabs_for_min_size" },
	{ "ColorRect", "color" },
	{ "VisibilityNotifier2D", "rect" },
	{ "RayCast2D", "cast_to collide_with_areas collide_with_bodies collision_mask enabled exclude_parent" },
	{ "CPUParticles2D", "amount angle angle_curve angle_random angular_velocity angular_velocity_curve angular_velocity_random anim_offset anim_offset_curve anim_offset_random anim_speed anim_speed_curve anim_speed_random color color_initial_ramp color_ramp damping damping_curve damping_random direction draw_order emission_colors emission_normals emission_points emission_rect_extents emission_shape emission_sphere_radius emitting explosiveness fixed_fps flag_align_y fract_delta gravity hue_variation hue_variation_curve hue_variation_random initial_velocity initial_velocity_random lifetime lifetime_randomness linear_accel linear_accel_curve linear_accel_random local_coords normalmap one_shot orbit_velocity orbit_velocity_curve orbit_velocity_random physics_interpolation_mode preprocess radial_accel radial_accel_curve radial_accel_random randomness scale_amount scale_amount_curve scale_amount_random speed_scale spread tangential_accel tangential_accel_curve tangential_accel_random texture" },
	{ "Camera", "affect_lod cull_mask current doppler_tracking environment far fov frustum_offset h_offset keep_aspect near projection size v_offset" },
	{ "MeshInstance", "mesh skeleton skin software_skinning_transform_normals" },
	{ "KinematicBody", "axis_lock_motion_x axis_lock_motion_y axis_lock_motion_z move_lock_x move_lock_y move_lock_z moving_platform_apply_velocity_on_leave" },
	{ "RigidBody", "angular_damp angular_velocity axis_lock_angular_x axis_lock_angular_y axis_lock_angular_z axis_lock_linear_x axis_lock_linear_y axis_lock_linear_z bounce can_sleep contact_monitor contacts_reported continuous_cd custom_integrator friction gravity_scale linear_damp linear_velocity mass mode physics_material_override sleeping weight" },
	{ "Resource", "resource_local_to_scene resource_name resource_path" },
	{ "HTTPRequest", "body_size_limit download_chunk_size download_file max_redirects timeout use_threads" },
	{ "CollisionShape2D", "disabled one_way_collision one_way_collision_margin shape" },
	{ "CollisionPolygon2D", "build_mode disabled one_way_collision one_way_collision_margin polygon" },
	{ "Position2D", "" },
	{ "YSort", "sort_enabled" },
	{ "RemoteTransform2D", "remote_path update_position update_rotation update_scale use_global_coordinates" },
	{ "CanvasModulate", "color" },
	{ NULL, NULL }
};

// Classes of Godot 3.6, a class_name can't use their names
static const char *godot3_classes =
		"AABB AESContext ARVRAnchor ARVRCamera ARVRController ARVRInterface ARVRInterfaceGDNative ARVROrigin "
		"ARVRPositionalTracker ARVRServer AStar AStar2D AcceptDialog AnimatedSprite AnimatedSprite3D AnimatedTexture "
		"Animation AnimationNode AnimationNodeAdd2 AnimationNodeAdd3 AnimationNodeAnimation AnimationNodeBlend2 "
		"AnimationNodeBlend3 AnimationNodeBlendSpace1D AnimationNodeBlendSpace2D AnimationNodeBlendTree "
		"AnimationNodeOneShot AnimationNodeOutput AnimationNodeStateMachine AnimationNodeStateMachinePlayback "
		"AnimationNodeStateMachineTransition AnimationNodeTimeScale AnimationNodeTimeSeek AnimationNodeTransition "
		"AnimationPlayer AnimationRootNode AnimationTrackEditPlugin AnimationTree AnimationTreePlayer Area Area2D "
		"Array ArrayMesh AspectRatioContainer AtlasTexture AudioBusLayout AudioEffect AudioEffectAmplify "
		"AudioEffectBandLimitFilter AudioEffectBandPassFilter AudioEffectCapture AudioEffectChorus "
		"AudioEffectCompressor AudioEffectDelay AudioEffectDistortion AudioEffectEQ AudioEffectEQ10 AudioEffectEQ21 "
		"AudioEffectEQ6 AudioEffectFilter AudioEffectHighPassFilter AudioEffectHighShelfFilter AudioEffectInstance "
		"AudioEffectLimiter AudioEffectLowPassFilter AudioEffectLowShelfFilter AudioEffectNotchFilter "
		"AudioEffectPanner AudioEffectPhaser AudioEffectPitchShift AudioEffectRecord AudioEffectReverb "
		"AudioEffectSpectrumAnalyzer AudioEffectSpectrumAnalyzerInstance AudioEffectStereoEnhance AudioServer "
		"AudioStream AudioStreamGenerator AudioStreamGeneratorPlayback AudioStreamMP3 AudioStreamMicrophone "
		"AudioStreamOGGVorbis AudioStreamPlayback AudioStreamPlaybackResampled AudioStreamPlayer AudioStreamPlayer2D "
		"AudioStreamPlayer3D AudioStreamRandomPitch AudioStreamSample BackBufferCopy BakedLightmap BakedLightmapData "
		"BaseButton Basis BitMap BitmapFont Bone2D BoneAttachment BoxContainer BoxShape Button ButtonGroup "
		"CPUParticles CPUParticles2D CSGBox CSGCombiner CSGCylinder CSGMesh CSGPolygon CSGPrimitive CSGShape "
		"CSGSphere CSGTorus CSharpScript CallbackTweener Camera Camera2D CameraFeed CameraServer CameraTexture "
		"CanvasItem CanvasItemMaterial CanvasLayer CanvasModulate CapsuleMesh CapsuleShape CapsuleShape2D "
		"CenterContainer CharFXTransform CheckBox CheckButton CircleShape2D ClassDB ClippedCamera CollisionObject "
		"CollisionObject2D CollisionPolygon CollisionPolygon2D CollisionShape CollisionShape2D Color ColorPicker "
		"ColorPickerButton ColorRect ConcavePolygonShape ConcavePolygonShape2D ConeTwistJoint ConfigFile "
		"ConfirmationDialog Container Control ConvexPolygonShape ConvexPolygonShape2D Crypto CryptoKey CubeMap "
		"CubeMesh CullInstance Curve Curve2D Curve3D CurveTexture CylinderMesh CylinderShape DTLSServer "
		"DampedSpringJoint2D Dictionary DirectionalLight Directory DynamicFont DynamicFontData EditorExportPlugin "
		"EditorFeatureProfile EditorFileDialog EditorFileSystem EditorFileSystemDirectory EditorImportPlugin "
		"EditorInspector EditorInspectorPlugin EditorInterface EditorPlugin EditorProperty "
		"EditorResourceConversionPlugin EditorResourcePicker EditorResourcePreview EditorResourcePreviewGenerator "
		"EditorSceneImporter EditorSceneImporterFBX EditorSceneImporterGLTF EditorScenePostImport EditorScript "
		"EditorScriptPicker EditorSelection EditorSettings EditorSpatialGizmo EditorSpatialGizmoPlugin "
		"EditorSpinSlider EditorVCSInterface EncodedObjectAsID Engine Environment Expression ExternalTexture File "
		"FileDialog FileSystemDock FlowContainer Font FuncRef GDNative GDNativeLibrary GDScript GDScriptFunctionState "
		"GIProbe GIProbeData GLTFAccessor GLTFAnimation GLTFBufferView GLTFCamera GLTFCollider GLTFDocument "
		"GLTFDocumentExtension GLTFLight GLTFMesh GLTFNode GLTFPhysicsBody GLTFSkeleton GLTFSkin GLTFSpecGloss "
		"GLTFState GLTFTexture GLTFTextureSampler Generic6DOFJoint Geometry GeometryInstance GodotSharp Gradient "
		"GradientTexture GradientTexture2D GraphEdit GraphNode GridContainer GridMap GrooveJoint2D HBoxContainer "
		"HFlowContainer HMACContext HScrollBar HSeparator HSlider HSplitContainer HTTPClient HTTPRequest "
		"HashingContext HeightMapShape HingeJoint IP Image ImageTexture ImmediateGeometry Input InputEvent "
		"InputEventAction InputEventGesture InputEventJoypadButton InputEventJoypadMotion InputEventKey "
		"InputEventMIDI InputEventMagnifyGesture InputEventMouse InputEventMouseButton InputEventMouseMotion "
		"InputEventPanGesture InputEventScreenDrag InputEventScreenTouch InputEventWithModifiers InputMap "
		"InstancePlaceholder InterpolatedCamera IntervalTweener ItemList JNISingleton JSON JSONParseResult JSONRPC "
		"JavaClass JavaClassWrapper JavaScript JavaScriptObject Joint Joint2D KinematicBody KinematicBody2D "
		"KinematicCollision KinematicCollision2D LOD Label Label3D LargeTexture Light Light2D LightOccluder2D Line2D "
		"LineEdit LineShape2D LinkButton Listener Listener2D MainLoop MarginContainer Marshalls Material Material3D "
		"MenuButton MergeGroup Mesh MeshDataTool MeshInstance MeshInstance2D MeshLibrary MeshTexture MethodTweener "
		"MobileVRInterface MultiMesh MultiMeshInstance MultiMeshInstance2D MultiplayerAPI MultiplayerPeerGDNative "
		"Mutex NativeScript Navigation Navigation2D Navigation2DServer NavigationAgent NavigationAgent2D "
		"NavigationMesh NavigationMeshGenerator NavigationMeshInstance NavigationObstacle NavigationObstacle2D "
		"NavigationPolygon NavigationPolygonInstance NavigationServer NetworkedMultiplayerCustom "
		"NetworkedMultiplayerENet NetworkedMultiplayerPeer NinePatchRect Node Node2D NodePath NoiseTexture "
		"ORMSpatialMaterial OS Object Occluder OccluderPolygon2D OccluderShape OccluderShapePolygon "
		"OccluderShapeSphere OmniLight OpenSimplexNoise OptionButton PCKPacker PHashTranslation PackedDataContainer "
		"PackedDataContainerRef PackedScene PackedSceneGLTF PacketPeer PacketPeerDTLS PacketPeerGDNative "
		"PacketPeerStream PacketPeerUDP Panel PanelContainer PanoramaSky ParallaxBackground ParallaxLayer Particles "
		"Particles2D ParticlesMaterial Path Path2D PathFollow PathFollow2D Performance PhysicalBone "
		"Physics2DDirectBodyState Physics2DDirectSpaceState Physics2DServer Physics2DShapeQueryParameters "
		"Physics2DTestMotionResult PhysicsBody PhysicsBody2D PhysicsDirectBodyState PhysicsDirectSpaceState "
		"PhysicsMaterial PhysicsServer PhysicsShapeQueryParameters PhysicsTestMotionResult PinJoint PinJoint2D Plane "
		"PlaneMesh PlaneShape PluginScript PointMesh Polygon2D PolygonPathFinder PoolByteArray PoolColorArray "
		"PoolIntArray PoolRealArray PoolStringArray PoolVector2Array PoolVector3Array Popup PopupDialog PopupMenu "
		"PopupPanel Portal Position2D Position3D PrimitiveMesh PrismMesh ProceduralSky ProgressBar ProjectSettings "
		"PropertyTweener ProximityGroup ProxyTexture QuadMesh Quat RID RandomNumberGenerator Range RayCast RayCast2D "
		"RayShape RayShape2D Rect2 RectangleShape2D Reference ReferenceRect ReflectionProbe RegEx RegExMatch "
		"RemoteTransform RemoteTransform2D Resource ResourceFormatLoader ResourceFormatSaver ResourceImporter "
		"ResourceInteractiveLoader ResourceLoader ResourcePreloader ResourceSaver RichTextEffect RichTextLabel "
		"RigidBody RigidBody2D Room RoomGroup RoomManager RootMotionView SceneState SceneTree SceneTreeTimer "
		"SceneTreeTween Script ScriptCreateDialog ScriptEditor ScrollBar ScrollContainer SegmentShape2D Semaphore "
		"Separator Shader ShaderMaterial Shape Shape2D ShapeCast ShapeCast2D ShortCut Skeleton Skeleton2D SkeletonIK "
		"Skin SkinReference Sky Slider SliderJoint SoftBody Spatial SpatialGizmo SpatialMaterial "
		"SpatialVelocityTracker SphereMesh SphereShape SpinBox SplitContainer SpotLight SpringArm Sprite Sprite3D "
		"SpriteBase3D SpriteFrames StaticBody StaticBody2D StreamPeer StreamPeerBuffer StreamPeerGDNative "
		"StreamPeerSSL StreamPeerTCP StreamTexture String StyleBox StyleBoxEmpty StyleBoxFlat StyleBoxLine "
		"StyleBoxTexture SurfaceTool TCP_Server TabContainer Tabs TextEdit TextFile TextMesh Texture Texture3D "
		"TextureArray TextureButton TextureLayered TextureProgress TextureRect Theme Thread TileMap TileSet Time "
		"Timer ToolButton TorusMesh TouchScreenButton Transform Transform2D Translation TranslationServer Tree "
		"TreeItem TriangleMesh Tween Tweener UDPServer UPNP UPNPDevice UndoRedo VBoxContainer VFlowContainer "
		"VScrollBar VSeparator VSlider VSplitContainer Variant Vector2 Vector3 VehicleBody VehicleWheel VideoPlayer "
		"VideoStream VideoStreamGDNative VideoStreamTheora VideoStreamWebm Viewport ViewportContainer ViewportTexture "
		"VisibilityEnabler VisibilityEnabler2D VisibilityNotifier VisibilityNotifier2D VisualInstance VisualScript "
		"VisualScriptBasicTypeConstant VisualScriptBuiltinFunc VisualScriptClassConstant VisualScriptComment "
		"VisualScriptComposeArray VisualScriptCondition VisualScriptConstant VisualScriptConstructor "
		"VisualScriptCustomNode VisualScriptDeconstruct VisualScriptEditor VisualScriptEmitSignal "
		"VisualScriptEngineSingleton VisualScriptExpression VisualScriptFunction VisualScriptFunctionCall "
		"VisualScriptFunctionState VisualScriptGlobalConstant VisualScriptIndexGet VisualScriptIndexSet "
		"VisualScriptInputAction VisualScriptIterator VisualScriptLists VisualScriptLocalVar VisualScriptLocalVarSet "
		"VisualScriptMathConstant VisualScriptNode VisualScriptOperator VisualScriptPreload VisualScriptPropertyGet "
		"VisualScriptPropertySet VisualScriptResourcePath VisualScriptReturn VisualScriptSceneNode "
		"VisualScriptSceneTree VisualScriptSelect VisualScriptSelf VisualScriptSequence VisualScriptSubCall "
		"VisualScriptSwitch VisualScriptTypeCast VisualScriptVariableGet VisualScriptVariableSet VisualScriptWhile "
		"VisualScriptYield VisualScriptYieldSignal VisualServer VisualShader VisualShaderNode "
		"VisualShaderNodeBooleanConstant VisualShaderNodeBooleanUniform VisualShaderNodeColorConstant "
		"VisualShaderNodeColorFunc VisualShaderNodeColorOp VisualShaderNodeColorUniform VisualShaderNodeCompare "
		"VisualShaderNodeCubeMap VisualShaderNodeCubeMapUniform VisualShaderNodeCustom VisualShaderNodeDeterminant "
		"VisualShaderNodeDotProduct VisualShaderNodeExpression VisualShaderNodeFaceForward VisualShaderNodeFresnel "
		"VisualShaderNodeGlobalExpression VisualShaderNodeGroupBase VisualShaderNodeIf VisualShaderNodeInput "
		"VisualShaderNodeIs VisualShaderNodeOuterProduct VisualShaderNodeOutput VisualShaderNodeScalarClamp "
		"VisualShaderNodeScalarConstant VisualShaderNodeScalarDerivativeFunc VisualShaderNodeScalarFunc "
		"VisualShaderNodeScalarInterp VisualShaderNodeScalarOp VisualShaderNodeScalarSmoothStep "
		"VisualShaderNodeScalarSwitch VisualShaderNodeScalarUniform VisualShaderNodeSwitch VisualShaderNodeTexture "
		"VisualShaderNodeTextureUniform VisualShaderNodeTextureUniformTriplanar VisualShaderNodeTransformCompose "
		"VisualShaderNodeTransformConstant VisualShaderNodeTransformDecompose VisualShaderNodeTransformFunc "
		"VisualShaderNodeTransformMult VisualShaderNodeTransformUniform VisualShaderNodeTransformVecMult "
		"VisualShaderNodeUniform VisualShaderNodeUniformRef VisualShaderNodeVec3Constant VisualShaderNodeVec3Uniform "
		"VisualShaderNodeVectorClamp VisualShaderNodeVectorCompose VisualShaderNodeVectorDecompose "
		"VisualShaderNodeVectorDerivativeFunc VisualShaderNodeVectorDistance VisualShaderNodeVectorFunc "
		"VisualShaderNodeVectorInterp VisualShaderNodeVectorLen VisualShaderNodeVectorOp "
		"VisualShaderNodeVectorRefract VisualShaderNodeVectorScalarMix VisualShaderNodeVectorScalarSmoothStep "
		"VisualShaderNodeVectorScalarStep VisualShaderNodeVectorSmoothStep WeakRef WebRTCDataChannel "
		"WebRTCDataChannelGDNative WebRTCMultiplayer WebRTCPeerConnection WebRTCPeerConnectionGDNative "
		"WebSocketClient WebSocketMultiplayerPeer WebSocketPeer WebSocketServer WebXRInterface WindowDialog World "
		"World2D WorldEnvironment X509Certificate XMLParser YSort bool float int ";

static String _table_lookup(const char *p_table[][2], const String &p_key) {

	for (int i = 0; p_table[i][0]; i++) {
		if (p_key == p_table[i][0]) {
			return p_table[i][1];
		}
	}
	return String();
}

String EditorExportGodot3::_get_script_base_type(const String &p_path, int p_depth) {

	// The native type a script extends, following inheritance through other scripts
	if (p_depth > 16) {
		return String();
	}

	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		return String();
	}

	Vector<String> lines;
	while (!f->eof_reached()) {
		lines.push_back(f->get_line());
	}

	return _get_script_base_type_from_lines(p_path, lines, p_depth);
}

String EditorExportGodot3::_get_script_base_type_from_lines(const String &p_path, const Vector<String> &p_lines, int p_depth) {

	for (int i = 0; i < p_lines.size(); i++) {
		String line = p_lines[i].strip_edges();
		if (line.begins_with("tool")) {
			line = line.substr(4, line.length()).strip_edges();
		}
		if (!line.begins_with("extends ") && !line.begins_with("extends\t")) {
			continue;
		}

		String base = line.substr(7, line.length()).strip_edges();
		int comment = base.find("#");
		if (comment != -1) {
			base = base.substr(0, comment).strip_edges();
		}

		if (base.begins_with("\"") || base.begins_with("'")) {
			String path = base.substr(1, base.length()).get_slice(base.substr(0, 1), 0);
			if (!path.begins_with("res://")) {
				path = p_path.get_base_dir().plus_file(path);
			}
			return _get_script_base_type(path, p_depth + 1);
		}

		return base;
	}

	return "Reference";
}

String EditorExportGodot3::_convert_script_code(const String &p_code, Vector<String> &r_strings, ScriptContext &p_ctx) {

	String code = p_code;

	/* var.type == InputEvent.KEY => var is InputEventKey */
	{
		int pos = 0;
		while ((pos = code.find("InputEvent.", pos)) != -1) {

			int name_start = pos + 11;
			int name_end = name_start;
			while (name_end < code.length() && _is_ident_char(code[name_end])) {
				name_end++;
			}
			String new_type = _table_lookup(script_input_event_types, code.substr(name_start, name_end - name_start));

			String before = code.substr(0, pos).strip_edges(false, true);
			bool negated = before.ends_with("!=");
			if (new_type == String() || (pos > 0 && _is_ident_char(code[pos - 1])) || !(before.ends_with("==") || negated)) {
				pos = name_end;
				continue;
			}

			before = before.substr(0, before.length() - 2).strip_edges(false, true);
			if (!before.ends_with(".type")) {
				pos = name_end;
				continue;
			}

			int recv_end = before.length() - 5;
			int recv_start = _get_receiver_start(before, recv_end);
			String receiver = before.substr(recv_start, recv_end - recv_start);
			String replacement = (negated ? "not " : "") + receiver + " is " + new_type;

			code = before.substr(0, recv_start) + replacement + code.substr(name_end, code.length() - name_end);
			pos = recv_start + replacement.length();
		}
	}

	/* Calls, identifiers and members, in a single pass so a conversion is never applied twice */

	String out;
	int i = 0;
	int len = code.length();

	while (i < len) {

		CharType c = code[i];
		bool word_start = (_is_ident_char(c) && !(c >= '0' && c <= '9')) && (i == 0 || !_is_ident_char(code[i - 1]));

		if (!word_start) {
			out += c;
			i++;
			continue;
		}

		int word_end = i;
		while (word_end < len && _is_ident_char(code[word_end])) {
			word_end++;
		}
		String word = code.substr(i, word_end - i);

		bool after_dot = out.length() > 0 && out[out.length() - 1] == '.';

		int paren = word_end;
		while (paren < len && (code[paren] == ' ' || code[paren] == '\t')) {
			paren++;
		}
		bool is_call = paren < len && code[paren] == '(';

		if (!is_call) {

			String renamed;
			if (!after_dot && p_ctx.local_renames.has(word)) {
				renamed = p_ctx.local_renames[word];
			} else if (after_dot && out.ends_with("Image.") && _table_lookup(script_image_formats, word) != String()) {
				renamed = _table_lookup(script_image_formats, word);
			} else if (after_dot) {
				renamed = _table_lookup(script_dot_member_renames, word);
				if (renamed != String() && user_members.has(word)) {
					// Could be a member of a project script
					p_ctx.needs_review = true;
				}
			} else if (type_rename_map.has(word)) {
				renamed = type_rename_map[word];
			}

			if (renamed == String()) {
				renamed = _table_lookup(script_identifier_renames, word);
				if (renamed != String() && after_dot && word.length() <= 4) {
					// Short singleton names are only renamed when used directly
					renamed = String();
				}
			}

			// Operator "extends" was renamed to "is", but not when defining a class
			if (word == "extends" && out.strip_edges() != "" && !out.strip_edges().begins_with("class ") && out.strip_edges() != "tool") {
				renamed = "is";
			}

			out += (renamed != String()) ? renamed : word;
			i = word_end;
			continue;
		}

		int close = _find_closing(code, paren);
		if (close == -1) {
			out += word;
			i = word_end;
			continue;
		}

		// Receiver of the call, already converted in "out"
		String receiver;
		int recv_start = out.length();
		bool apply_rules = true;
		if (after_dot) {
			recv_start = _get_receiver_start(out, out.length() - 1);
			receiver = out.substr(recv_start, out.length() - 1 - recv_start);
			if (receiver == "") {
				// Unusual formatting, keep the call as is
				recv_start = out.length();
				apply_rules = false;
			}
		}
		String before = out.substr(0, recv_start);
		bool statement = before.strip_edges() == "" && code.substr(close + 1, len - close - 1).strip_edges() == "";
		bool declaration = before.strip_edges().ends_with("func");

		String args_str = _convert_script_code(code.substr(paren + 1, close - paren - 1), r_strings, p_ctx);
		Vector<String> args = _split_args(args_str);

		String replacement;
		bool handled = false;
		if (declaration) {
			// Only the callbacks are renamed in declarations
			if (word == "_fixed_process" || (word == "_input_event" && p_ctx.is_control)) {
				handled = _convert_script_call(word, receiver, args, statement, before, r_strings, p_ctx, replacement);
			}
		} else if (apply_rules) {
			handled = _convert_script_call(word, receiver, args, statement, before, r_strings, p_ctx, replacement);
		}

		if (!handled) {
			// Constructors of renamed types and other calls, keeping their formatting
			String name = word;
			if (!after_dot && type_rename_map.has(word)) {
				name = type_rename_map[word];
			}
			String spacing = code.substr(word_end, paren - word_end);
			replacement = (receiver != "" ? receiver + "." : String()) + name + spacing + "(" + args_str + ")";
		}

		out = before + replacement;
		i = close + 1;
	}

	return out;
}

bool EditorExportGodot3::_convert_script_call(const String &p_name, const String &p_receiver, const Vector<String> &p_args, bool p_statement, const String &p_before, Vector<String> &r_strings, ScriptContext &p_ctx, String &r_replacement) {

	String name = p_name;
	String receiver = p_receiver;
	String args = _join_args(p_args);
	int argc = p_args.size();

	// Callbacks, renamed even when declared by the script
	if (name == "_fixed_process") {
		r_replacement = _with_receiver(receiver, "_physics_process") + "(" + args + ")";
		return true;
	}
	if (name == "_input_event" && p_ctx.is_control) {
		r_replacement = _with_receiver(receiver, "_gui_input") + "(" + args + ")";
		return true;
	}

	// Renames given in the export options, they win over everything else
	String full_name = receiver == "" ? name : receiver + "." + name;
	if (function_renames.has(full_name)) {
		r_replacement = function_renames[full_name] + "(" + args + ")";
		return true;
	}

	// Functions declared by the project scripts are not renamed: calls on self when the script
	// (or a parent script) declares the function, calls on other objects when it's not a method
	// of the engine.
	if (receiver == "" || receiver == "self") {
		if (p_ctx.declared_functions.has(name)) {
			return false;
		}
	} else if (user_functions.has(name) && !engine_methods.has(name)) {
		return false;
	}

	// tr() is not a static function, it can't be called from one in Godot 3
	if (name == "tr" && receiver == "" && p_ctx.in_static) {
		r_replacement = "TranslationServer.translate(" + args + ")";
		return true;
	}

	/* RegEx: find() and the captures became search() returning a RegExMatch, kept in a member */

	if (p_ctx.regex_vars.has(receiver)) {

		String match = receiver.is_valid_identifier() ? "_" + receiver + "_match" : String("_regex_match");

		// The match is a member, or a local variable when the RegEx is local to the function
		bool local = !p_ctx.member_vars.has(receiver) || p_ctx.in_static;
		String match_decl = match;
		if (local && !p_ctx.function_locals.has(match)) {
			match_decl = "var " + match;
		}

		if (name == "compile" && argc >= 1) {
			r_replacement = receiver + ".compile(" + p_args[0] + ")";
			return true;
		}

		if (name == "find" && argc >= 1) {
			String search = receiver + ".search(" + args + ")";
			if (p_statement || p_ctx.prefix_allowed) {
				if (local) {
					p_ctx.function_locals.insert(match);
				} else {
					p_ctx.extra_members.insert(match);
				}
			}
			if (p_statement) {
				r_replacement = match_decl + " = " + search;
			} else if (p_ctx.prefix_allowed) {
				p_ctx.prefix_statements.push_back(match_decl + " = " + search);
				r_replacement = "(" + match + ".get_start() if " + match + " else -1)";
			} else {
				// No statement can be added before "elif" or "while", the captures are not kept
				r_replacement = "(" + search + ".get_start() if " + search + " else -1)";
				p_ctx.needs_review = true;
			}
			return true;
		}

		if (name == "get_capture" && argc == 1) {
			if (!local) {
				p_ctx.extra_members.insert(match);
			}
			r_replacement = match + ".get_string(" + args + ")";
			return true;
		}

		if (name == "get_capture_start" && argc == 1) {
			if (!local) {
				p_ctx.extra_members.insert(match);
			}
			r_replacement = match + ".get_start(" + args + ")";
			return true;
		}

		if (name == "get_captures" && argc == 0) {
			if (!local) {
				p_ctx.extra_members.insert(match);
			}
			r_replacement = match + ".strings";
			return true;
		}

		if (name == "get_capture_count" && argc == 0) {
			if (!local) {
				p_ctx.extra_members.insert(match);
			}
			// The group 0 (whole match) was counted
			r_replacement = "(" + match + ".get_group_count() + 1 if " + match + " else 0)";
			return true;
		}
	}

	/* Image: it's a Resource, created with Image.new() and create(), locked to access its pixels */

	if (name == "Image" && receiver == "") {
		String before = p_before.strip_edges();
		String var = before.ends_with("=") ? before.substr(0, before.length() - 1).replace("var ", "").strip_edges() : String();
		if (argc == 0) {
			r_replacement = "Image.new()";
			return true;
		}
		if (var.is_valid_identifier() && !before.ends_with("==") && p_ctx.postfix_allowed) {
			r_replacement = "Image.new()";
			p_ctx.postfix_statements.push_back(var + ".create(" + args + ")");
			p_ctx.postfix_statements.push_back(var + ".lock()");
			p_ctx.needs_review = true; // unlock() is needed before using the image as a texture
			return true;
		}
		p_ctx.needs_review = true;
		return false;
	}

	if (p_ctx.image_vars.has(receiver)) {

		if (name == "put_pixel" && argc >= 3) {
			r_replacement = receiver + ".set_pixel(" + p_args[0] + ", " + p_args[1] + ", " + p_args[2] + ")";
			return true;
		}

		if (name == "get_pixel" && argc >= 2) {
			r_replacement = receiver + ".get_pixel(" + p_args[0] + ", " + p_args[1] + ")";
			return true;
		}

		if (name == "empty" && argc == 0) {
			r_replacement = receiver + ".is_empty()";
			return true;
		}

		if (name == "load" && argc == 1 && p_statement) {
			r_replacement = receiver + ".load(" + args + ")";
			p_ctx.postfix_statements.push_back(receiver + ".lock()");
			return true;
		}

		if (name == "resized" || name == "converted" || name == "compressed" || name == "decompressed") {
			// Those return a modified copy in 2.1, they modify the image in Godot 3
			p_ctx.needs_review = true;
			return false;
		}
	}

	// Signals given as strings
	if ((name == "connect" || name == "disconnect" || name == "is_connected" || name == "emit_signal" || name == "yield") && argc > 0) {

		if (name == "connect" && p_ctx.http_vars.has(receiver)) {
			r_replacement = _with_receiver(receiver, "connect_to_host") + "(" + args + ")";
			return true;
		}

		int sig_arg = (name == "yield") ? 1 : 0;
		if (sig_arg < argc && p_args[sig_arg].length() == 1 && _is_placeholder(p_args[sig_arg][0])) {
			int idx = p_args[sig_arg][0] - STRING_PLACEHOLDER_BASE;
			String str = r_strings[idx];
			String quote = str.substr(0, 1);
			String signal = str.substr(1, str.length() - 2);
			String new_signal;
			if (signal_rename_map.has(signal)) {
				new_signal = signal_rename_map[signal];
			} else if (signal == "input_event" && receiver == "" && name != "yield" && p_ctx.is_control) {
				new_signal = "gui_input";
			}
			if (new_signal != String()) {
				r_strings[idx] = quote + new_signal + quote;
			}
		}
		return false;
	}

	// ProjectSettings (Globals in 2.1)
	if (receiver == "ProjectSettings") {
		if (argc > 0 && p_args[0].length() == 1 && _is_placeholder(p_args[0][0])) {
			int idx = p_args[0][0] - STRING_PLACEHOLDER_BASE;
			String str = r_strings[idx];
			String key = str.substr(1, str.length() - 2);
			if (globals_rename_map.has(key)) {
				r_strings[idx] = str.substr(0, 1) + globals_rename_map[key] + str.substr(0, 1);
			}
		}
		if (name == "has") {
			r_replacement = "ProjectSettings.has_setting(" + args + ")";
			return true;
		}
		if (name == "get_singleton" || name == "has_singleton") {
			r_replacement = "Engine." + name + "(" + args + ")";
			return true;
		}
		return false;
	}

	// OS methods which moved to Engine
	if (receiver == "OS") {
		for (int i = 0; script_engine_methods[i]; i++) {
			if (name == script_engine_methods[i]) {
				r_replacement = "Engine." + name + "(" + args + ")";
				return true;
			}
		}
		if (name == "get_engine_version") {
			r_replacement = "Engine.get_version_info()";
			return true;
		}
		if (name == "get_video_mode_size") {
			r_replacement = "OS.get_window_size()";
			return true;
		}
		if (name == "is_video_mode_fullscreen") {
			r_replacement = "OS.is_window_fullscreen()";
			return true;
		}
		if (name == "is_video_mode_resizable") {
			r_replacement = "OS.is_window_resizable()";
			return true;
		}
	}

	if ((receiver == "OS" || receiver == "Input") && name == "get_mouse_pos") {
		r_replacement = "get_viewport().get_mouse_position()";
		p_ctx.needs_review = true;
		return true;
	}

	if ((name == "is_editor_hint" || name == "set_editor_hint") && receiver.ends_with("get_tree()")) {
		r_replacement = "Engine." + name + "(" + args + ")";
		return true;
	}

	if (name == "get_rect" && argc == 0 && (receiver.ends_with("get_viewport()") || receiver.ends_with("get_root()"))) {
		r_replacement = _with_receiver(receiver, "get_visible_rect") + "()";
		return true;
	}

	if ((name == "basename" || name == "extension") && argc == 0 && receiver != "") {
		r_replacement = _with_receiver(receiver, "get_" + name) + "()";
		return true;
	}

	if (name == "get_type" && argc == 0) {
		r_replacement = _with_receiver(receiver, "get_class") + "()";
		return true;
	}

	if ((name == "set_layer_mask" || name == "get_layer_mask" || name == "set_layer_mask_bit" || name == "get_layer_mask_bit")) {
		// The layer mask of RayCast is now its collision mask, the one of bodies and areas their collision layer
		String target = (p_ctx.is_raycast && receiver == "") ? "collision_mask" : "collision_layer";
		r_replacement = _with_receiver(receiver, name.replace("layer_mask", target)) + "(" + args + ")";
		return true;
	}

	/* KinematicBody: move() returned the remaining motion and the collision was queried on the body,
	   move_and_collide() returns the collision. It's stored in a member of the script. */

	// Bodies moved by another script: the collision is kept in that script, the receiver must be
	// the same expression.
	if (receiver != "" && name == "move" && argc == 1 && !p_ctx.moved_receivers.has(receiver)) {
		p_ctx.moved_receivers.insert(receiver);
	}

	if ((p_ctx.is_kinematic && receiver == "") || (receiver != "" && p_ctx.moved_receivers.has(receiver))) {

		String collision = "_kinematic_collision";
		String body = receiver == "" ? String() : receiver + ".";
		bool spatial = receiver == "" ? p_ctx.is_spatial : false;

		if (name == "move" && argc == 1) {
			p_ctx.uses_kinematic_collision = true;
			if (receiver != "") {
				p_ctx.needs_review = true;
			}
			String move = collision + " = " + body + "move_and_collide(" + args + ")";
			if (p_statement) {
				r_replacement = move;
			} else if (!p_ctx.prefix_allowed) {
				r_replacement = body + "move_and_collide(" + args + ")";
				p_ctx.needs_review = true;
			} else {
				// The collision is computed on its own line, before the one using the remaining motion
				p_ctx.prefix_statements.push_back(move);
				r_replacement = "(" + collision + ".remainder if " + collision + " else " + (spatial ? "Vector3()" : "Vector2()") + ")";
				String before = p_before.strip_edges();
				if (!before.ends_with("=") || before.begins_with("if ") || before.begins_with("while ") || before.begins_with("elif ")) {
					p_ctx.needs_review = true;
				}
			}
			return true;
		}

		if (name == "revert_motion" && argc == 0 && p_statement) {
			// Moved the body back by the travel of the last motion
			p_ctx.uses_kinematic_collision = true;
			p_ctx.needs_review = true;
			r_replacement = body + String(spatial ? "global_transform.origin" : "global_position") + " -= (" + collision + ".travel if " + collision + " else " + (spatial ? "Vector3()" : "Vector2()") + ")";
			return true;
		}

		if (name == "is_colliding" && argc == 0) {
			p_ctx.uses_kinematic_collision = true;
			r_replacement = "(" + collision + " != null)";
			return true;
		}

		static const char *collision_getters[][2] = {
			{ "get_collision_normal", "normal" },
			{ "get_collision_pos", "position" },
			{ "get_collider", "collider" },
			{ "get_collider_shape", "collider_shape_index" },
			{ "get_collider_metadata", "collider_metadata" },
			{ "get_collider_velocity", "collider_velocity" },
			{ "get_travel", "travel" },
			{ NULL, NULL }
		};

		String member = _table_lookup(collision_getters, name);
		if (member != String() && argc == 0) {
			p_ctx.uses_kinematic_collision = true;
			r_replacement = collision + "." + member;
			return true;
		}
	}

	if (name == "move" && argc == 1 && receiver != "") {
		// Returns a KinematicCollision2D instead of the remaining motion
		r_replacement = _with_receiver(receiver, "move_and_collide") + "(" + args + ")";
		p_ctx.needs_review = true;
		return true;
	}

	/* 2D rotations are now clockwise, see _rename_properties */

	// Spatial rotations did not change
	if (p_ctx.is_spatial && (name == "set_rotation" || name == "get_rotation" || name == "set_rotation_deg" || name == "get_rotation_deg")) {
		r_replacement = _with_receiver(receiver, name.replace("_deg", "_degrees")) + "(" + args + ")";
		return true;
	}

	String setter = _table_lookup(script_rotation_setters, name);
	if (setter != String() && argc == 1) {
		r_replacement = _with_receiver(receiver, setter) + "(" + _negate_expression(p_args[0]) + ")";
		return true;
	}

	String getter = _table_lookup(script_rotation_getters, name);
	if (getter != String() && argc == 0) {
		r_replacement = _wrap_unary(p_before, "-" + _with_receiver(receiver, getter) + "()");
		return true;
	}

	if ((name == "rotate" || name == "rotated") && argc == 1) {
		r_replacement = _with_receiver(receiver, name) + "(" + _negate_expression(p_args[0]) + ")";
		return true;
	}

	if (name == "Matrix32" && receiver == "" && argc == 2) {
		r_replacement = "Transform2D(" + _negate_expression(p_args[0]) + ", " + p_args[1] + ")";
		return true;
	}

	if (receiver != "" && name == "angle" && argc == 0) {
		// Vector2.angle() was atan2(x, y), it's now atan2(y, x)
		r_replacement = "(PI / 2 - " + receiver + ".angle())";
		return true;
	}

	if (receiver != "" && name == "angle_to_point" && argc == 1) {
		r_replacement = "(PI / 2 - " + receiver + ".angle_to_point(" + args + "))";
		return true;
	}

	if (receiver != "" && name == "angle_to" && argc == 1) {
		r_replacement = _wrap_unary(p_before, "-" + receiver + ".angle_to(" + args + ")");
		return true;
	}

	// The color of ColorPicker[Button] is now its "pick color", get_color is the theme getter
	if (p_ctx.is_color_picker && ((name == "get_color" && argc == 0) || (name == "set_color" && argc == 1))) {
		r_replacement = _with_receiver(receiver, name.replace("_color", "_pick_color")) + "(" + args + ")";
		return true;
	}

	// Shapes are now handled by shape owners
	if (name == "clear_shapes" && argc == 0 && p_statement) {
		r_replacement = "for owner_id in " + _with_receiver(receiver, "get_shape_owners") + "():\n" + p_ctx.line_indent + p_ctx.indent_unit + _with_receiver(receiver, "remove_shape_owner") + "(owner_id)";
		return true;
	}

	/* Visibility, opacity and mouse filter */

	if (name == "set_hidden" && argc == 1) {
		r_replacement = _with_receiver(receiver, "set_visible") + "(" + _not_expression(p_args[0]) + ")";
		return true;
	}

	if (name == "is_hidden" && argc == 0) {
		r_replacement = _wrap_unary(p_before, "not " + _with_receiver(receiver, "is_visible") + "()");
		return true;
	}

	if (name == "is_visible" && argc == 0) {
		// 2.1 checked the parents too
		r_replacement = _with_receiver(receiver, "is_visible_in_tree") + "()";
		return true;
	}

	if ((name == "get_opacity" || name == "get_self_opacity") && argc == 0) {
		r_replacement = _with_receiver(receiver, name == "get_opacity" ? "modulate.a" : "self_modulate.a");
		return true;
	}

	if ((name == "set_opacity" || name == "set_self_opacity") && argc == 1) {
		String member = (name == "set_opacity") ? "modulate" : "self_modulate";
		if (p_statement) {
			r_replacement = _with_receiver(receiver, member + ".a") + " = " + p_args[0];
			return true;
		}
		p_ctx.needs_review = true;
		return false;
	}

	if (name == "set_ignore_mouse" && argc == 1) {
		r_replacement = _with_receiver(receiver, "set_mouse_filter") + "(" + _choose(p_args[0], "Control.MOUSE_FILTER_IGNORE", "Control.MOUSE_FILTER_STOP") + ")";
		return true;
	}

	if (name == "set_stop_mouse" && argc == 1) {
		r_replacement = _with_receiver(receiver, "set_mouse_filter") + "(" + _choose(p_args[0], "Control.MOUSE_FILTER_STOP", "Control.MOUSE_FILTER_PASS") + ")";
		return true;
	}

	if (name == "is_ignoring_mouse" && argc == 0) {
		r_replacement = "(" + _with_receiver(receiver, "get_mouse_filter") + "() == Control.MOUSE_FILTER_IGNORE)";
		return true;
	}

	if (name == "is_stopping_mouse" && argc == 0) {
		r_replacement = "(" + _with_receiver(receiver, "get_mouse_filter") + "() == Control.MOUSE_FILTER_STOP)";
		return true;
	}

	if (name == "set_click_on_press" && argc == 1) {
		r_replacement = _with_receiver(receiver, "set_action_mode") + "(" + _choose(p_args[0], "BaseButton.ACTION_MODE_BUTTON_PRESS", "BaseButton.ACTION_MODE_BUTTON_RELEASE") + ")";
		return true;
	}

	if (name == "get_click_on_press" && argc == 0) {
		r_replacement = "(" + _with_receiver(receiver, "get_action_mode") + "() == BaseButton.ACTION_MODE_BUTTON_PRESS)";
		return true;
	}

	if (name == "set_area_as_parent_rect") {
		String extra = argc ? ", Control.PRESET_MODE_MINSIZE, " + p_args[0] : "";
		r_replacement = _with_receiver(receiver, "set_anchors_and_margins_preset") + "(Control.PRESET_WIDE" + extra + ")";
		return true;
	}

	/* Groups: the flags became optional */

	if ((name == "call_group" && argc >= 3) || (name == "set_group" && argc >= 4) || (name == "notify_group" && argc >= 3)) {
		String flags = p_args[0];
		if (flags == "0" || flags == "SceneTree.GROUP_CALL_DEFAULT" || flags == "GROUP_CALL_DEFAULT") {
			r_replacement = _with_receiver(receiver, name) + "(" + _join_args(p_args, 1) + ")";
		} else {
			r_replacement = _with_receiver(receiver, name + "_flags") + "(" + args + ")";
		}
		return true;
	}

	if (name == "preload" && receiver == "" && argc == 1 && p_args[0].length() == 1 && _is_placeholder(p_args[0][0])) {
		String str = r_strings[p_args[0][0] - STRING_PLACEHOLDER_BASE];
		String target = str.substr(1, str.length() - 2);
		if (!target.begins_with("res://")) {
			target = p_ctx.path.get_base_dir().plus_file(target);
		}
		if (cyclic_preloads.has(p_ctx.path + "|" + target)) {
			r_replacement = "load(" + args + ")";
			p_ctx.cyclic_preload = true;
			return true;
		}
		return false;
	}

	/* Functions which only exist in this fork */

	if (name == "randi_range" && receiver == "" && argc == 2) {
		// randi_range(from, to) excludes "to", its arguments were converted to integers
		String from = p_args[0];
		String to = p_args[1];
		if (!from.is_valid_integer() && !from.ends_with(".size()") && !from.ends_with(".length()")) {
			from = "int(" + from + ")";
		}
		if (!to.is_valid_integer() && !to.ends_with(".size()") && !to.ends_with(".length()")) {
			to = "int(" + to + ")";
		}
		if (from == "0") {
			r_replacement = _wrap_unary(p_before, "randi() % " + (_is_simple_expression(to) ? to : "(" + to + ")"));
		} else {
			r_replacement = "(randi() % (" + to + " - " + from + ") + " + from + ")";
		}
		return true;
	}

	/* JSON */

	if (name == "parse_json" && receiver != "" && argc == 1) {
		if (p_statement) {
			r_replacement = receiver + " = parse_json(" + args + ")";
			return true;
		}
		p_ctx.needs_review = true;
		return false;
	}

	if (name == "to_json" && receiver != "" && argc == 0) {
		r_replacement = "to_json(" + receiver + ")";
		return true;
	}

	/* Simple renames */

	String renamed = _table_lookup(script_method_renames, name);
	if (renamed != String()) {
		r_replacement = _with_receiver(receiver, renamed) + "(" + args + ")";
		return true;
	}

	// Most of the *_pos methods became *_position
	if (name.ends_with("_pos")) {
		r_replacement = _with_receiver(receiver, name.substr(0, name.length() - 4) + "_position") + "(" + args + ")";
		return true;
	}

	return false;
}

Error EditorExportGodot3::_convert_script(const String &p_path, const String &p_target_path, bool mark_converted_lines) {

	FileAccessRef src = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V(!src.operator->(), FAILED);

	Vector<String> lines;
	while (!src->eof_reached()) {
		lines.push_back(src->get_line());
	}
	src->close();

	// Remove the extra empty line added by eof
	if (lines.size() && lines[lines.size() - 1] == "") {
		lines.resize(lines.size() - 1);
	}

	Vector<String> result = _convert_script_lines(p_path, lines, mark_converted_lines);

	FileAccessRef dst = FileAccess::open(p_target_path, FileAccess::WRITE);
	ERR_FAIL_COND_V(!dst.operator->(), FAILED);

	for (int i = 0; i < result.size(); i++) {
		dst->store_line(result[i]);
	}

	return OK;
}

Vector<String> EditorExportGodot3::_convert_script_lines(const String &p_path, const Vector<String> &p_lines, bool mark_converted_lines) {

	Vector<String> lines = p_lines;

	const String note = "  #-- NOTE: Automatically converted by Godot 2 to 3 converter, please review";

	ScriptContext ctx;
	ctx.path = p_path;
	ctx.base_type = _get_script_base_type_from_lines(p_path, lines);
	if (script_member_renames.has(p_path)) {
		ctx.local_renames = script_member_renames[p_path];
	}

	// Functions declared by this script and its parents, RegEx and Image variables
	{
		Vector<String> chain_lines = lines;
		String chain_path = p_path;
		for (int depth = 0; depth < 16; depth++) {
			String parent;
			for (int i = 0; i < chain_lines.size(); i++) {
				String stripped = chain_lines[i].strip_edges();
				if (stripped.begins_with("static ")) {
					stripped = stripped.substr(7, stripped.length()).strip_edges();
				}
				if (stripped.begins_with("func ")) {
					ctx.declared_functions.insert(stripped.substr(5, stripped.length()).get_slice("(", 0).strip_edges());
				} else if (parent == String() && (stripped.begins_with("extends \"") || stripped.begins_with("extends '"))) {
					parent = stripped.substr(9, stripped.length()).get_slice(stripped.substr(8, 1), 0);
					if (!parent.begins_with("res://")) {
						parent = chain_path.get_base_dir().plus_file(parent);
					}
				}
			}
			if (parent == String()) {
				break;
			}
			chain_path = parent;
			chain_lines = _read_lines(parent);
		}

		for (int i = 0; i < lines.size(); i++) {
			String stripped = lines[i].strip_edges();
			int eq = stripped.find("=");
			if (eq == -1) {
				continue;
			}
			String var = stripped.substr(0, eq).replace("var ", "").replace("onready ", "").strip_edges();
			String value = stripped.substr(eq + 1, stripped.length()).strip_edges();
			if (!var.is_valid_identifier()) {
				continue;
			}
			if (!lines[i].begins_with(" ") && !lines[i].begins_with("\t")) {
				ctx.member_vars.insert(var);
			}
			if (value.begins_with("RegEx.new()")) {
				ctx.regex_vars.insert(var);
			} else if (value.begins_with("Image(") || value.begins_with("Image.new()")) {
				ctx.image_vars.insert(var);
			}
		}
	}

	// Narrow "extends" to the type of the nodes using the script, Godot 3 checks the methods
	// called on self when parsing. Also follow the nodes which changed type.
	for (int i = 0; i < lines.size(); i++) {

		String stripped = lines[i].strip_edges();
		String prefix;
		if (stripped.begins_with("tool")) {
			prefix = "tool ";
			stripped = stripped.substr(4, stripped.length()).strip_edges();
		}
		if (!stripped.begins_with("extends ")) {
			continue;
		}

		String base = stripped.substr(8, stripped.length()).get_slice("#", 0).strip_edges();
		if (!base.is_valid_identifier() || base != ctx.base_type) {
			break;
		}

		String new_base = base;
		if (script_owners.has(p_path)) {
			const Set<String> &owners = script_owners[p_path];
			if (owners.size() == 1 && owners.front()->get() != base && ObjectTypeDB::is_type(owners.front()->get(), base)) {
				new_base = owners.front()->get();
			}
		}
		if (new_base == "ButtonGroup") {
			new_base = "VBoxContainer";
		} else if (new_base == "Particles2D") {
			new_base = "CPUParticles2D";
		}

		if (new_base != base) {
			print_line("-- " + p_path + ": extends " + base + " changed to extends " + new_base);
			lines[i] = prefix + "extends " + new_base;
			ctx.base_type = ObjectTypeDB::type_exists(new_base) ? new_base : base;
		}
		break;
	}

	// Godot 3 doesn't allow redefining a constant of a parent script, rename the ones of this script
	{
		Set<String> parent_consts;
		String parent_path;
		for (int i = 0; i < lines.size(); i++) {
			String stripped = lines[i].strip_edges();
			if (stripped.begins_with("extends \"") || stripped.begins_with("extends '")) {
				parent_path = stripped.substr(9, stripped.length()).get_slice(stripped.substr(8, 1), 0);
				break;
			}
		}

		for (int depth = 0; parent_path != String() && depth < 16; depth++) {
			if (!parent_path.begins_with("res://")) {
				parent_path = p_path.get_base_dir().plus_file(parent_path);
			}
			FileAccessRef pf = FileAccess::open(parent_path, FileAccess::READ);
			String next_path;
			while (pf && !pf->eof_reached()) {
				String l = pf->get_line();
				String stripped = l.strip_edges();
				if (l.begins_with("const ")) {
					String decl = stripped.substr(6, stripped.length()).strip_edges();
					int end = 0;
					while (end < decl.length() && _is_ident_char(decl[end])) {
						end++;
					}
					parent_consts.insert(decl.substr(0, end));
				} else if (next_path == String() && (stripped.begins_with("extends \"") || stripped.begins_with("extends '"))) {
					next_path = stripped.substr(9, stripped.length()).get_slice(stripped.substr(8, 1), 0);
					if (!next_path.begins_with("res://")) {
						next_path = parent_path.get_base_dir().plus_file(next_path);
					}
				}
			}
			parent_path = next_path;
		}

		String suffix = p_path.get_file().basename().to_upper();
		for (int i = 0; i < suffix.length(); i++) {
			if (!_is_ident_char(suffix[i])) {
				suffix[i] = '_';
			}
		}

		for (int i = 0; i < lines.size(); i++) {
			if (!lines[i].begins_with("const ")) {
				continue;
			}
			String decl = lines[i].substr(6, lines[i].length()).strip_edges();
			int end = 0;
			while (end < decl.length() && _is_ident_char(decl[end])) {
				end++;
			}
			String name = decl.substr(0, end);
			if (parent_consts.has(name)) {
				ctx.local_renames[name] = name + "_" + suffix;
				print_line("-- " + p_path + ": constant " + name + " redefines the one of a parent script, renamed to " + name + "_" + suffix);
			}
		}
	}
	ctx.is_control = ObjectTypeDB::is_type(ctx.base_type, "Control");
	ctx.is_kinematic = ObjectTypeDB::is_type(ctx.base_type, "KinematicBody2D") || ObjectTypeDB::is_type(ctx.base_type, "KinematicBody");
	ctx.is_raycast = ObjectTypeDB::is_type(ctx.base_type, "RayCast2D") || ObjectTypeDB::is_type(ctx.base_type, "RayCast");
	ctx.is_spatial = ObjectTypeDB::is_type(ctx.base_type, "Spatial");
	ctx.is_color_picker = ObjectTypeDB::is_type(ctx.base_type, "ColorPicker") || ObjectTypeDB::is_type(ctx.base_type, "ColorPickerButton");
	bool is_node = ObjectTypeDB::is_type(ctx.base_type, "Node");

	/* Callbacks which are processed automatically in Godot 3, but needed to be enabled in 2.1 */

	static const char *callbacks[][2] = {
		{ "_process", "set_process" },
		{ "_fixed_process", "set_physics_process" },
		{ "_input", "set_process_input" },
		{ "_unhandled_input", "set_process_unhandled_input" },
		{ "_unhandled_key_input", "set_process_unhandled_key_input" },
		{ NULL, NULL }
	};

	Vector<String> disable_lines;
	String indent = "\t";
	bool indent_found = false;
	int ready_line = -1;
	{
		Set<String> defined;
		Set<String> enabled_early;
		String current_func;

		for (int i = 0; i < lines.size(); i++) {

			String line = lines[i];
			String stripped = line.strip_edges();

			// The indentation unit is the smallest indentation of the file
			if (stripped != "" && (line.begins_with(" ") || line.begins_with("\t")) && !stripped.begins_with("#")) {
				String line_indent = line.substr(0, line.length() - line.strip_edges(true, false).length());
				if (!indent_found || line_indent.length() < indent.length()) {
					indent = line_indent;
					indent_found = true;
				}
			}

			if (line.begins_with("func ") || line.begins_with("static func ")) {
				String decl = line.substr(line.find("func ") + 5, line.length()).strip_edges();
				current_func = decl.get_slice("(", 0).strip_edges();
				defined.insert(current_func);
				if (current_func == "_ready") {
					ready_line = i;
				}
			} else if (stripped != "" && !line.begins_with(" ") && !line.begins_with("\t") && !stripped.begins_with("#")) {
				current_func = "";
			}

			if (current_func == "_init" || current_func == "_enter_tree") {
				for (int j = 0; callbacks[j][0]; j++) {
					if (stripped.find(String(callbacks[j][1]) + "(true)") != -1) {
						enabled_early.insert(callbacks[j][0]);
					}
				}
			}
		}

		if (is_node) {
			for (int j = 0; callbacks[j][0]; j++) {
				if (defined.has(callbacks[j][0]) && !enabled_early.has(callbacks[j][0])) {
					disable_lines.push_back(String(callbacks[j][1]) + "(false)");
				}
			}
		}

		String ready_rest = ready_line != -1 ? lines[ready_line].get_slice(")", 1).get_slice(":", 1) : String();
		if (ready_rest.find("#") != -1) {
			ready_rest = ready_rest.substr(0, ready_rest.find("#"));
		}
		if (ready_line != -1 && ready_rest.strip_edges() != "") {
			// One line function, don't touch it
			print_line("WARNING: " + p_path + ": _ready is defined on a single line, the processing callbacks were not disabled.");
			disable_lines.clear();
		}
	}

	// Methods connected to AnimationPlayer's "animation_finished" receive the animation name
	if (animation_finished_methods.has(p_path)) {
		for (int i = 0; i < lines.size(); i++) {
			if (!lines[i].begins_with("func ")) {
				continue;
			}
			String name = lines[i].substr(5, lines[i].length()).get_slice("(", 0).strip_edges();
			int open = lines[i].find("(");
			int close = lines[i].find(")");
			if (animation_finished_methods[p_path].has(name) && open != -1 && close == open + 1) {
				lines[i] = lines[i].substr(0, open + 1) + "_anim_name = \"\"" + lines[i].substr(close, lines[i].length());
			}
		}
	}

	// InputEvent became a class hierarchy: "var ev = InputEvent()" followed by "ev.type = InputEvent.KEY"
	// creates an InputEventKey
	for (int i = 0; i < lines.size(); i++) {

		int ctor = lines[i].find("InputEvent()");
		if (ctor == -1) {
			continue;
		}

		String var = lines[i].substr(0, ctor).replace("var ", "").replace("=", "").strip_edges();
		if (!var.is_valid_identifier()) {
			continue;
		}

		for (int j = i + 1; j < lines.size() && j < i + 4; j++) {
			String stripped = lines[j].strip_edges();
			if (!stripped.begins_with(var + ".type") || stripped.find("InputEvent.") == -1) {
				continue;
			}
			String new_type = _table_lookup(script_input_event_types, stripped.get_slice("InputEvent.", 1).strip_edges());
			if (new_type != String()) {
				lines[i] = lines[i].replace("InputEvent()", new_type + ".new()");
				lines.remove(j);
			}
			break;
		}
	}

	// Godot 3 doesn't allow two arguments with the same name, 2.1 did
	for (int i = 0; i < lines.size(); i++) {
		String stripped = lines[i].strip_edges();
		if (!stripped.begins_with("func ") && !stripped.begins_with("static func ")) {
			continue;
		}
		int open = lines[i].find("(");
		int close = lines[i].find(")", open);
		if (open == -1 || close == -1) {
			continue;
		}
		Vector<String> args = lines[i].substr(open + 1, close - open - 1).split(",");
		Set<String> names;
		bool changed = false;
		for (int j = 0; j < args.size(); j++) {
			String arg = args[j].strip_edges();
			String name = arg.get_slice("=", 0).strip_edges();
			if (name != String() && names.has(name)) {
				int k = 2;
				while (names.has(name + "_" + itos(k))) {
					k++;
				}
				args[j] = " " + name + "_" + itos(k) + arg.substr(name.length(), arg.length());
				name = name + "_" + itos(k);
				changed = true;
			}
			names.insert(name);
		}
		if (changed) {
			String joined;
			for (int j = 0; j < args.size(); j++) {
				joined += (j ? "," : "") + args[j];
			}
			lines[i] = lines[i].substr(0, open + 1) + joined + lines[i].substr(close, lines[i].length());
		}
	}

	/* Godot 3 doesn't allow declaring twice a local variable in the same block, 2.1 did */
	{
		Map<String, int> locals; // name => indentation of the declaration
		for (int i = 0; i < lines.size(); i++) {

			String stripped = lines[i].strip_edges();
			if (stripped == "" || stripped.begins_with("#")) {
				continue;
			}

			int indent_len = lines[i].length() - lines[i].strip_edges(true, false).length();
			if (indent_len == 0) {
				locals.clear();
				continue;
			}

			// Leaving blocks
			Vector<String> closed;
			for (Map<String, int>::Element *E = locals.front(); E; E = E->next()) {
				if (E->get() > indent_len) {
					closed.push_back(E->key());
				}
			}
			for (int j = 0; j < closed.size(); j++) {
				locals.erase(closed[j]);
			}

			if (!stripped.begins_with("var ")) {
				continue;
			}

			String decl = stripped.substr(4, stripped.length()).strip_edges();
			int end = 0;
			while (end < decl.length() && _is_ident_char(decl[end])) {
				end++;
			}
			String name = decl.substr(0, end);

			if (locals.has(name) && locals[name] == indent_len) {
				lines[i] = lines[i].substr(0, indent_len) + decl;
			} else if (locals.has(name) && locals[name] < indent_len) {
				// Shadowing a variable of an outer block, rename it in its block
				int k = 2;
				while (locals.has(name + "_" + itos(k))) {
					k++;
				}
				String new_name = name + "_" + itos(k);
				for (int j = i; j < lines.size(); j++) {
					String other = lines[j].strip_edges();
					int other_indent = lines[j].length() - lines[j].strip_edges(true, false).length();
					if (j > i && other != "" && !other.begins_with("#") && other_indent < indent_len) {
						break;
					}
					lines[j] = _rename_identifier(lines[j], name, new_name);
				}
				locals[new_name] = indent_len;
			} else {
				locals[name] = indent_len;
			}
		}
	}

	/* Convert the lines */

	Vector<String> result;
	String multiline;

	for (int i = 0; i < lines.size(); i++) {

		String line = lines[i];

		String code;
		String comment;
		Vector<String> strings;
		bool was_multiline = multiline != "";
		_split_script_line(line, code, strings, comment, multiline);

		// Try to detect a HTTPClient object
		int http = code.find("HTTPClient.new()");
		if (http != -1) {
			String var = code.substr(0, http).replace("var ", "").replace("=", "").strip_edges();
			if (var.is_valid_identifier()) {
				ctx.http_vars.insert(var);
			}
		}

		ctx.needs_review = false;
		ctx.cyclic_preload = false;
		ctx.line_indent = line.substr(0, line.length() - line.strip_edges(true, false).length());
		ctx.indent_unit = indent;
		ctx.prefix_statements.clear();
		ctx.postfix_statements.clear();

		String stripped_line = line.strip_edges();
		ctx.prefix_allowed = !stripped_line.begins_with("elif ") && !stripped_line.begins_with("while ") && !stripped_line.begins_with("elif(") && !stripped_line.begins_with("while(");
		ctx.postfix_allowed = stripped_line.begins_with("var ") || (!stripped_line.begins_with("if ") && !stripped_line.begins_with("elif ") && !stripped_line.begins_with("while ") && !stripped_line.begins_with("for ") && !stripped_line.begins_with("return ") && !stripped_line.ends_with(":"));

		// Static functions
		if (line.begins_with("static func ") || line.begins_with("func ")) {
			ctx.function_locals.clear();
		}
		if (line.begins_with("static func ")) {
			ctx.in_static = true;
		} else if (line.begins_with("func ") || (stripped_line != "" && !line.begins_with(" ") && !line.begins_with("\t") && !stripped_line.begins_with("#") && !was_multiline)) {
			ctx.in_static = false;
		}

		String converted = _convert_script_code(code, strings, ctx);

		if (ctx.cyclic_preload && converted.begins_with("const ")) {
			// load() is not a constant expression
			converted = "var " + converted.substr(6, converted.length());
			ctx.needs_review = true;
		}

		for (int j = 0; j < strings.size(); j++) {
			String str = strings[j];
			if (str.length() < 2 || str.length() > 256) {
				continue;
			}
			String quote = str.substr(0, 1);
			String content = str.substr(1, str.length() - 2);

			// Resource paths may have been renamed (.xml => .tscn, .tex => .png...)
			if (content.begins_with("res://")) {
				if (resource_replace_map.has(content) && resource_replace_map[content] != content) {
					strings[j] = quote + resource_replace_map[content] + quote;
				}
				continue;
			}

			// Property names given as strings (Tween, set, get, animation tracks...), with an optional node path
			String node_path;
			String prop = content;
			if (content.find(":") != -1) {
				node_path = content.get_slice(":", 0) + ":";
				prop = content.substr(node_path.length(), content.length());
			}
			if (prop.find("/") == -1 || prop.find(" ") != -1) {
				continue;
			}

			String new_prop;
			if (prop == "visibility/opacity") {
				new_prop = "modulate:a";
			} else if (prop == "visibility/self_opacity") {
				new_prop = "self_modulate:a";
			} else if (prop_rename_map.has(prop)) {
				new_prop = prop_rename_map[prop];
			}

			if (new_prop != String()) {
				strings[j] = quote + node_path + new_prop + quote;
				if (prop == "transform/rot" || prop == "rect/rotation") {
					// The values have to be negated, see _rename_properties
					ctx.needs_review = true;
				}
			}
		}

		String new_line = _join_script_line(converted, strings, comment);

		if (mark_converted_lines && !was_multiline && (new_line != line || ctx.needs_review)) {
			new_line += note;
		}

		if (ctx.prefix_statements.size()) {
			String line_indent = line.substr(0, line.length() - line.strip_edges(true, false).length());
			for (int j = 0; j < ctx.prefix_statements.size(); j++) {
				result.push_back(line_indent + _join_script_line(ctx.prefix_statements[j], strings, String()) + (mark_converted_lines ? note : String()));
			}
		}

		result.push_back(new_line);

		for (int j = 0; j < ctx.postfix_statements.size(); j++) {
			result.push_back(ctx.line_indent + _join_script_line(ctx.postfix_statements[j], strings, String()) + (mark_converted_lines ? note : String()));
		}

		if (i == ready_line && disable_lines.size()) {
			// Find the indentation of the body
			String body_indent = indent;
			for (int j = i + 1; j < lines.size(); j++) {
				if (lines[j].strip_edges() != "") {
					body_indent = lines[j].substr(0, lines[j].length() - lines[j].strip_edges(true, false).length());
					break;
				}
			}
			for (int j = 0; j < disable_lines.size(); j++) {
				result.push_back(body_indent + disable_lines[j] + (mark_converted_lines ? note : String()));
			}
		}
	}

	if (ready_line == -1 && disable_lines.size()) {
		result.push_back("");
		result.push_back("func _ready():");
		for (int j = 0; j < disable_lines.size(); j++) {
			result.push_back(indent + disable_lines[j] + (mark_converted_lines ? note : String()));
		}
	}

	/* class_name given with "# CONVERT2TO3_CLASS_NAME" followed by "# class_name Name" */

	for (int i = 0; i < result.size(); i++) {

		String stripped = result[i].strip_edges();

		// Preloads of scripts which have a class_name now, under the same name
		if (stripped.begins_with("const ") && stripped.find("preload(") != -1) {
			String name = stripped.substr(6, stripped.length()).get_slice("=", 0).strip_edges();
			String path = _get_quoted_path(stripped.get_slice("preload(", 1), p_path);
			if (class_names.has(path) && class_names[path].name == name) {
				result.remove(i);
				i--;
				continue;
			}
		}

		// Inheritance of scripts which have a class_name now
		if (stripped.begins_with("extends \"") || stripped.begins_with("extends '")) {
			String path = _get_quoted_path(stripped.substr(8, stripped.length()), p_path);
			String rest = stripped.substr(8, stripped.length()).strip_edges();
			rest = rest.substr(rest.find(rest.substr(0, 1), 1) + 1, rest.length());
			if (class_names.has(path) && rest.strip_edges() == "") {
				result[i] = result[i].substr(0, result[i].find("extends")) + "extends " + class_names[path].name;
			}
		}
	}

	if (class_names.has(p_path)) {
		int directive = -1;
		for (int i = 0; i + 1 < result.size(); i++) {
			if (result[i].strip_edges() == "# CONVERT2TO3_CLASS_NAME") {
				directive = i;
				break;
			}
		}
		if (directive != -1) {
			result.remove(directive);
			result.remove(directive);
		}

		int insert_at = 0;
		for (int i = 0; i < result.size(); i++) {
			String stripped = result[i].strip_edges();
			if (stripped.begins_with("extends ") || stripped.begins_with("tool extends ")) {
				insert_at = i + 1;
				break;
			}
		}
		result.insert(insert_at, "class_name " + class_names[p_path].name + class_names[p_path].extra);
	}

	if (ctx.uses_kinematic_collision) {
		ctx.extra_members.insert("_kinematic_collision");
	}

	if (ctx.extra_members.size()) {
		// Declare the members added by the conversion (last collision, last regex match) after "extends"
		int insert_at = 0;
		for (int i = 0; i < result.size(); i++) {
			String stripped = result[i].strip_edges();
			if (stripped.begins_with("extends ") || stripped.begins_with("tool extends ")) {
				insert_at = i + 1;
				break;
			}
		}
		result.insert(insert_at, "");
		for (Set<String>::Element *E = ctx.extra_members.back(); E; E = E->prev()) {
			result.insert(insert_at + 1, "var " + E->get() + " = null" + (mark_converted_lines ? note : String()));
		}
	}

	return result;
}

bool EditorExportGodot3::_is_godot3_member(const String &p_base_type, const String &p_name) {

	if (godot3_member_map.empty()) {
		for (int i = 0; godot3_members[i][0]; i++) {
			Vector<String> names = String(godot3_members[i][1]).split(" ");
			for (int j = 0; j < names.size(); j++) {
				godot3_member_map[godot3_members[i][0]].insert(names[j]);
			}
		}
	}

	String type = p_base_type;
	while (type != String()) {
		String type3 = type;
		if (type == "Particles2D") {
			type3 = "CPUParticles2D";
		} else if (type_rename_map.has(type)) {
			type3 = type_rename_map[type];
		}
		if (godot3_member_map.has(type3) && godot3_member_map[type3].has(p_name)) {
			return true;
		}
		type = ObjectTypeDB::type_inherits_from(type);
	}
	return false;
}

static void _get_script_members(const Vector<String> &p_lines, Set<String> &r_members) {

	for (int i = 0; i < p_lines.size(); i++) {
		String line = p_lines[i];
		if (line.begins_with(" ") || line.begins_with("\t")) {
			continue;
		}
		line = line.strip_edges();
		bool is_const = line.begins_with("const ");
		if (!is_const && !line.begins_with("var ") && !((line.begins_with("onready ") || line.begins_with("export")) && line.find("var ") != -1)) {
			continue;
		}
		String decl = line.substr(line.find(is_const ? "const " : "var ") + (is_const ? 6 : 4), line.length()).strip_edges();
		int end = 0;
		while (end < decl.length() && _is_ident_char(decl[end])) {
			end++;
		}
		r_members.insert(decl.substr(0, end));
	}
}

void EditorExportGodot3::_find_member_renames(const List<String> &p_files) {

	// Members declared by scripts which became properties of their base class in Godot 3
	// ("position", "name", "text"...) are renamed, in the scripts and the scenes using them.
	script_member_renames.clear();

	for (const List<String>::Element *E = p_files.front(); E; E = E->next()) {

		String path = E->get();
		if (path.extension().to_lower() != "gd") {
			continue;
		}

		String base_type = _get_script_base_type(path);
		if (base_type == String()) {
			continue;
		}

		// Members of the script and of its parent scripts
		Set<String> members;
		String script_path = path;
		for (int depth = 0; script_path != String() && depth < 16; depth++) {
			Vector<String> lines = _read_lines(script_path);
			_get_script_members(lines, members);

			String next_path;
			for (int i = 0; i < lines.size(); i++) {
				String stripped = lines[i].strip_edges();
				if (stripped.begins_with("extends \"") || stripped.begins_with("extends '")) {
					next_path = stripped.substr(9, stripped.length()).get_slice(stripped.substr(8, 1), 0);
					if (!next_path.begins_with("res://")) {
						next_path = script_path.get_base_dir().plus_file(next_path);
					}
					break;
				}
			}
			script_path = next_path;
		}

		for (Set<String>::Element *M = members.front(); M; M = M->next()) {
			if (M->get() == String() || !_is_godot3_member(base_type, M->get())) {
				continue;
			}
			String new_name = "_" + M->get();
			if (members.has(new_name)) {
				new_name = M->get() + "_value";
			}
			script_member_renames[path][M->get()] = new_name;
			print_line("-- " + path + ": member " + M->get() + " is a property of " + base_type + " in Godot 3, renamed to " + new_name);
		}
	}
}

String EditorExportGodot3::_get_quoted_path(const String &p_text, const String &p_script_path) {

	// The path of the first string in p_text, made absolute
	String text = p_text.strip_edges();
	if (!text.begins_with("\"") && !text.begins_with("'")) {
		return String();
	}

	String path = text.substr(1, text.length()).get_slice(text.substr(0, 1), 0);
	if (!path.begins_with("res://")) {
		path = p_script_path.get_base_dir().plus_file(path);
	}
	return path;
}

void EditorExportGodot3::_find_class_names(const List<String> &p_files) {

	// Scripts declaring their Godot 3 class_name with:
	// # CONVERT2TO3_CLASS_NAME
	// # class_name Name
	class_names.clear();

	Set<String> reserved;
	Vector<String> classes = String(godot3_classes).split(" ", false);
	for (int i = 0; i < classes.size(); i++) {
		reserved.insert(classes[i]);
	}

	Set<String> used;
	for (const List<String>::Element *E = p_files.front(); E; E = E->next()) {

		String path = E->get();
		if (path.extension().to_lower() != "gd") {
			continue;
		}

		Vector<String> lines = _read_lines(path);
		for (int i = 0; i + 1 < lines.size(); i++) {

			if (lines[i].strip_edges() != "# CONVERT2TO3_CLASS_NAME") {
				continue;
			}

			String decl = lines[i + 1].strip_edges();
			decl = decl.substr(1, decl.length()).strip_edges(); // "#"
			if (!decl.begins_with("class_name ")) {
				print_line("WARNING: " + path + ": CONVERT2TO3_CLASS_NAME must be followed by \"# class_name Name\".");
				break;
			}

			decl = decl.substr(11, decl.length()).strip_edges();
			int end = 0;
			while (end < decl.length() && _is_ident_char(decl[end])) {
				end++;
			}

			ClassNameData data;
			data.name = decl.substr(0, end);
			data.extra = decl.substr(end, decl.length()); // Icon: , "res://icon.png"

			String error;
			if (!data.name.is_valid_identifier()) {
				error = "is not a valid identifier";
			} else if (reserved.has(data.name) || ObjectTypeDB::type_exists(data.name)) {
				error = "is the name of an engine class";
			} else if (Globals::get_singleton()->has("autoload/" + data.name)) {
				error = "is the name of an autoload";
			} else if (used.has(data.name)) {
				error = "is used by another script";
			}

			if (error != String()) {
				print_line("WARNING: " + path + ": class_name " + data.name + " " + error + ", it was ignored.");
				break;
			}

			used.insert(data.name);
			class_names[path] = data;
			print_line("-- " + path + ": class_name " + data.name);
			break;
		}
	}

	// Base types, for the list of global classes of project.godot
	for (Map<String, ClassNameData>::Element *E = class_names.front(); E; E = E->next()) {

		Vector<String> lines = _read_lines(E->key());
		String base;
		for (int i = 0; i < lines.size() && base == String(); i++) {
			String stripped = lines[i].strip_edges();
			if (stripped.begins_with("tool")) {
				stripped = stripped.substr(4, stripped.length()).strip_edges();
			}
			if (!stripped.begins_with("extends ")) {
				continue;
			}
			String parent = _get_quoted_path(stripped.substr(8, stripped.length()), E->key());
			if (parent != String()) {
				base = class_names.has(parent) ? class_names[parent].name : _get_script_base_type(parent);
			} else {
				base = stripped.substr(8, stripped.length()).get_slice("#", 0).strip_edges();
			}
		}

		if (base == String()) {
			base = "Reference";
		}
		if (type_rename_map.has(base)) {
			base = type_rename_map[base];
		}
		E->get().base = base;
	}
}

void EditorExportGodot3::_find_cyclic_preloads(const List<String> &p_files) {

	// Godot 3 can't preload resources depending on the script preloading them, 2.1 could.
	// Those preloads are turned into loads.
	cyclic_preloads.clear();

	Map<String, Vector<String> > deps;
	Map<String, Vector<String> > preloads;

	for (const List<String>::Element *E = p_files.front(); E; E = E->next()) {

		String path = E->get();

		if (path.extension().to_lower() == "gd") {
			FileAccessRef f = FileAccess::open(path, FileAccess::READ);
			while (f && !f->eof_reached()) {
				String line = f->get_line();
				int pos = 0;
				while ((pos = line.find("preload(", pos)) != -1) {
					pos += 8;
					String rest = line.substr(pos, line.length()).strip_edges();
					if (rest.begins_with("\"") || rest.begins_with("'")) {
						String target = rest.substr(1, rest.length()).get_slice(rest.substr(0, 1), 0);
						if (!target.begins_with("res://")) {
							target = path.get_base_dir().plus_file(target);
						}
						deps[path].push_back(target);
						preloads[path].push_back(target);
					}
				}
				String stripped = line.strip_edges();
				if (stripped.begins_with("extends \"") || stripped.begins_with("extends '")) {
					String target = stripped.substr(9, stripped.length()).get_slice(stripped.substr(8, 1), 0);
					if (!target.begins_with("res://")) {
						target = path.get_base_dir().plus_file(target);
					}
					deps[path].push_back(target);
				}
			}
		} else {
			List<String> resource_deps;
			ResourceLoader::get_dependencies(path, &resource_deps);
			for (List<String>::Element *D = resource_deps.front(); D; D = D->next()) {
				deps[path].push_back(D->get().get_slice("::", 0));
			}
		}
	}

	for (Map<String, Vector<String> >::Element *E = preloads.front(); E; E = E->next()) {

		for (int i = 0; i < E->get().size(); i++) {

			// Does the preloaded resource depend on the script?
			String target = E->get()[i];
			Set<String> visited;
			List<String> queue;
			queue.push_back(target);
			bool cyclic = false;

			while (queue.size() && !cyclic) {
				String current = queue.front()->get();
				queue.pop_front();
				if (visited.has(current) || !deps.has(current)) {
					continue;
				}
				visited.insert(current);
				const Vector<String> &next = deps[current];
				for (int j = 0; j < next.size(); j++) {
					if (next[j] == E->key()) {
						cyclic = true;
						break;
					}
					queue.push_back(next[j]);
				}
			}

			if (cyclic) {
				cyclic_preloads.insert(E->key() + "|" + target);
				print_line("-- " + E->key() + ": preload of " + target + " is cyclic, it was changed to load");
			}
		}
	}
}

void EditorExportGodot3::_find_script_owners(const List<String> &p_files) {

	// Types of the nodes each script is attached to
	script_owners.clear();
	animation_finished_methods.clear();

	for (const List<String>::Element *E = p_files.front(); E; E = E->next()) {

		if (ResourceLoader::get_resource_type(E->get()) != "PackedScene") {
			continue;
		}

		Ref<PackedScene> scene = ResourceLoader::load(E->get());
		if (scene.is_null()) {
			continue;
		}

		Ref<SceneState> state = scene->get_state();
		Map<String, int> node_indices;
		Vector<String> node_types;
		Vector<String> node_scripts;

		for (int i = 0; i < state->get_node_count(); i++) {

			node_indices[String(state->get_node_path(i))] = i;
			node_types.push_back(String());
			node_scripts.push_back(String());

			String type = state->get_node_type(i);
			if (type == String() && state->get_node_instance(i).is_valid()) {
				Ref<SceneState> instance_state = state->get_node_instance(i)->get_state();
				type = instance_state->get_node_count() ? String(instance_state->get_node_type(0)) : String();
			}
			if (type == String()) {
				continue;
			}

			for (int j = 0; j < state->get_node_property_count(i); j++) {
				if (String(state->get_node_property_name(i, j)) != "script/script") {
					continue;
				}
				Ref<Script> script = state->get_node_property_value(i, j);
				if (script.is_valid() && script->get_path() != String()) {
					script_owners[script->get_path()].insert(type);
					node_scripts[i] = script->get_path();
				}
			}
			node_types[i] = type;
		}

		// AnimationPlayer's "animation_finished" has an argument that "finished" had not
		for (int i = 0; i < state->get_connection_count(); i++) {

			if (String(state->get_connection_signal(i)) != "finished" || state->get_connection_binds(i).size()) {
				continue;
			}

			String source = state->get_connection_source(i);
			String target = state->get_connection_target(i);
			if (!node_indices.has(source) || !node_indices.has(target)) {
				continue;
			}

			String type = node_types[node_indices[source]];
			String script = node_scripts[node_indices[target]];
			if (script != String() && ObjectTypeDB::is_type(type, "AnimationPlayer")) {
				animation_finished_methods[script].insert(state->get_connection_method(i));
			}
		}
	}
}

void EditorExportGodot3::_find_user_functions(const List<String> &p_files) {

	// Functions declared by the project are not renamed in calls, unless the engine has a method
	// with the same name
	user_functions.clear();
	user_members.clear();

	if (engine_methods.size() == 0) {
		List<StringName> types;
		ObjectTypeDB::get_type_list(&types);
		for (List<StringName>::Element *T = types.front(); T; T = T->next()) {
			List<MethodInfo> methods;
			ObjectTypeDB::get_method_list(T->get(), &methods, true);
			for (List<MethodInfo>::Element *M = methods.front(); M; M = M->next()) {
				engine_methods.insert(M->get().name);
			}
		}
	}

	Set<String> engine_callbacks;
	static const char *callbacks[] = { "_init", "_ready", "_process", "_fixed_process", "_input", "_unhandled_input", "_unhandled_key_input", "_input_event", "_enter_tree", "_exit_tree", "_notification", "_draw", "_get", "_set", "_get_property_list", "_get_minimum_size", "_integrate_forces", NULL };
	for (int i = 0; callbacks[i]; i++) {
		engine_callbacks.insert(callbacks[i]);
	}

	for (const List<String>::Element *E = p_files.front(); E; E = E->next()) {

		if (E->get().extension().to_lower() != "gd") {
			continue;
		}

		FileAccessRef f = FileAccess::open(E->get(), FileAccess::READ);
		if (!f) {
			continue;
		}

		while (!f->eof_reached()) {
			String line = f->get_line().strip_edges();
			if (line.begins_with("static ")) {
				line = line.substr(7, line.length()).strip_edges();
			}
			bool is_const = line.begins_with("const ");
			if (is_const || line.begins_with("var ") || ((line.begins_with("onready ") || line.begins_with("export")) && line.find("var ") != -1)) {
				String decl = line.substr(line.find(is_const ? "const " : "var ") + (is_const ? 6 : 4), line.length()).strip_edges();
				int end = 0;
				while (end < decl.length() && _is_ident_char(decl[end])) {
					end++;
				}
				user_members.insert(decl.substr(0, end));
				continue;
			}
			if (!line.begins_with("func ")) {
				continue;
			}
			String name = line.substr(5, line.length()).get_slice("(", 0).strip_edges();
			if (!engine_callbacks.has(name)) {
				user_functions.insert(name);
			}
		}
	}
}

static bool _is_image_extension(const String &p_ext) {

	return p_ext == "png" || p_ext == "jpg" || p_ext == "jpeg" || p_ext == "webp" || p_ext == "bmp" || p_ext == "tga" || p_ext == "svg" || p_ext == "hdr" || p_ext == "exr";
}

bool EditorExportGodot3::_is_text_bmfont(const String &p_path) {

	// 2.1 could load BMFont text files (.fnt) directly, Godot 3 still can. Binary
	// resources start with "RSRC" (or "RSCC" when compressed).
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	if (!f) {
		return false;
	}

	uint8_t header[4] = { 0, 0, 0, 0 };
	f->get_buffer(header, 4);
	bool binary = header[0] == 'R' && header[1] == 'S' && ((header[2] == 'R' && header[3] == 'C') || (header[2] == 'C' && header[3] == 'C'));
	return !binary;
}

String EditorExportGodot3::_get_texture_target(const String &p_path) {

	// Godot 3 has no imported textures stored as resources anymore, the source image is
	// imported again. When the source image is not in the project, it is extracted from the .tex
	if (ResourceLoader::get_resource_type(p_path) != "ImageTexture") {
		return p_path;
	}

	Ref<ResourceImportMetadata> imd = ResourceLoader::load_import_metadata(p_path);
	if (imd.is_valid() && imd->get_source_count() > 1) {
		// Atlases and multiple sources can't be mapped to a single image
		return p_path;
	}

	if (imd.is_valid() && imd->get_source_count() == 1) {
		String src = EditorImportPlugin::expand_source_path(imd->get_source_path(0));
		if (src.begins_with("res://") && _is_image_extension(src.extension().to_lower()) && FileAccess::exists(src)) {
			return src;
		}
	}

	return p_path.basename() + ".png";
}

void EditorExportGodot3::_write_texture_import(const String &p_export_path, const String &p_source, uint32_t p_flags) {

	if (written_imports.has(p_export_path)) {
		return;
	}
	written_imports.insert(p_export_path);

	// Only the parameters are needed, Godot 3 reimports the file as the imported data is missing
	FileAccessRef f = FileAccess::open(p_export_path + ".import", FileAccess::WRITE);
	ERR_FAIL_COND(!f);

	f->store_line("[remap]\n");
	f->store_line("importer=\"texture\"");
	f->store_line("type=\"StreamTexture\"\n");
	f->store_line("[deps]\n");
	f->store_line("source_file=\"" + p_source + "\"\n");
	f->store_line("[params]\n");
	f->store_line("compress/mode=0");
	f->store_line("compress/lossy_quality=0.7");
	f->store_line("compress/hdr_mode=0");
	f->store_line("compress/bptc_ldr=0");
	f->store_line("compress/normal_map=0");
	f->store_line("flags/repeat=" + String((p_flags & Texture::FLAG_MIRRORED_REPEAT) ? "2" : ((p_flags & Texture::FLAG_REPEAT) ? "1" : "0")));
	f->store_line("flags/filter=" + String((p_flags & Texture::FLAG_FILTER) ? "true" : "false"));
	f->store_line("flags/mipmaps=" + String((p_flags & Texture::FLAG_MIPMAPS) ? "true" : "false"));
	f->store_line("flags/anisotropic=" + String((p_flags & Texture::FLAG_ANISOTROPIC_FILTER) ? "true" : "false"));
	f->store_line("flags/srgb=2");
	f->store_line("process/fix_alpha_border=true");
	f->store_line("process/premult_alpha=false");
	f->store_line("process/HDR_as_SRGB=false");
	f->store_line("process/invert_color=false");
	f->store_line("process/normal_map_invert_y=false");
	f->store_line("stream=false");
	f->store_line("size_limit=0");
	// Keep the flags of 2.1, even when the texture is used in 3D
	f->store_line("detect_3d=false");
	f->store_line("svg/scale=1.0");
}

Error EditorExportGodot3::_export_texture(const String &p_path, const String &p_export_path) {

	String target = _replace_resource(p_path);
	String target_export_path = p_export_path.plus_file(target.replace("res://", ""));

	Ref<ImageTexture> texture = ResourceLoader::load(p_path);
	ERR_FAIL_COND_V(texture.is_null(), ERR_CANT_OPEN);

	if (!FileAccess::exists(target)) {
		// The source image is not in the project, extract it
		Image img = texture->get_data();
		if (img.is_compressed() && img.decompress() != OK) {
			ERR_EXPLAIN("Can't decompress texture: " + p_path);
			ERR_FAIL_V(ERR_UNAVAILABLE);
		}
		if (img.get_format() != Image::FORMAT_RGBA && img.get_format() != Image::FORMAT_RGB && img.get_format() != Image::FORMAT_GRAYSCALE && img.get_format() != Image::FORMAT_GRAYSCALE_ALPHA) {
			img.convert(Image::FORMAT_RGBA);
		}
		img.clear_mipmaps();

		Error err = img.save_png(target_export_path);
		if (err != OK) {
			return err;
		}
		print_line("-- Extracted texture " + p_path + " to " + target);
	}

	_write_texture_import(target_export_path, target, texture->get_flags());
	return OK;
}

Error EditorExportGodot3::export_godot3(const String &p_path, bool convert_scripts, bool mark_converted_lines) {

	List<String> files;
	_find_files(EditorFileSystem::get_singleton()->get_filesystem(), &files);

	// Function renames of the export options: "old_name" or "Receiver.old_name" => replacement
	function_renames.clear();
	Dictionary renames;
	if (Globals::get_singleton()->has("godot3_export/function_renames")) {
		renames = Globals::get_singleton()->get("godot3_export/function_renames");
	}
	Array keys = renames.keys();
	for (int i = 0; i < keys.size(); i++) {
		String from = String(keys[i]).strip_edges().replace("()", "");
		String to = String(renames[keys[i]]).strip_edges().replace("()", "");
		if (from != String() && to != String()) {
			function_renames[from] = to;
		}
	}

	if (convert_scripts) {
		_find_class_names(files);
		_find_user_functions(files);
		_find_script_owners(files);
		_find_cyclic_preloads(files);
		_find_member_renames(files);
	}

	EditorProgress progress("exporting", "Exporting the project to Godot 3.0", files.size());

	// find XML resources

	resource_replace_map.clear();
	instanced_types.clear();
	export_convert_scripts = convert_scripts;
	export_mark_lines = mark_converted_lines;
	written_imports.clear();

	Set<String> xml_extensions;
	Set<String> binary_extensions;
	Set<String> text_extensions;

	{
		List<String> xml_exts;
		ResourceFormatLoaderXML::singleton->get_recognized_extensions(&xml_exts);
		for (List<String>::Element *E = xml_exts.front(); E; E = E->next()) {
			xml_extensions.insert(E->get());
		}
	}

	{
		List<String> binary_exts;
		ResourceFormatLoaderBinary::singleton->get_recognized_extensions(&binary_exts);
		for (List<String>::Element *E = binary_exts.front(); E; E = E->next()) {
			binary_extensions.insert(E->get());
		}
	}

	{
		List<String> text_exts;
		ResourceFormatLoaderText::singleton->get_recognized_extensions(&text_exts);
		for (List<String>::Element *E = text_exts.front(); E; E = E->next()) {
			text_extensions.insert(E->get());
		}
	}

	// Changing all the old extensions to new Godot 3.0 extensions.
	// Refer PR #9201. ImageTexture kept "tex", and imported textures are handled separately.
	Map<String, String> extension_renames;
	extension_renames["anm"] = "anim";
	extension_renames["asogg"] = "oggstr";
	extension_renames["atex"] = "atlastex";
	extension_renames["cbm"] = "cubemap";
	extension_renames["cvtex"] = "curvetex";
	extension_renames["fnt"] = "font";
	extension_renames["gt"] = "meshlib";
	extension_renames["ltex"] = "largetex";
	extension_renames["mmsh"] = "multimesh";
	extension_renames["msh"] = "mesh";
	extension_renames["mtl"] = "material";
	extension_renames["sbx"] = "stylebox";
	extension_renames["sgp"] = "vshader";
	extension_renames["shd"] = "shader";
	extension_renames["shp"] = "shape";
	extension_renames["smp"] = "sample";
	extension_renames["thm"] = "theme";
	extension_renames["wrd"] = "world";
	extension_renames["xl"] = "translation";

	Set<String> text_bmfonts;
	Set<String> texture_sources;

	for (List<String>::Element *E = files.front(); E; E = E->next()) {

		String file = E->get();
		String extension = file.extension().to_lower();
		String target = file;

		if (xml_extensions.has(extension)) {
			if (ResourceLoader::get_resource_type(file) == "PackedScene") {
				target = file.basename() + ".tscn";
			} else {
				target = file.basename() + ".tres";
			}
		} else if (extension == "fnt" && _is_text_bmfont(file)) {
			text_bmfonts.insert(file);
		} else if (extension == "tex") {
			target = _get_texture_target(file);
			if (target != file) {
				texture_sources.insert(target);
			}
		} else if (extension_renames.has(extension)) {
			target = file.basename() + "." + extension_renames[extension];
		}

		String file_local = file.replace("res://", "");
		resource_replace_map[file] = target;
		resource_replace_map[file_local] = target.replace("res://", "");
	}

	DirAccess *directory = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

	if (directory->change_dir(p_path) != OK) {
		memdelete(directory);
		ERR_FAIL_V(ERR_CANT_OPEN);
	}

	int idx = 0;
	for (List<String>::Element *E = files.front(); E; E = E->next()) {

		String path = E->get();
		String extension = path.extension().to_lower();

		String target_path = p_path.plus_file(_replace_resource(path).replace("res://", ""));
		bool repack = false;

		progress.step(target_path.get_file(), idx++);

		print_line("-- Exporting file: " + target_path);
		current_path = path;

		if (directory->make_dir_recursive(target_path.get_base_dir()) != OK) {
			memdelete(directory);
			ERR_FAIL_V(ERR_CANT_CREATE);
		}

		ExportData resource_data;

		Error err;
		bool cont = false;
		if (extension == "tex" && _replace_resource(path) != path) {

			err = _export_texture(path, p_path);
			cont = true;
		} else if (text_bmfonts.has(path)) {

			err = directory->copy(path, target_path);
			cont = true;
		} else if (xml_extensions.has(extension)) {

			err = ResourceLoader::get_export_data(path, resource_data);
		} else if (text_extensions.has(extension)) {

			err = ResourceLoader::get_export_data(path, resource_data);
		} else if (binary_extensions.has(extension)) {

			err = ResourceLoader::get_export_data(path, resource_data);
		} else {

			if (convert_scripts && extension == "gd") {
				err = _convert_script(path, target_path, mark_converted_lines);
			} else {
				// single file, copy it
				err = directory->copy(path, target_path);

				// Keep the texture flags of images used directly (project settings and .flags files)
				if (err == OK && _is_image_extension(extension) && !texture_sources.has(path)) {
					_write_texture_import(target_path, path, ResourceFormatLoaderImage::load_image_flags(path));
				}
			}

			cont = true; // no longer needed to do anything, just copied the file!
		}

		if (err != OK) {
			memdelete(directory);
			ERR_EXPLAIN("Failed to export file: " + path);
			ERR_FAIL_V(err);
		}

		if (cont) {
			continue;
		}

		if (resource_data.nodes.size() == 0 && resource_data.resources[resource_data.resources.size() - 1].type == "PackedScene") {
			// must unpack a PackedScene
			_unpack_packed_scene(resource_data);
			repack = true;
		}

		_convert_resources(resource_data);

		if (repack) {

			if (xml_extensions.has(extension) || text_extensions.has(extension)) {
				_packed_to_text(resource_data);
			} else {
				_pack_packed_scene(resource_data);
			}
		}

		if (xml_extensions.has(extension) || text_extensions.has(extension)) {
			_save_text(target_path, resource_data);
		} else if (binary_extensions.has(extension)) {
			_save_binary(target_path, resource_data);
		}
	}

	memdelete(directory);
	_save_config(p_path);

	return OK;
}

EditorExportGodot3::EditorExportGodot3() {

	export_convert_scripts = true;
	export_mark_lines = false;

	int idx = 0;
	while (globals_renames[idx][0] != NULL) {

		globals_rename_map[globals_renames[idx][0]] = globals_renames[idx][1];
		idx++;
	}

	idx = 0;
	while (prop_renames[idx][0] != NULL) {

		prop_rename_map[prop_renames[idx][0]] = prop_renames[idx][1];
		idx++;
	}

	idx = 0;
	while (type_renames[idx][0] != NULL) {

		type_rename_map[type_renames[idx][0]] = type_renames[idx][1];
		idx++;
	}

	idx = 0;
	while (signal_renames[idx][0] != NULL) {

		signal_rename_map[signal_renames[idx][0]] = signal_renames[idx][1];
		idx++;
	}
}
