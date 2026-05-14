#include "display_server_nx.h"

#include "core/os/memory.h"
#include "core/os/os.h"

#include <nn/util/util_BitUtil.h>
#include <nv/nv_MemoryManagement.h>
#include <nn/gll.h>
#include <nn/swkbd/swkbd_Api.h>
#include <nn/swkbd/swkbd_Result.h>

#include "core/config/project_settings.h"

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

DisplayServer *DisplayServerNX::create_func(const String &p_rendering_driver, DisplayServer::WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, Context p_context, int64_t p_parent_window, Error &r_error) {
    return memnew(DisplayServerNX(p_rendering_driver, p_mode, p_vsync_mode, p_flags, p_position, p_resolution, p_screen, p_parent_window, r_error));
}

Vector<String> DisplayServerNX::get_rendering_drivers_func() {
    Vector<String> drivers;

	drivers.push_back("vulkan");

	return drivers;
}

DisplayServerNX::DisplayServerNX(const String &p_rendering_driver, DisplayServer::WindowMode p_mode, DisplayServer::VSyncMode p_vsync_mode, uint32_t p_flags, const Vector2i *p_position, const Vector2i &p_resolution, int p_screen, int64_t p_parent_window, Error &r_error)
{
	rendering_driver = p_rendering_driver;

	native_menu = memnew(NativeMenu);

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

			union {
#ifdef VULKAN_ENABLED
				VulkanContextNX::WindowPlatformData vulkan;
#endif
			} wpd;
		
			wpd.vulkan.viLayer = viLayer;

			if (context_vulkan->window_create(MAIN_WINDOW_ID, &wpd) != OK) {
			    memdelete(context_vulkan);
			    context_vulkan = nullptr;
			    ERR_FAIL_MSG("Failed to create Vulkan window.");
		    }

            rendering_device_vulkan = memnew(RenderingDevice);
		    
			if (rendering_device_vulkan->initialize(context_vulkan, MAIN_WINDOW_ID) != OK) {
				rendering_device_vulkan = nullptr;
				memdelete(context_vulkan);
				context_vulkan = nullptr;
				r_error = ERR_UNAVAILABLE;
				ERR_FAIL_MSG("Failed to initialize Vulkan rendering device.");
				return;
			}
			
			context_vulkan->window_set_size(MAIN_WINDOW_ID, p_resolution.width, p_resolution.height);
			context_vulkan->window_set_vsync_mode(MAIN_WINDOW_ID, p_vsync_mode);

			rendering_device_vulkan->screen_create(MAIN_WINDOW_ID);

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

	if (native_menu) {
		memdelete(native_menu);
		native_menu = nullptr;
	}

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
		case FEATURE_MOUSE:
		case FEATURE_MOUSE_WARP:
        case FEATURE_TOUCHSCREEN:
    	case FEATURE_VIRTUAL_KEYBOARD:
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
	if (resolution.x == p_size.width and resolution.y == p_size.height) {
		return;
	}

	rendering_device_vulkan->screen_free(MAIN_WINDOW_ID);

	context_vulkan->window_destroy(MAIN_WINDOW_ID);
	
    nn::vi::DestroyLayer(viLayer);
    viLayer = nullptr;
	
	nn::Result result = nn::vi::CreateLayer(&viLayer, viDisplay, p_size.width, p_size.height);
	if (!result.IsSuccess())
		OS::get_singleton()->alert("Failed to create layer");
	
	union {
		VulkanContextNX::WindowPlatformData vulkan;
	} wpd;

	wpd.vulkan.viLayer = viLayer;

	if (context_vulkan->window_create(MAIN_WINDOW_ID, &wpd) != OK) {
		memdelete(context_vulkan);
		context_vulkan = nullptr;
		ERR_FAIL_MSG("Failed to create Vulkan window.");
	}

	context_vulkan->window_set_size(MAIN_WINDOW_ID, p_size.width, p_size.height);
	// context_vulkan->window_set_vsync_mode(MAIN_WINDOW_ID, p_vsync_mode);

	rendering_device_vulkan->screen_create(MAIN_WINDOW_ID);

	RendererCompositorRD::make_current();
	
	resolution = Size2i(p_size.width, p_size.height);
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
	if (cb.is_valid()) {
		cb.call(p_event);
	}
}

DisplayServer::VSyncMode DisplayServerNX::window_get_vsync_mode(WindowID p_vsync_mode) const {
    return DisplayServer::VSYNC_ENABLED;
}

void DisplayServerNX::mouse_set_mode(MouseMode p_mode) {
	ERR_FAIL_INDEX(p_mode, MouseMode::MOUSE_MODE_MAX);
	if (p_mode == mouse_mode) {
		return;
	}
	mouse_mode = p_mode;
}

void DisplayServerNX::cursor_set_custom_image(const Ref<Resource> &p_cursor, CursorShape p_shape, const Vector2 &p_hotspot) {
	ERR_FAIL_INDEX(p_shape, CURSOR_MAX);	
	String cursor_path = p_cursor.is_valid() ? p_cursor->get_path() : "";
	if (!cursor_path.is_empty()) {
		cursor_path = ProjectSettings::get_singleton()->globalize_path(cursor_path);
	}
	
	// STUB. NX doesn't show any cursor natively
}

