#include "export_plugin.h"
#include "editor/editor_paths.h"
#include "core/config/project_settings.h"

#include "modules/modules_enabled.gen.h"
#ifdef MODULE_SVG_ENABLED
#include "modules/svg/image_loader_svg.h"
#endif

EditorExportPlatformNX::EditorExportPlatformNX() 
{
#ifdef MODULE_SVG_ENABLED
	Ref<Image> img = memnew(Image);
	const bool upsample = !Math::is_equal_approx(Math::round(EDSCALE), EDSCALE);

	ImageLoaderSVG img_loader;
	img_loader.create_image_from_string(img, _nx_logo_svg, EDSCALE, upsample, false);
	m_logo = ImageTexture::create_from_image(img);

#endif

    release_file_32 = "godot_nx.nx.template_release.arm32.nss";
    debug_file_32   = "godot_nx.nx.template_debug.arm32.nss";
    release_file_64 = "godot_nx.nx.template_release.arm64.nss";
    debug_file_64   = "godot_nx.nx.template_debug.arm64.nss";

	devices_changed = true;
	quit_request = false;
	device_thread.start(_device_poll_thread, this);
}

EditorExportPlatformNX::~EditorExportPlatformNX()
{
	quit_request = true;
	device_thread.wait_to_finish();
}

void EditorExportPlatformNX::get_export_options(List<ExportOption> *r_options) const
{

	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "custom_package/debug", PROPERTY_HINT_GLOBAL_FILE, "*.nss"), ""));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "custom_package/release", PROPERTY_HINT_GLOBAL_FILE, "*.nss"), ""));

	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/name", PROPERTY_HINT_PLACEHOLDER_TEXT, "Game Name"), ""));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/identifier", PROPERTY_HINT_PLACEHOLDER_TEXT, "0x01004b9000490000"), "0x01004b9000490000"));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/publisher"), "Publisher"));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/icon", PROPERTY_HINT_FILE, "*.bmp,*.jpg"), ""));
	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "application/version"), "1.0.0"));

	r_options->push_back(ExportOption(PropertyInfo(Variant::STRING, "command_line/extra_args"), ""));

	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "binary_format/64_bits"), true));

	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "texture_format/s3tc"), true));
	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "texture_format/etc"), false));
	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "texture_format/etc2"), false));
	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "debug/renderdoc"), false));
	r_options->push_back(ExportOption(PropertyInfo(Variant::BOOL, "debug/log_verbose"), false));
}

Error EditorExportPlatformNX::execute_cmd(const String &p_path, const List<String> &p_arguments, bool p_log_verbose) const {
	String pipe;
	if (p_log_verbose) {
		print_line("[EditorExportPlatformNX] Executing:", p_path);
	}
	Error err = OS::get_singleton()->execute(p_path, p_arguments, &pipe);
	PackedStringArray lines = pipe.split("\n\r", false);
	String error_string = "Error:";
	for (int i = 0; i < lines.size(); i++) {
		String line = lines[i];
		if (error_string.is_subsequence_of(line)) {
			print_error(line.strip_edges());
		} else if (p_log_verbose) {
			print_line(line.strip_edges());
		}
	}
	return err;
}

void EditorExportPlatformNX::get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) const {
    if (p_preset->get("texture_format/s3tc")) {
		r_features->push_back("s3tc");
	}
	if (p_preset->get("texture_format/etc")) {
		r_features->push_back("etc");
	}
	if (p_preset->get("texture_format/etc2")) {
		r_features->push_back("etc2");
	}

	r_features->push_back("64");
}

Ref<Texture2D> EditorExportPlatformNX::get_logo() const
{
    return m_logo;
}

