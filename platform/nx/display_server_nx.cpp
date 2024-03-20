#include "display_server_nx.h"

#include "core/os/memory.h"
#include "core/os/os.h"

#include <nn/util/util_BitUtil.h>
#include <nv/nv_MemoryManagement.h>
#include <nn/gll.h>

#include <stdlib.h>

const size_t gfxMemSize = 8 * 1024 * 1024;

static void *NvAllocateFunction(size_t size, size_t alignment, void *userPtr)
{
    return ::aligned_alloc(alignment, nn::util::align_up(size, alignment));
}

static void NvFreeFunction(void *addr, void *userPtr)
{
    ::free(addr);
}

static void *NvReallocateFunction(void *addr, size_t newSize, void *userPtr)
{
    return ::realloc(addr, newSize);
}

static void *NvDevtoolsAllocateFunction(size_t size, size_t alignment, void *userPtr)
{
    return ::aligned_alloc(alignment, nn::util::align_up(size, alignment));
}

static void NvDevtoolsFreeFunction(void *addr, void *userPtr)
{
    ::free(addr);
}

static void *NvDevtoolsReallocateFunction(void *addr, size_t newSize, void *userPtr)
{
    return ::realloc(addr, newSize);
}

DisplayServerNX *DisplayServerNX::get_singleton() {
    return static_cast<DisplayServerNX *>(DisplayServer::get_singleton());
}

void DisplayServerNX::register_nx_driver() {
    register_create_function("nx", create_func, get_rendering_drivers_func);
}

DisplayServer *DisplayServerNX::create_func(const String &p_rendering_driver, WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Error &r_error) {
    return memnew(DisplayServerNX(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_position, p_resolution, p_screen, r_error));
}

Vector<String> DisplayServerNX::get_rendering_drivers_func() {
    Vector<String> drivers;

	drivers.push_back("vulkan");

	return drivers;
}

DisplayServerNX::DisplayServerNX(const String &p_rendering_driver, DisplayServer::WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Error &r_error)
{
    rendering_driver = p_rendering_driver;

    // platform init

    nv::SetGraphicsAllocator(NvAllocateFunction, NvFreeFunction, NvReallocateFunction, nullptr);
    nv::SetGraphicsDevtoolsAllocator(NvDevtoolsAllocateFunction, NvDevtoolsFreeFunction, NvDevtoolsReallocateFunction, nullptr);

    // graphics init
    void *graphicsHeap = ::malloc(gfxMemSize);
    nv::InitializeGraphics(graphicsHeap, gfxMemSize);

    // vi init
    nn::vi::Initialize();

    nn::Result result = nn::vi::OpenDefaultDisplay(&viDisplay);
    if (!result.IsSuccess())
        OS::get_singleton()->alert("Failed open display");

    if (rendering_driver == "vulkan") {
        context_vulkan = memnew(VulkanContextNX);
        if (context_vulkan->initialize() != OK) {
			memdelete(context_vulkan);
			context_vulkan = nullptr;
			ERR_FAIL_MSG("Failed to initialize Vulkan context");
		}

        // Create Layer
        if (!viLayer) {
            result = nn::vi::CreateLayer(&viLayer, viDisplay, p_resolution.width, p_resolution.height);
            if (!result.IsSuccess())
                OS::get_singleton()->alert("Failed to create layer");

            // result = nn::vi::SetLayerScalingMode(m_viLayer, nn::vi::ScalingMode_FitToLayer);
            // if (!result.IsSuccess())
            //     OS::get_singleton()->alert("Failed to set layer scaling mode");

            if (context_vulkan->window_create(MAIN_WINDOW_ID, p_vsync_mode, viLayer, p_resolution.width, p_resolution.height) != OK) {
			    memdelete(context_vulkan);
			    context_vulkan = nullptr;
			    ERR_FAIL_MSG("Failed to create Vulkan window.");
		    }

            rendering_device_vulkan = memnew(RenderingDeviceVulkan);
		    rendering_device_vulkan->initialize(context_vulkan);

		    RendererCompositorRD::make_current();
            resolution = Size2i(p_resolution.width, p_resolution.height);
        }


    } else {
        // only vulkan is supported for now
        r_error = ERR_UNAVAILABLE;
    }

    r_error = OK;
	
	Input::get_singleton()->set_event_dispatch_function(_dispatch_input_event);
}
DisplayServerNX::~DisplayServerNX() {

    if (rendering_device_vulkan) {
		rendering_device_vulkan->finalize();
		memdelete(rendering_device_vulkan);
		rendering_device_vulkan = nullptr;
	}

	if (context_vulkan) {
		context_vulkan->window_destroy(MAIN_WINDOW_ID);
		memdelete(context_vulkan);
		context_vulkan = nullptr;
	}

    if (viLayer) {
        nn::vi::DestroyLayer(viLayer);
        viLayer = nullptr;
    }

    nn::vi::CloseDisplay(viDisplay);
    nn::vi::Finalize();

    nv::FinalizeGraphics();
}

bool DisplayServerNX::has_feature(Feature p_feature) const {
    switch (p_feature) {
        case FEATURE_TOUCHSCREEN:
            return true;
        default:
            return false;
    }
}

String DisplayServerNX::get_name() const {
    return "Nintendo Switch";
}

int DisplayServerNX::get_screen_count() const {
    return 1;
}

int DisplayServerNX::get_primary_screen() const {
    return 0;
}

