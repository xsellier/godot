#include "vulkan_context_nx.h"

#ifndef VK_USE_PLATFORM_VI_NN
#define VK_USE_PLATFORM_VI_NN
#endif

#include <nn/nn_Assert.h>
#include <nn/nn_Log.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_vi.h>

const char *VulkanContextNX::_get_platform_surface_extension() const {
	return VK_NN_VI_SURFACE_EXTENSION_NAME;
}

int VulkanContextNX::window_create(DisplayServer::WindowID p_window_id, DisplayServer::VSyncMode p_vsync_mode, nn::vi::Layer* p_layer, int p_width, int p_height) {
    VkViSurfaceCreateInfoNN createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN;
    createInfo.pNext = NULL;
    createInfo.flags = 0;

    VkResult result;
    nn::vi::NativeWindowHandle nativeWindow;
    nn::vi::GetNativeWindow(&nativeWindow, p_layer);
    createInfo.window = nativeWindow;
 
    VkSurfaceKHR surfaceKhr;

    PFN_vkCreateViSurfaceNN vkCreateViSurfaceNN;
    vkCreateViSurfaceNN = reinterpret_cast<PFN_vkCreateViSurfaceNN>(vkGetInstanceProcAddr(get_instance(), "vkCreateViSurfaceNN"));

    result = vkCreateViSurfaceNN(get_instance(), &createInfo, nullptr, &surfaceKhr);
    NN_ASSERT_EQUAL(result, VK_SUCCESS);
    return _window_create(p_window_id, p_vsync_mode, surfaceKhr, p_width, p_height);
}

VulkanContextNX::VulkanContextNX() {
}

VulkanContextNX::~VulkanContextNX() {
}