bool EditorExportPlatformNX::has_valid_export_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates, bool p_debug) const
{
    String err;
	bool valid = true;
	bool use64 = p_preset->get("binary_format/64_bits");
	
#ifndef WINDOWS_ENABLED
    // Can only export on windows
    valid = false;
    err += "Export to Nintendo Switch only possible on Windows";
    r_error = err;
    r_missing_templates = !valid;
    return valid;
#endif

	// Must have valid:
	String name = p_preset->get("application/name");
	if (name.is_empty()) {
		err += "An Application name must be provided";
		valid = false;
	}
	String id = p_preset->get("application/identifier");
	if (id.is_empty()) {
		err += "An Application ID must be provided";
		valid = false;
	}
	if (!id.is_valid_hex_number(true)) {
		err += "Application ID must be hex number (0x000001)";
		valid = false;
	}
	String publisher = p_preset->get("application/publisher");
	if (publisher.is_empty()) {
		err += "Publisher name must be provided";
		valid = false;
	}
	String icon = p_preset->get("application/icon");
	if (icon.is_empty()) {
		err += "Icon is required for packaging";
		valid = false;
	}
	String version = p_preset->get("application/version");
	if (version.is_empty()) {
		err += "Version must be provided";
		valid = false;
	}

	if (use64 && (!exists_export_template(debug_file_64, &err) || !exists_export_template(release_file_64, &err))) {
		valid = false;
		r_missing_templates = true;
	}

	if (!use64 && (!exists_export_template(debug_file_32, &err) || !exists_export_template(release_file_32, &err))) {
		valid = false;
		r_missing_templates = true;
	}

	// Must have Nintendo SDK Installed
	String sdkPath = OS::get_singleton()->get_environment("NINTENDO_SDK_ROOT");
	if (sdkPath.is_empty()) {
		err += "NINTENDO_SDK_ROOT environment variable not set.";
		valid = false;
	}

	if (!err.is_empty())
	    r_error = err;

    return valid;
}

bool EditorExportPlatformNX::has_valid_project_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error) const 
{
	String err;
	bool valid = true;

	// TODO Check for Nintendo Switch SDK and tools to be available before exporting

	if (!err.is_empty()) {
		r_error = err;
	}
	return valid;
}

List<String> EditorExportPlatformNX::get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const
{
	List<String> extensions;
	extensions.push_back("nsp");
    return extensions;
}

