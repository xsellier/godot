#ifndef VULKAN_DEVICE_NX_H
#define VULKAN_DEVICE_NX_H

#include "drivers/vulkan/vulkan_context.h"
#include <nn/vi.h>


class VulkanContextNX : public VulkanContext {
	virtual const char *_get_platform_surface_extension() const;

public:
	int window_create(DisplayServer::WindowID p_window_id, DisplayServer::VSyncMode p_vsync_mode, nn::vi::Layer* p_layer, int p_width, int p_height);

	VulkanContextNX();
	~VulkanContextNX();
};

#endif // VULKAN_DEVICE_NX_H