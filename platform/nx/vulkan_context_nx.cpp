#include "vulkan_context_nx.h"

#ifndef VK_USE_PLATFORM_VI_NN
#define VK_USE_PLATFORM_VI_NN
#endif

#include <nn/nn_Assert.h>
#include <nn/nn_Log.h>

#include "drivers/vulkan/godot_vulkan.h"

const char *VulkanContextNX::_get_platform_surface_extension() const {
	return VK_NN_VI_SURFACE_EXTENSION_NAME;
}

VulkanContextNX::SurfaceID VulkanContextNX::surface_create(const void *p_platform_data) {
	const WindowPlatformData *wpd = (const WindowPlatformData *)(p_platform_data);

	VkViSurfaceCreateInfoNN createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
    createInfo.pNext = NULL;
    createInfo.flags = 0;

	VkResult result;
    nn::vi::NativeWindowHandle nativeWindow;
    nn::vi::GetNativeWindow(&nativeWindow, wpd->viLayer);
    createInfo.window = nativeWindow;

    VkSurfaceKHR surfaceKhr;

    PFN_vkCreateViSurfaceNN vkCreateViSurfaceNN;
    vkCreateViSurfaceNN = reinterpret_cast<PFN_vkCreateViSurfaceNN>(vkGetInstanceProcAddr(instance_get(), "vkCreateViSurfaceNN"));
	
	result = vkCreateViSurfaceNN(instance_get(), &createInfo, nullptr, &surfaceKhr);
    NN_ASSERT_EQUAL(result, VK_SUCCESS);

	Surface *surface = memnew(Surface);
	surface->vk_surface = surfaceKhr;
	return SurfaceID(surface);
}

VulkanContextNX::VulkanContextNX() {
}

VulkanContextNX::~VulkanContextNX() {
}