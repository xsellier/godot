#include "export.h"
#include "export_plugin.h"
#include "publishing_metadata_nx.h"

void register_nx_exporter_types() {
	GDREGISTER_VIRTUAL_CLASS(EditorExportPlatformNX);
	GDREGISTER_CLASS(LocalizationMetadataNX);
	GDREGISTER_CLASS(RatingMetadataNX);
	GDREGISTER_CLASS(PublishingMetadataNX);
}

void register_nx_exporter() {
    Ref<EditorExportPlatformNX> platform;
	platform.instantiate();

	EditorExport::get_singleton()->add_export_platform(platform);
}