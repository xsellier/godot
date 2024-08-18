#ifndef VULKAN_DEVICE_NX_H
#define VULKAN_DEVICE_NX_H

#include "drivers/vulkan/rendering_context_driver_vulkan.h"
#include <nn/vi.h>

struct ANativeWindow;

class VulkanContextNX : public RenderingContextDriverVulkan {
	virtual const char *_get_platform_surface_extension() const override final;

protected:
	SurfaceID surface_create(const void *p_platform_data) override final;
	
public:
	struct WindowPlatformData {
		ANativeWindow *window;
		nn::vi::Layer *viLayer;
	};

	VulkanContextNX();
	~VulkanContextNX();
};

#endif // VULKAN_DEVICE_NX_H