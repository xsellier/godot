/*************************************************************************/
/*  editor_export_godot3.h                                               */
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

#ifndef EDITOR_EXPORT_GODOT3_H
#define EDITOR_EXPORT_GODOT3_H

#include "editor/editor_file_system.h"
#include "io/export_data.h"
#include "scene/main/node.h"

class EditorExportGodot3 {

	Map<String, int> pack_names;
	HashMap<Variant, int, VariantHasher> pack_values;

	int _pack_name(const String &p_name) {
		if (pack_names.has(p_name)) {
			return pack_names[p_name];
		}

		int idx = pack_names.size();
		pack_names[p_name] = idx;
		return idx;
	}

	int _pack_value(const Variant &p_value) {
		if (pack_values.has(p_value)) {
			return pack_values[p_value];
		}

		int idx = pack_values.size();
		pack_values[p_value] = idx;
		return idx;
	}

	Map<String, String> globals_rename_map;
	Map<String, String> prop_rename_map;
	Map<String, String> type_rename_map;
	Map<String, String> signal_rename_map;

	Map<String, String> resource_replace_map;

	String _replace_resource(const String &p_res) {
		if (resource_replace_map.has(p_res))
			return resource_replace_map[p_res];
		else
			return p_res;
	}

	Error _get_property_as_text(const Variant &p_variant, String &p_string);

	void _save_text(const String &p_path, ExportData &resource);

	void _save_binary_property(const Variant &p_property, FileAccess *f);

	void _save_binary(const String &p_path, ExportData &resource);
	void _save_config(const String &p_path);

	Map<String, String> instanced_types;

	void _packed_to_text(ExportData &resource);
	Set<String> written_imports;

	int _get_next_resource_index(const ExportData &resource) const;
	String _add_sub_resource(ExportData &resource, const String &p_type, const List<ExportData::PropertyData> &p_props);
	bool _image_to_resource(const Image &p_image, ExportData::ResourceData &r_resource);
	void _convert_images(ExportData &resource);
	String _get_instanced_type(const ExportData &resource, int p_node);
	String _get_node_type(const ExportData &resource, int p_node);
	void _get_node_parents(const ExportData &resource, Vector<int> &r_parents);
	int _get_connection_source(const ExportData &resource, const ExportData::Connection &p_connection);
	void _convert_connections(ExportData &resource);
	void _convert_particles_2d(ExportData &resource, int p_node);
	void _convert_blend_mode(ExportData &resource, int p_node);
	void _convert_button_groups(ExportData &resource);
	void _convert_dependencies(ExportData &resource);

	String _get_texture_target(const String &p_path);
	Error _export_texture(const String &p_path, const String &p_export_path);
	void _write_texture_import(const String &p_export_path, const String &p_source, uint32_t p_flags);
	bool _is_text_bmfont(const String &p_path);
	String _get_script_base_type(const String &p_path, int p_depth = 0);

	void _rename_properties(const String &p_type, List<ExportData::PropertyData> *p_props);
	void _add_new_properties(const String &p_type, List<ExportData::PropertyData> *p_props);
	void _convert_resources(ExportData &resource);
	void _unpack_packed_scene(ExportData &resource);
	void _pack_packed_scene(ExportData &resource);

	struct ScriptContext {
		String base_type;
		bool is_control;
		bool is_kinematic;
		bool is_raycast;
		bool is_spatial;
		bool is_color_picker;
		bool needs_review;
		Set<String> http_vars;
		Set<String> moved_receivers;
		Map<String, String> local_renames;
		String path;
		String line_indent;
		String indent_unit;
		bool cyclic_preload;
		Vector<String> prefix_statements;
		Vector<String> postfix_statements;
		bool prefix_allowed;
		bool postfix_allowed;
		bool in_static;
		Set<String> declared_functions;
		Set<String> regex_vars;
		Set<String> member_vars;
		Set<String> function_locals;
		Set<String> image_vars;
		Set<String> extra_members;
		bool uses_kinematic_collision;

		ScriptContext() {
			is_control = false;
			is_kinematic = false;
			is_raycast = false;
			is_spatial = false;
			is_color_picker = false;
			needs_review = false;
			uses_kinematic_collision = false;
			prefix_allowed = true;
			postfix_allowed = true;
			in_static = false;
			cyclic_preload = false;
		}
	};

	Set<String> user_functions;
	Set<String> engine_methods;

	struct ClassNameData {
		String name;
		String extra;
		String base;
	};

	Map<String, ClassNameData> class_names;
	Map<String, String> function_renames;

	String _get_quoted_path(const String &p_text, const String &p_script_path);
	void _find_class_names(const List<String> &p_files);
	String current_path;
	bool export_convert_scripts;
	bool export_mark_lines;

	String _get_script_base_type_from_lines(const String &p_path, const Vector<String> &p_lines, int p_depth = 0);
	Vector<String> _convert_script_lines(const String &p_path, const Vector<String> &p_lines, bool mark_converted_lines);
	Map<String, Set<String> > script_owners;
	Map<String, Set<String> > animation_finished_methods;

	void _find_script_owners(const List<String> &p_files);
	Set<String> cyclic_preloads;
	Map<String, Map<String, String> > script_member_renames;
	Map<String, Set<String> > godot3_member_map;
	bool _is_godot3_member(const String &p_base_type, const String &p_name);
	void _find_member_renames(const List<String> &p_files);
	void _find_cyclic_preloads(const List<String> &p_files);
	Set<String> user_members;

	void _find_user_functions(const List<String> &p_files);
	String _convert_script_code(const String &p_code, Vector<String> &r_strings, ScriptContext &p_ctx);
	bool _convert_script_call(const String &p_name, const String &p_receiver, const Vector<String> &p_args, bool p_statement, const String &p_before, Vector<String> &r_strings, ScriptContext &p_ctx, String &r_replacement);
	Error _convert_script(const String &p_path, const String &p_target_path, bool mark_converted_lines);

	void _find_files(EditorFileSystemDirectory *p_dir, List<String> *r_files);

public:
	Error export_godot3(const String &p_path, bool convert_scripts, bool mark_converted_lines);

	EditorExportGodot3();
};

#endif // EDITOR_EXPORT_GODOT3_H