Point2i DisplayServerNX::mouse_get_position() const {
	return Input::get_singleton()->get_mouse_position();
}

void DisplayServerNX::warp_mouse(const Point2i &p_to) {
	return Input::get_singleton()->set_mouse_position(p_to);
}

DisplayServer::MouseMode DisplayServerNX::mouse_get_mode() const {
	return mouse_mode;
}

void DisplayServerNX::_window_callback(const Callable &p_callable, const Variant &p_arg, bool p_deferred) const {
	if (p_callable.is_valid()) {
		if (p_deferred) {
			p_callable.call_deferred(p_arg);
		} else {
			p_callable.call(p_arg);
		}
	}
}

void DisplayServerNX::virtual_keyboard_show(const String &p_existing_text, const Rect2 &p_screen_rect, VirtualKeyboardType p_type, int p_max_length, int p_cursor_start, int p_cursor_end) {
    nn::swkbd::ShowKeyboardArg showKeyboardArg{};

	showKeyboardArg.keyboardConfig.textMaxLength = p_max_length;

	if (p_type == KEYBOARD_TYPE_PASSWORD) {
		// TODO NX: This does not seem to toggle the password mode properly
		nn::swkbd::MakePreset( &( showKeyboardArg.keyboardConfig ), nn::swkbd::Preset_Password );
		showKeyboardArg.keyboardConfig.passwordMode = nn::swkbd::PasswordMode_Hide;
	} else {
		nn::swkbd::MakePreset( &( showKeyboardArg.keyboardConfig ), nn::swkbd::Preset_Default );
	}

	if (p_type == KEYBOARD_TYPE_MULTILINE) {
		showKeyboardArg.keyboardConfig.inputFormMode = nn::swkbd::InputFormMode::InputFormMode_MultiLine;
		showKeyboardArg.keyboardConfig.isUseNewLine = true;
	} else {
		showKeyboardArg.keyboardConfig.inputFormMode = nn::swkbd::InputFormMode::InputFormMode_OneLine;
		showKeyboardArg.keyboardConfig.isUseNewLine = false;
	}

	if (p_type == KEYBOARD_TYPE_NUMBER) {
		showKeyboardArg.keyboardConfig.keyboardMode = nn::swkbd::KeyboardMode::KeyboardMode_Numeric;
	} else if (p_type == KEYBOARD_TYPE_NUMBER_DECIMAL) {
		showKeyboardArg.keyboardConfig.keyboardMode = nn::swkbd::KeyboardMode::KeyboardMode_Numeric;
		showKeyboardArg.keyboardConfig.leftOptionalSymbolKey = u'-';
		showKeyboardArg.keyboardConfig.rightOptionalSymbolKey = u'.';
	} else if (p_type == KEYBOARD_TYPE_PHONE) {
		showKeyboardArg.keyboardConfig.keyboardMode = nn::swkbd::KeyboardMode::KeyboardMode_Numeric;
		showKeyboardArg.keyboardConfig.leftOptionalSymbolKey = u'+';
	} else {
		showKeyboardArg.keyboardConfig.keyboardMode = nn::swkbd::KeyboardMode::KeyboardMode_LanguageSet2;
	}

	if (p_type == KEYBOARD_TYPE_EMAIL_ADDRESS) {
		showKeyboardArg.keyboardConfig.invalidCharFlag = nn::swkbd::InvalidCharFlag_Percent | nn::swkbd::InvalidCharFlag_Slash | nn::swkbd::InvalidCharFlag_Space;
	}

    //showKeyboardArg.keyboardConfig.isUseUtf8 = true;
    showKeyboardArg.keyboardConfig.textMaxLength = p_max_length;
    showKeyboardArg.keyboardConfig.textMinLength = 0;

    // Allocate buffers for shared memory
    size_t in_heap_size = nn::swkbd::GetRequiredWorkBufferSize( false );
    void* swkbd_work_buffer = static_cast<char16_t *>(aligned_alloc(nn::os::MemoryPageSize, in_heap_size));

    showKeyboardArg.workBuf = swkbd_work_buffer;
    showKeyboardArg.workBufSize = in_heap_size;

    Char16String text_utf16 = p_existing_text.utf16();
    nn::swkbd::SetInitialText(&showKeyboardArg, text_utf16.get_data());

    size_t out_heap_size = nn::swkbd::GetRequiredStringBufferSize();
    nn::swkbd::String output_string{};
    output_string.ptr = static_cast<char16_t *>(aligned_alloc(nn::os::MemoryPageSize, out_heap_size));
    output_string.bufSize = out_heap_size;

	if (nn::Result result = nn::swkbd::ShowKeyboard(&output_string, showKeyboardArg); nn::swkbd::ResultCanceled::Includes(result)) {
        //WARN_PRINT( " -- cancel --\n" );
    }
    else if( result.IsSuccess() )
    {
        char16_t* str_ptr = static_cast< char16_t* >( output_string.ptr );
        String s = String::utf16(str_ptr);
        _window_callback(input_text_callback, s);
    }
    free(swkbd_work_buffer);
    free(output_string.ptr);
}