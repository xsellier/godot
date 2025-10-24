#ifndef OS_NX_H
#define OS_NX_H

#include "core/input/input.h"
#include "core/os/os.h"
#include "servers/audio_server.h"
#include "drivers/vulkan/rendering_device_driver_vulkan.h"
#include "vulkan_context_nx.h"

#include "audio_driver_nx.h"

// Input
#include <nn/hid.h>
#include <nn/hid/hid_Npad.h>
#include <nn/hid/hid_NpadJoy.h>
#include <nn/hid/hid_Vibration.h>

static bool disable_applet_support_calling_on_changed_connect = false;

class OS_NX : public OS {
	enum {
		MAX_EVENTS = 64,
	};

    union NpadState {
		nn::hid::NpadHandheldState handheld;
		nn::hid::NpadFullKeyState fullKey;
		nn::hid::NpadJoyDualState joyDual;
	};

	struct {
		bool connected = false;
		bool previous_connected = false;
		int deviceId = -1;
		NpadState current;
		NpadState previous;
		float ff_timestamp = 0.0f;
		float ff_end_timestamp = 0.0f;
		bool vibrating = false;
	} npadStates[5];

	int vibration_device_count = 0;
	nn::hid::VibrationDeviceHandle vibration_handles[10] = {};
	int npadIdCountMax;
	nn::hid::ControllerSupportArg controllerArg;
	bool npadInit = false;
	bool handHeldMode = false;
	bool ignore_changed_connect_status = false;

    bool force_quit = false;

    MainLoop *main_loop = nullptr;

    AudioDriverNX audio_driver;

	Ref<InputEvent> event_queue[MAX_EVENTS];
	int event_count = 0;
	void queue_event(const Ref<InputEvent> &p_event);
	int last_touch_count = 0;
	nn::hid::TouchScreenState<nn::hid::TouchStateCountMax> state = {};
	Vector2 last_touch_pos[nn::hid::TouchStateCountMax];

    String stdin_buf;

	// File System Data
	void mountRom();
	void unmountRom();
	size_t romCacheSize = 0;
    char* romCacheBuffer = nullptr;
	void mountCache();
	void unmountCache();


	void getTouchscreenEvents();
	void getNpadEvents();
	void getNpadSupport();
	void process_input();
	void process_joy_buttons(int deviceIndex, const nn::hid::NpadButtonSet& currentState);
	void process_joy_vibration(int deviceIndex);
	void process_joy_axis(int deviceIndex, const nn::hid::AnalogStickState &leftStick, const nn::hid::AnalogStickState &rightStick);
	
	void joypad_vibration_start(int p_device, float p_weak_magnitude, float p_strong_magnitude, float p_duration, uint64_t p_timestamp);
	void joypad_vibration_stop(int p_device, uint64_t p_timestamp);

protected:
    void initialize() override;
	void initialize_joypads() override;

    void set_main_loop(MainLoop *p_main_loop) override;
	void delete_main_loop() override;

	void finalize() override;
	void finalize_core() override;

    bool _check_internal_feature_support(const String &p_feature) override;

public:
    String get_stdin_string(int64_t p_buffer_size = 1024) override;
	PackedByteArray get_stdin_buffer(int64_t p_buffer_size = 1024) override;

	Vector<String> get_video_adapter_driver_info() const override;

	Error get_entropy(uint8_t *r_buffer, int p_bytes) override;

	Error open_dynamic_library(const String &p_path, void *&p_library_handle, GDExtensionData *p_data = nullptr) override;
	Error close_dynamic_library(void *p_library_handle) override;
	Error get_dynamic_library_symbol_handle(void *p_library_handle, const String &p_name, void *&p_symbol_handle, bool p_optional = false) override;

    Error execute(const String &p_path, const List<String> &p_arguments, String *r_pipe = nullptr, int *r_exitcode = nullptr, bool read_stderr = false, Mutex *p_pipe_mutex = nullptr, bool p_open_console = false) override;
	Error create_process(const String &p_path, const List<String> &p_arguments, ProcessID *r_child_id = nullptr, bool p_open_console = false) override;
    Error kill(const ProcessID &p_pid) override;
    bool is_process_running(const ProcessID &p_pid) const override;

    bool has_environment(const String &p_var) const override;
	String get_environment(const String &p_var) const override;
	void set_environment(const String &p_var, const String &p_value) const override;
	void unset_environment(const String &p_var) const override;

    String get_name() const override;
	String get_distribution_name() const override;
	String get_version() const override;

    MainLoop *get_main_loop() const override;

    DateTime get_datetime(bool p_utc = false) const override;

	TimeZoneInfo get_time_zone_info() const override;

    void delay_usec(uint32_t p_usec) const override;

    uint64_t get_ticks_usec() const override;

	String get_locale() const override;
	int get_processor_count() const override;

	String get_user_data_dir() const override;
	String get_resource_dir() const override;
	String get_cache_path() const override;
	
	int get_process_exit_code(const ProcessID &p_pid) const override;

    OS_NX();
    void run();
};

#endif // OS_NX_H