Point2i DisplayServerNX::screen_get_position(int p_screen) const {
    return Point2i();
}

Size2i DisplayServerNX::screen_get_size(int p_screen) const {
    if (!viLayer)
        return Size2i();

    return resolution;
}

Rect2i DisplayServerNX::screen_get_usable_rect(int p_screen) const {
    return Rect2i(screen_get_position(p_screen), screen_get_size(p_screen));
}

int DisplayServerNX::screen_get_dpi(int p_screen) const {
    // TODO: Maybe there is a better way to handle this
    // TV's are hard I guess since, but we should know the size of
    // the device screen.  Not sure what the correct way to do this is.
    return 72;
}

float DisplayServerNX::screen_get_refresh_rate(int p_screen) const {
    return 60.0f;
}

bool DisplayServerNX::is_touchscreen_available() const {
    // TODO: only return true when in handheld mode
    return true;
}

Vector<DisplayServer::WindowID> DisplayServerNX::get_window_list() const {
    Vector<DisplayServer::WindowID> list;
	list.push_back(MAIN_WINDOW_ID);
	return list;
}

DisplayServer::WindowID DisplayServerNX::get_window_at_screen_position(const Point2i &p_position) const {
    return MAIN_WINDOW_ID;
}

void DisplayServerNX::window_attach_instance_id(ObjectID p_instance, WindowID p_window) {
    window_attached_instance_id = p_instance;
}

ObjectID DisplayServerNX::window_get_attached_instance_id(WindowID p_window) const {
    return window_attached_instance_id;
}

void DisplayServerNX::window_set_rect_changed_callback(const Callable &p_callable, WindowID p_window) {
    window_resize_callback = p_callable;
}

void DisplayServerNX::window_set_window_event_callback(const Callable &p_callable, WindowID p_window) {
    window_event_callback = p_callable;
}

void DisplayServerNX::window_set_input_event_callback(const Callable &p_callable, WindowID p_window) {
    input_event_callback = p_callable;
}

void DisplayServerNX::window_set_input_text_callback(const Callable &p_callable, WindowID p_window) {
    input_text_callback = p_callable;
}

void DisplayServerNX::window_set_drop_files_callback(const Callable &p_callable, WindowID p_window) {
    // not supported
}

void DisplayServerNX::window_set_title(const String &p_title, WindowID p_window) {
    // not supported
}

int DisplayServerNX::window_get_current_screen(WindowID p_window) const {
    return SCREEN_OF_MAIN_WINDOW;
}

void DisplayServerNX::window_set_current_screen(int p_screen, WindowID p_window) {
    // not supported
}

Point2i DisplayServerNX::window_get_position(WindowID p_window) const {
    return Point2i();
}

Point2i DisplayServerNX::window_get_position_with_decorations(WindowID p_window) const
{
    return Point2i();
}

void DisplayServerNX::window_set_position(const Point2i &p_position, WindowID p_window) {
    // not supported
}

void DisplayServerNX::window_set_transient(WindowID p_window, WindowID p_parent) {
    // not supported
}

void DisplayServerNX::window_set_max_size(const Size2i p_size, WindowID p_window) {
    // not supported
}

Size2i DisplayServerNX::window_get_max_size(WindowID p_window) const {
    return Size2i();
}

void DisplayServerNX::window_set_min_size(const Size2i p_size, WindowID p_window) {
    // not supported
}

Size2i DisplayServerNX::window_get_min_size(WindowID p_window) const {
    return Size2i();
}

void DisplayServerNX::window_set_size(const Size2i p_size, WindowID p_window) {
    // not supported
}

Size2i DisplayServerNX::window_get_size(WindowID p_window) const {
    return resolution;
}

Size2i DisplayServerNX::window_get_size_with_decorations(WindowID p_window) const {
    return window_get_size(p_window);
}

void DisplayServerNX::window_set_mode(WindowMode p_mode, WindowID p_window) {
    // not supported
}

DisplayServer::WindowMode DisplayServerNX::window_get_mode(WindowID p_window) const {
    return DisplayServer::WindowMode::WINDOW_MODE_FULLSCREEN;
}

bool DisplayServerNX::window_is_maximize_allowed(WindowID p_window) const {
    return false;
}

void DisplayServerNX::window_set_flag(WindowFlags p_flag, bool p_enabled, WindowID p_window) {
    // not supported
}

bool DisplayServerNX::window_get_flag(WindowFlags p_flag, WindowID p_window) const {
    return false;
}

void DisplayServerNX::window_request_attention(WindowID p_window) {
    // not supported
}

void DisplayServerNX::window_move_to_foreground(WindowID p_window) {
    // not supported
}

bool DisplayServerNX::window_is_focused(WindowID p_window) const {
    return true;
}

bool DisplayServerNX::window_can_draw(WindowID p_window) const {
    return true;
}

bool DisplayServerNX::can_any_window_draw() const {
    return true;
}

void DisplayServerNX::process_events() {

}

void DisplayServerNX::_dispatch_input_event(const Ref<InputEvent> &p_event) {
	Callable cb = get_singleton()->input_event_callback;
	if (!cb.is_null()) {
		Variant ev = p_event;
		Variant *evp = &ev;
		Variant ret;
		Callable::CallError ce;
		cb.callp((const Variant **)&evp, 1, ret, ce);
	}
}

DisplayServer::VSyncMode DisplayServerNX::window_get_vsync_mode(WindowID p_vsync_mode) const {
    return DisplayServer::VSYNC_ENABLED;
}

