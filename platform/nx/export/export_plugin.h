#ifndef NX_EXPORT_PLUGIN_H
#define NX_EXPORT_PLUGIN_H

#include "core/os/os.h"
#include "core/io/file_access.h"
#include "editor/export/editor_export.h"
#include "editor/editor_node.h"
#include "editor/editor_settings.h"
#include "platform/nx/logo_svg.gen.h"
#include "scene/resources/texture.h"
#include "core/error/error_list.h"
#include "editor/editor_scale.h"

class EditorExportPlatformNX : public EditorExportPlatform {
    GDCLASS(EditorExportPlatformNX, EditorExportPlatform);

    Ref<ImageTexture> m_logo;
	String release_file_32;
	String release_file_64;
	String debug_file_32;
	String debug_file_64;

	struct Device {

		String name;
		String hardware;
		String connection;
		String serialNumber;
		String ipAddress;
		String status;
	};

	Vector<Device> devices;
	volatile bool devices_changed;
	Mutex device_lock;
	Thread device_thread;
	volatile bool quit_request;

	static void _device_poll_thread(void *ud) {

		EditorExportPlatformNX *ea = (EditorExportPlatformNX *)ud;

		// $NINTENDO_SDK_ROOT/Tools/CommandLineTools/ControlTarget.exe list-target
		String control_target_cmd = OS::get_singleton()->get_environment("NINTENDO_SDK_ROOT") + "/Tools/CommandLineTools/ControlTarget.exe";
		List<String> control_target_args;
		control_target_args.push_back("list-target");
		control_target_args.push_back("--csv");

		while (!ea->quit_request) {
			if (FileAccess::exists(control_target_cmd)) {

				String devices;
				int ec;
				OS::get_singleton()->execute(control_target_cmd, control_target_args, &devices, &ec);

				Vector<String> ds = devices.split("\n");
				Vector<Device> ndevices;
				for (int i = 1; i < ds.size(); i++) {
					// Skipped first line (headers)
					// rest is CSV "Name","Hardware","Connection","SerialNumber","IPAddress","Status"
					Vector<String> fields = ds[i].split(",");
					if (fields.size() != 6)
						break;
					Device d;
					d.name = fields[0];
					d.hardware = fields[1];
					d.connection = fields[2];
					d.serialNumber = fields[3];
					d.ipAddress = fields[4];
					d.status = fields[5];
					ndevices.push_back(d);
				}

				ea->device_lock.lock();

				bool different = false;

				if (ea->devices.size() != ndevices.size()) {

					different = true;
				} else {

					for (int i = 0; i < ea->devices.size(); i++) {

						if (ea->devices[i].name != ndevices[i].name) {
							different = true;
							break;
						}
					}
				}

				if (different) {
					ea->devices = ndevices;
					ea->devices_changed = true;
				}

				ea->device_lock.unlock();
			}

			uint64_t sleep = OS::get_singleton()->is_in_low_processor_usage_mode() ? 1000 : 100;
			uint64_t wait = 3000000;
			uint64_t time = OS::get_singleton()->get_ticks_usec();
			while (OS::get_singleton()->get_ticks_usec() - time < wait) {
				OS::get_singleton()->delay_usec(1000 * sleep);
				if (ea->quit_request)
					break;
			}
		}
	}

protected:
    void get_preset_features(const Ref<EditorExportPreset> &p_preset, List<String> *r_features) const override;
    void get_export_options(List<ExportOption> *r_options) const override;


public:
    EditorExportPlatformNX();
    ~EditorExportPlatformNX();

    String get_os_name() const override { return "nx"; }
    String get_name() const override { return "Nintendo Switch"; }
    Ref<Texture2D> get_logo() const override;

	bool poll_export() override;
	int get_options_count() const override;
	String get_options_tooltip() const override;
	String get_option_label(int p_index) const override;
	String get_option_tooltip(int p_index) const override;

	Error run(const Ref<EditorExportPreset> &p_preset, int p_device, int p_debug_flags);

    bool has_valid_export_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error, bool &r_missing_templates, bool p_debug = false) const override;
	bool has_valid_project_configuration(const Ref<EditorExportPreset> &p_preset, String &r_error) const override;

    List<String> get_binary_extensions(const Ref<EditorExportPreset> &p_preset) const override;
    Error export_project(const Ref<EditorExportPreset> &p_preset, bool p_debug, const String &p_path, int p_flags = 0) override;
    void get_platform_features(List<String> *r_features) const override;
    void resolve_platform_feature_priorities(const Ref<EditorExportPreset> &p_preset, HashSet<String> &p_features) override;

	Error copy_file(const String &src_path, const String &dst_path) const;
};


#endif