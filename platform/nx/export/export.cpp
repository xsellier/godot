#include "export.h"
#include "export_plugin.h"

void register_nx_exporter_types() {
	GDREGISTER_VIRTUAL_CLASS(EditorExportPlatformNX);
}

void register_nx_exporter() {
    Ref<EditorExportPlatformNX> platform;
	platform.instantiate();

	EditorExport::get_singleton()->add_export_platform(platform);
}