Error EditorExportPlatformNX::export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags)
{
	bool log_verbose = p_preset->get("debug/log_verbose");
	String terminate_cmd = OS::get_singleton()->get_environment("NINTENDO_SDK_ROOT") + "/Tools/CommandLineTools/ControlTarget.exe";
	List<String> terminate_args;
	terminate_args.push_back("terminate");
	Error err_cmd = execute_cmd(terminate_cmd, terminate_args, log_verbose);
	ERR_FAIL_COND_V(err_cmd, err_cmd);
	
	ExportNotifier notifier(*this, p_preset, p_debug, p_path, p_flags);

	EditorProgress ep("export", "Exporting for Nintendo Switch", 5, true);
	
	String src_pkg_name;
	String dest_dir = p_path.get_base_dir() + "/";
	String binary_name = p_path.get_file().get_basename();
	// binary_name might have "." at end
	int pos = binary_name.rfind(".");
	if (pos > 0)
		binary_name = binary_name.substr(0, pos);

	String sdkPath = OS::get_singleton()->get_environment("NINTENDO_SDK_ROOT");

	// Get the right .nss template file to start packaing from
	if (p_debug)
		src_pkg_name = p_preset->get("custom_package/debug");
	else
		src_pkg_name = p_preset->get("custom_package/release");

	bool use64 = p_preset->get("binary_format/64_bits");

	if (src_pkg_name == "") {
	 	String err;
		if (p_debug) {
			if (use64)
				src_pkg_name = find_export_template(debug_file_64, &err);
			else
				src_pkg_name = find_export_template(debug_file_32, &err);
		} else {
			if (use64)
				src_pkg_name = find_export_template(release_file_64, &err);
			else
				src_pkg_name = find_export_template(release_file_32, &err);
		}
		
	 	if (src_pkg_name == "") {
	 		EditorNode::add_io_error(err);
	 		return ERR_FILE_NOT_FOUND;
	 	}
	}

	String cmdline = p_preset->get("command_line/extra_args");
	Vector<String> cl = cmdline.strip_edges().split(" ");
	for (int i = 0; i < cl.size(); i++) {
		if (cl[i].strip_edges().length() == 0) {
			cl.remove_at(i);
			i--;
		}
	}

	gen_export_flags(cl, p_flags);

	// Create a new folder to create package in (removing old one if necessary)
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);

    String current_dir = da->get_current_dir();

    // remove leftovers from last export so they don't interfere
    // in case some files are no longer needed
    if (da->change_dir(dest_dir + binary_name) == OK) {
        da->erase_contents_recursive();
    }

    da->change_dir(current_dir);

    Error err;
    if (!da->dir_exists(dest_dir + binary_name)) {
        err = da->make_dir(dest_dir + binary_name);
        if (err) {
            return err;
        }
    }
    da->change_dir(dest_dir + binary_name);
    // Create code region dir
    err = da->make_dir("code");
    if (err) {
        return err;
    }
    // Create data region dir
    err = da->make_dir("data");
    if (err) {
        return err;
    }

	// Package datafile from project
	if (ep.step("Making .pck", 0))
		return ERR_SKIP;
	String pack_path = dest_dir + binary_name + "/data/" + "project" + ".pck";
	Vector<SharedObject> libraries;
	err = save_pack(p_preset, p_debug, pack_path, &libraries);
	if (err != OK)
		return err;

	// Create .nso file from .nss
	if (ep.step("Generating NSO", 1))
		return ERR_SKIP;
	String makenso_command = sdkPath + "/Tools/CommandLineTools/MakeNso/MakeNso.exe";
	List<String> makenso_args;
	makenso_args.push_back(src_pkg_name);
	//makenso_args.push_back(dest_dir + binary_name + "/code/" + binary_name + ".nso");
	makenso_args.push_back(dest_dir + binary_name + "/code/" + "main");
	err = execute_cmd(makenso_command, makenso_args, log_verbose);
	ERR_FAIL_COND_V(err, err);

	// Special preprocessing for application icon
	String appIconSource = p_preset->get("application/icon");
	String appIconTarget = appIconSource.get_file();
	copy_file(appIconSource, dest_dir + binary_name + "/" + appIconTarget);

	// Create .nmeta file based on project properties
	if (ep.step("Generating .nmeta information", 2))
		return ERR_SKIP;
	String nmetaTemplate;
	if (use64)
		nmetaTemplate = sdkPath + "/Resources/SpecFiles/Application.aarch64.lp64.nmeta";
	else
		nmetaTemplate = sdkPath + "/Resources/SpecFiles/Application.arm.ilp32.nmeta";

	Ref<FileAccess> f = FileAccess::open(nmetaTemplate, FileAccess::READ, &err);
	if (err != OK) {
		ERR_PRINT("Can't read '" + nmetaTemplate + "'.");
		return err;
	};
	Vector<uint8_t> data;
	data.resize(f->get_length());
	f->get_buffer(data.ptrw(), data.size());
    f.unref(); // close

	String nmetaString;
	String newNmetaString;
	nmetaString.parse_utf8((const char *)data.ptr(), data.size());
	Vector<String> lines = nmetaString.split("\n");
	for (int i = 0; i < lines.size(); i++) {
		if (lines[i].find("<Name>Application</Name>") != -1) {
			newNmetaString += lines[i].replace("Application", binary_name) + "\n";
		} else if (lines[i].find("<ApplicationId>0x01004b9000490000</ApplicationId>") != -1) {
			newNmetaString += lines[i].replace("0x01004b9000490000", p_preset->get("application/identifier")) + "\n";
		} else if (lines[i].find("<Name>Name</Name>") != -1) {
			String app = ">" + (String)p_preset->get("application/name") + "<";
			newNmetaString += lines[i].replace(">Name<", app) + "\n";
		} else if (lines[i].find("<Publisher>Publisher</Publisher>") != -1) {
			String publisher = ">" + (String)p_preset->get("application/publisher") + "<";
			newNmetaString += lines[i].replace(">Publisher<", publisher) + "\n";
		} else if (lines[i].find("<IconPath>./NintendoSDK_Application.bmp</IconPath>") != -1) {
			newNmetaString += lines[i].replace("./NintendoSDK_Application.bmp", appIconTarget) + "\n";
		} else if (lines[i].find("<DisplayVersion>1.0.0</DisplayVersion>") != -1) {
			newNmetaString += lines[i].replace("1.0.0", p_preset->get("application/version")) + "\n";
/*			// Not currently used.
			// Added in <Application>, after <DisplayVersion>
			// Cache partition is only used when "read_only_cache" is off.
			// Using the cache partition requires a special permission by Nintendo.
			if(GLOBAL_GET("rendering/shader_compiler/shader_cache/enabled") && !GLOBAL_GET("rendering/shader_compiler/shader_cache/read_only")){
				// FIXME: shader_cache_size_mb does not exist in Godot 4.x. Hardcoded 128MB
				// int64_t shader_cache_size = (int)GLOBAL_GET("rendering/gles3/shaders/shader_cache_size_mb.mobile") * 1024 * 1024;
				int64_t shader_cache_size = 128 * 1024 * 1024;
				String hex = String::num_int64(shader_cache_size, 16, true);
				String value;
				for(int i = 0; i<16-hex.length(); i++)
					value +="0";
				value += hex;
				newNmetaString += "<CacheStorageSize>0x"+value+"</CacheStorageSize>\n";
				newNmetaString += "<CacheStorageJournalSize>0x"+value+"</CacheStorageJournalSize>\n";
			}
*/
#ifdef NX_USE_PRESELECTED_USER
			// Added in <Application>, after <DisplayVersion> or <CacheStorageJournalSize>
			newNmetaString += "<StartupUserAccount>Required</StartupUserAccount>\n";
			newNmetaString += "<UserAccountSwitchLock>Enable</UserAccountSwitchLock>\n";
#endif
		} else {
			newNmetaString += lines[i] + "\n";
		}
	}

	// ### Enable cache if shader cache is enabled

	// Write out newNmetaString
	String nmetaOutputFile = dest_dir + binary_name + "/" + binary_name + ".nmeta";
	f = FileAccess::open(nmetaOutputFile, FileAccess::WRITE, &err);
	if (err != OK) {
		ERR_PRINT("Can't write '" + nmetaOutputFile + "'.");
		return err;
	};
	f->store_string(newNmetaString);
    f.unref(); // close

	// Create .npdm file from generated .nmeta
	if (ep.step("Writing .npdm", 3))
		return ERR_SKIP;
	String makemeta_command = sdkPath + "/Tools/CommandLineTools/MakeMeta/MakeMeta.exe";
	List<String> makemeta_args;
	makemeta_args.push_back("--desc");
	makemeta_args.push_back(sdkPath + "/Resources/SpecFiles/Application.desc");
	makemeta_args.push_back("--meta");
	makemeta_args.push_back(nmetaOutputFile);
	makemeta_args.push_back("-o");
	//makemeta_args.push_back(dest_dir + binary_name + "/code/" + binary_name + ".npdm");
	makemeta_args.push_back(dest_dir + binary_name + "/code/" + "main.npdm");
	err = execute_cmd(makemeta_command, makemeta_args, log_verbose);
	ERR_FAIL_COND_V(err, err);

	// Fill Code Region Directory
	if (ep.step("Filling Code Region", 4))
		return ERR_SKIP;
	// *.nso -> main (done already)
	// *.npdm -> main.npdm (done already)
	String lib_dir;
	if (use64)
		lib_dir = sdkPath + "/Libraries/NX-NXFP2-a64";
	else
		lib_dir = sdkPath + "/Libraries/NX-NXFP2-a32";

	if (p_debug)
		lib_dir += "/Develop/";
	else
		lib_dir += "/Release/";
	
	bool useRenderDoc = p_preset->get("debug/renderdoc");

	// nnrtld.nso -> rtld
	copy_file(lib_dir + "nnrtld.nso", dest_dir + binary_name + "/code/" + "rtld");
	// nnSdk.nso -> skd
	copy_file(lib_dir + "nnSdk.nso", dest_dir + binary_name + "/code/" + "sdk");
	// opengl.nso -> subsdk0
    // vulkan.nso -> subsdk1
	if (!useRenderDoc) {
		copy_file(lib_dir + "opengl.nso", dest_dir + binary_name + "/code/" + "subsdk0");
        copy_file(lib_dir + "vulkan.nso", dest_dir + binary_name + "/code/" + "subsdk1");
    } else {
		copy_file(lib_dir + "vulkanOpenGlDebug.nso", dest_dir + binary_name + "/code/" + "subsdk0");
    }

	// Generate .nsp Application Image
	if (ep.step("Generate final .nsp Application Image", 5))
		return ERR_SKIP;
	String authoring_tool_command = sdkPath + "/Tools/CommandLineTools/AuthoringTool/AuthoringTool.exe";
	List<String> creatensp_args;
	creatensp_args.push_back("creatensp");
	creatensp_args.push_back("-o");
	creatensp_args.push_back(dest_dir + binary_name + ".nsp");
	creatensp_args.push_back("--desc");
	creatensp_args.push_back(sdkPath + "/Resources/SpecFiles/Application.desc");
	creatensp_args.push_back("--meta");
	creatensp_args.push_back(nmetaOutputFile);
	creatensp_args.push_back("--type");
	creatensp_args.push_back("Application");
	creatensp_args.push_back("--program");
	creatensp_args.push_back(dest_dir + binary_name + "/code/");
	creatensp_args.push_back(dest_dir + binary_name + "/data/");
	err = execute_cmd(authoring_tool_command, creatensp_args, log_verbose);
	ERR_FAIL_COND_V(err, err);

    return OK;
}

void EditorExportPlatformNX::get_platform_features(List<String> *r_features) const
{

}

void EditorExportPlatformNX::resolve_platform_feature_priorities(const Ref<EditorExportPreset> &p_preset, HashSet<String> &p_features)
{

}

bool EditorExportPlatformNX::poll_export()
{
	bool dc = devices_changed;
	if (dc) {
		// don't clear unless we're reporting true, to avoid race
		devices_changed = false;
	}
	return dc;
}

int EditorExportPlatformNX::get_options_count() const
{
	device_lock.lock();
	int dc = devices.size();
	device_lock.unlock();

	return dc;
}

String EditorExportPlatformNX::get_options_tooltip() const
{
	return TTR("Select device from the list");
}

String EditorExportPlatformNX::get_option_label(int p_index) const
{
	ERR_FAIL_INDEX_V(p_index, devices.size(), "");
	device_lock.lock();
	String s = devices[p_index].name;
	device_lock.unlock();
	return s;
}

String EditorExportPlatformNX::get_option_tooltip(int p_index) const
{
	ERR_FAIL_INDEX_V(p_index, devices.size(), "");
	device_lock.lock();
	String s = devices[p_index].ipAddress;
		if (devices.size() == 1) {
		s = devices[p_index].name + "\n\n" + s;
	}
	device_lock.unlock();
	return s;
}

Error EditorExportPlatformNX::run(const Ref<EditorExportPreset> &p_preset, int p_device, int p_debug_flags)
{
	ERR_FAIL_INDEX_V(p_device, devices.size(), ERR_INVALID_PARAMETER);

	Vector<String> device_arguments;
	gen_export_flags(device_arguments, p_debug_flags);

	String can_export_error;
	bool can_export_missing_templates;
	if (!can_export(p_preset, can_export_error, can_export_missing_templates)) {
		EditorNode::add_io_error(can_export_error);
		return ERR_UNCONFIGURED;
	}
	EditorProgress ep("run", "Running on " + devices[p_device].name, 1);
	String path = EditorPaths::get_singleton()->get_cache_dir().path_join("tmp_export.nsp");
	if (ep.step("Exporting .NSP Application Image...", 0))
		return ERR_SKIP;
	Error err = export_project(p_preset, true, path, p_debug_flags);
	if (err) {
		return err;
	}

	if (ep.step("Running on device...", 1))
		return ERR_SKIP;
	String run_on_target_cmd = OS::get_singleton()->get_environment("NINTENDO_SDK_ROOT") + "/Tools/CommandLineTools/RunOnTarget.exe";
	List<String> run_on_target_args;
	run_on_target_args.push_back("--no-wait");
	run_on_target_args.push_back("--target");
	run_on_target_args.push_back(get_option_label(p_device));;
	run_on_target_args.push_back(path);
	// Add custom command line arguments here
	if (device_arguments.size() > 0) {
		run_on_target_args.push_back("--");
		for (int i = 0; i < device_arguments.size(); ++i)
			run_on_target_args.push_back(device_arguments[i]);
	}

	err = OS::get_singleton()->execute(run_on_target_cmd, run_on_target_args);
	ERR_FAIL_COND_V(err, err);
	return OK;

	// $NINTENDO_SDK_ROOT/Tools/CommandLineTools/RunOnTarget.exe
	// [--env=<arg>] [--no-wait] [--success-timeout=<arg>]
    //       [--failure-timeout=<arg>]
    //       [--pattern-success-exit=<arg>]...
    //       [--pattern-failure-exit=<arg>]...
    //       [--pattern-reset-success-timeout=<arg>]...
    //       [--pattern-reset-failure-timeout=<arg>]...
    //       [--pattern-not-found-failure=<arg>]... [--filter=<arg>]
    //       [--reset] [--connect-timeout=<arg>] [-v] [--enable-aslr]
    //       [--disable-aslr] [--working-directory] [--output-result=<arg>] <Program> [--] [<Arguments>...]
	// Also --target for device name or ip address
}

Error EditorExportPlatformNX::copy_file(const String &src_path, const String &dst_path) const
{
	// Read file in
	if (!FileAccess::exists(src_path)) {
		ERR_PRINT("Can't read '" + src_path + "'.");
		return ERR_DOES_NOT_EXIST;
	}
	Vector<uint8_t> data = FileAccess::get_file_as_bytes(src_path);

	// Write file to new location
    Error err;
	Ref<FileAccess> f = FileAccess::open(dst_path, FileAccess::WRITE, &err);
	if (err != OK) {
		ERR_PRINT("Can't write '" + dst_path + "'.");
		return err;
	};

	f->store_buffer(data.ptr(), data.size());

	return OK;
}
