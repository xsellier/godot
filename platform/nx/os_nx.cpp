#include "os_nx.h"
#include "netsocket_nx.h"
#include "ip_nx.h"
#include "dir_access_nx.h"
#include "file_access_nx.h"
#include "display_server_nx.h"

#include "main/main.h"

#include <assert.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>

// Operating Environment
#include <nn/oe.h>
// Dynamic Libraries (modules)
#include <nn/ro.h>
// File System
#include <nn/fs.h>
// Time
#include <nn/os/os_TickTypes.h>
#include <nn/time/time_Api.h>
#include <nn/time/time_TimeZoneApi.h>
#include <nn/time/time_StandardUserSystemClock.h>
#include <nn/crypto/crypto_Csrng.h>

#ifndef NO_NETWORK
// Network
#include <nn/nifm.h>
#include <nn/socket.h>
static nn::socket::ConfigDefaultWithMemory s_SocketConfigWithMemory;
#endif

/// Clock Setup function (used by get_ticks_usec)
static nn::os::Tick _clock_start = nn::os::Tick();
static void _setup_clock() {
	_clock_start = nn::os::GetSystemTick();
}

static nn::hid::NpadIdType s_NpadIds[] = {
	nn::hid::NpadId::No1,
    nn::hid::NpadId::No2,
    nn::hid::NpadId::No3,
    nn::hid::NpadId::No4,
	nn::hid::NpadId::Handheld
};

void OS_NX::queue_event(const Ref<InputEvent> &p_event)
{
	ERR_FAIL_INDEX(event_count, MAX_EVENTS);
	event_queue[event_count++] = p_event;
}

void OS_NX::mountRom() {
    nn::fs::QueryMountRomCacheSize(&romCacheSize);
	romCacheBuffer = memnew_arr(char, romCacheSize);
	nn::fs::MountRom("res", romCacheBuffer, romCacheSize);
}

void OS_NX::unmountRom() {
    nn::fs::Unmount("res");
	memdelete_arr(romCacheBuffer);
}

void OS_NX::getTouchscreenEvents() {
    nn::hid::TouchScreenState<nn::hid::TouchStateCountMax> state = {};
    nn::hid::GetTouchScreenState(&state);
  
    for (int i = 0; i < state.count; ++i)
    {
        // Processing for each touch.
		auto touchState = state.touches[i];
		Ref<InputEventScreenTouch> ev;
		ev.instantiate();
		ev->set_index(touchState.fingerId);
		ev->set_pressed(true);
		ev->set_position(Vector2(touchState.x, touchState.y));
		queue_event(ev);
    }
}

namespace {
	String getJoyName(int index, const nn::hid::NpadStyleSet &style)
	{
		String styleString;
		if (style.Test<nn::hid::NpadStyleFullKey>() == true) {
			styleString = "Fullkey";
		} else if (style.Test<nn::hid::NpadStyleHandheld>() == true) {
			styleString = "Handheld";
		} else if (style.Test<nn::hid::NpadStyleJoyDual>() == true) {
			styleString = "JoyDual";
		}

		return styleString + "_" + String::num(index);
	}

}

void OS_NX::getNpadEvents() {
    auto input = Input::get_singleton();
	for (int i = 0; i < 5; ++i) {
		nn::hid::NpadStyleSet npadStyleSet = nn::hid::GetNpadStyleSet(s_NpadIds[i]);
		if (npadStyleSet.IsAllOff()) {
			if (npadStates[i].connected == true) {
				npadStates[i].connected = false;
				input->joy_connection_changed(npadStates[i].deviceId, false, "", "");
			}
			continue;
		} else {
			if (npadStates[i].connected == false) {
				npadStates[i].connected = true;
				npadStates[i].deviceId = input->get_unused_joy_id();
				input->joy_connection_changed(npadStates[i].deviceId, true, getJoyName(i, npadStyleSet));
			}
		}

		
		if (npadStyleSet.Test<nn::hid::NpadStyleFullKey>() == true) {
			npadStates[i].previous.fullKey = npadStates[i].current.fullKey; 
			nn::hid::GetNpadState(&(npadStates[i].current.fullKey), s_NpadIds[i]);
			process_joy_buttons(npadStates[i].deviceId, npadStates[i].current.fullKey.buttons);
			process_joy_axis(npadStates[i].deviceId, npadStates[i].current.fullKey.analogStickL, npadStates[i].current.fullKey.analogStickR);
			process_joy_vibration(i);

		} else if (npadStyleSet.Test<nn::hid::NpadStyleHandheld>() == true) {
			npadStates[i].previous.handheld = npadStates[i].current.handheld;
			nn::hid::GetNpadState(&(npadStates[i].current.handheld), s_NpadIds[i]);
			process_joy_buttons(npadStates[i].deviceId, npadStates[i].current.handheld.buttons);
			process_joy_axis(npadStates[i].deviceId, npadStates[i].current.handheld.analogStickL, npadStates[i].current.handheld.analogStickR);
			process_joy_vibration(i);

		} else if (npadStyleSet.Test<nn::hid::NpadStyleJoyDual>() == true) {
			npadStates[i].previous.joyDual = npadStates[i].current.joyDual;
			nn::hid::GetNpadState(&(npadStates[i].current.joyDual), s_NpadIds[i]);
			process_joy_buttons(npadStates[i].deviceId, npadStates[i].current.joyDual.buttons);
			process_joy_axis(npadStates[i].deviceId, npadStates[i].current.joyDual.analogStickL, npadStates[i].current.joyDual.analogStickR);
			process_joy_vibration(i);

		}
	}
}

void OS_NX::process_joy_vibration(int deviceIndex)
{
	auto input = Input::get_singleton();
	uint64_t timestamp = input->get_joy_vibration_timestamp(deviceIndex);
	if (timestamp > npadStates[deviceIndex].ff_timestamp) {
		Vector2 strength = input->get_joy_vibration_strength(deviceIndex);
		float duration = input->get_joy_vibration_duration(deviceIndex);
		if (strength.x == 0 && strength.y == 0) {
			joypad_vibration_stop(deviceIndex, timestamp);
		} else {
			joypad_vibration_start(deviceIndex, strength.x, strength.y, duration, timestamp);
		}
	} else if (npadStates[deviceIndex].vibrating && npadStates[deviceIndex].ff_end_timestamp != 0) {
		uint64_t current_time = OS::get_singleton()->get_ticks_usec();
		if (current_time >= npadStates[deviceIndex].ff_end_timestamp)
			joypad_vibration_stop(deviceIndex, current_time);
	}
}


void OS_NX::process_input() {
	// Touchscreen
	getTouchscreenEvents();
	
	// Joypads
	getNpadEvents();

	// Empty queue
	for (int i = 0; i < event_count; i++)
		Input::get_singleton()->parse_input_event(event_queue[i]);
	event_count = 0;
}

void OS_NX::process_joy_buttons(int deviceIndex, const nn::hid::NpadButtonSet& currentState) {
    auto input = Input::get_singleton();

	if (currentState.Test<nn::hid::NpadJoyButton::A>()) {
		input->joy_button(deviceIndex, JoyButton::A, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::A, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::B>()) {
		input->joy_button(deviceIndex, JoyButton::B, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::B, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::X>()) {
		input->joy_button(deviceIndex, JoyButton::X, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::X, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Y>()) {
		input->joy_button(deviceIndex, JoyButton::Y, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::Y, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::StickL>()) {
		input->joy_button(deviceIndex, JoyButton::LEFT_STICK, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::LEFT_STICK, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::StickR>()) {
		input->joy_button(deviceIndex, JoyButton::RIGHT_STICK, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::RIGHT_STICK, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::L>()) {
		input->joy_button(deviceIndex, JoyButton::LEFT_SHOULDER, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::LEFT_SHOULDER, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::R>()) {
		input->joy_button(deviceIndex, JoyButton::RIGHT_SHOULDER, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::RIGHT_SHOULDER, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::ZL>()) {
        input->joy_axis(deviceIndex, JoyAxis::TRIGGER_LEFT, 1.0f);
		//input->joy_button(deviceIndex, JOY_L2, true);
	} else {
		//input->joy_button(deviceIndex, JOY_L2, false);
        input->joy_axis(deviceIndex, JoyAxis::TRIGGER_LEFT, -1.0f);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::ZR>()) {
		//input->joy_button(deviceIndex, JOY_R2, true);
        input->joy_axis(deviceIndex, JoyAxis::TRIGGER_RIGHT, 1.0f);
	} else {
		//input->joy_button(deviceIndex, JOY_R2, false);
        input->joy_axis(deviceIndex, JoyAxis::TRIGGER_RIGHT, -1.0f);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Plus>()) {
		input->joy_button(deviceIndex, JoyButton::START, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::START, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Minus>()) {
		input->joy_button(deviceIndex, JoyButton::BACK, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::BACK, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Left>()) {
		input->joy_button(deviceIndex, JoyButton::DPAD_LEFT, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::DPAD_LEFT, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Up>()) {
		input->joy_button(deviceIndex, JoyButton::DPAD_UP, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::DPAD_UP, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::Right>()) {
		input->joy_button(deviceIndex, JoyButton::DPAD_RIGHT, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::DPAD_RIGHT, false);
	} 
 	
	if (currentState.Test<nn::hid::NpadJoyButton::Down>()) {
		input->joy_button(deviceIndex, JoyButton::DPAD_DOWN, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::DPAD_DOWN, false);	
	}
}

void OS_NX::process_joy_axis(int deviceIndex, const nn::hid::AnalogStickState &leftStick, const nn::hid::AnalogStickState &rightStick) {
    auto input = Input::get_singleton();
	float lx = float(leftStick.x) / nn::hid::AnalogStickMax;
	float ly = -float(leftStick.y) / nn::hid::AnalogStickMax;
	float rx = float(rightStick.x) / nn::hid::AnalogStickMax;
	float ry = -float(rightStick.y) / nn::hid::AnalogStickMax;

	input->joy_axis(deviceIndex, JoyAxis::LEFT_X, lx);
	input->joy_axis(deviceIndex, JoyAxis::LEFT_Y, ly);
	input->joy_axis(deviceIndex, JoyAxis::RIGHT_X, rx);
	input->joy_axis(deviceIndex, JoyAxis::RIGHT_Y, ry);
}

void OS_NX::initialize() {
#ifndef NO_NETWORK
	nn::nifm::Initialize();
	nn::socket::Initialize(s_SocketConfigWithMemory);
	NetSocket_NX::make_default();
	IP_NX::make_default();
#endif

	// mount rom
	mountRom();


	// mount savedata

	FileAccess::make_default<FileAccessNX>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessNX>(FileAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessNX>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessNX>(DirAccess::ACCESS_USERDATA);

	NpadIdCountMax = sizeof(s_NpadIds) / sizeof(nn::hid::NpadIdType);
	nn::hid::InitializeTouchScreen();
	nn::hid::InitializeNpad();
	nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleHandheld::Mask);
	nn::hid::SetSupportedNpadIdType(s_NpadIds, NpadIdCountMax);

	for (int i = 0; i < NpadIdCountMax; ++i)
    {
        nn::hid::NpadStyleSet mask = (s_NpadIds[i] == nn::hid::NpadId::Handheld) ? nn::hid::NpadStyleHandheld::Mask : nn::hid::NpadStyleFullKey::Mask;
        vibration_device_count += nn::hid::GetVibrationDeviceHandles(&vibration_handles[2*i], 2, s_NpadIds[i], mask);
    }

    for (int i = 0; i < vibration_device_count; ++i)
    {
        nn::hid::InitializeVibrationDevice(vibration_handles[i]);
    }

    main_loop = nullptr;
}

void OS_NX::initialize_joypads() {

}

void OS_NX::joypad_vibration_start(int p_device, float p_weak_magnitude, float p_strong_magnitude, float p_duration, uint64_t p_timestamp) {

    nn::hid::VibrationValue vib = nn::hid::VibrationValue::Make(p_weak_magnitude, 60.0f, p_strong_magnitude, 120.0f);
    nn::hid::SendVibrationValue(vibration_handles[p_device*2], vib);
    nn::hid::SendVibrationValue(vibration_handles[(p_device*2) + 1], vib);
    npadStates[p_device].vibrating = true;
    npadStates[p_device].ff_timestamp = p_timestamp;
	npadStates[p_device].ff_end_timestamp = p_duration == 0 ? 0 : p_timestamp + (uint64_t)(p_duration * 1000000.0);
}

void OS_NX::joypad_vibration_stop(int p_device, uint64_t p_timestamp) {

    nn::hid::VibrationValue vib = nn::hid::VibrationValue::Make();
    nn::hid::SendVibrationValue(vibration_handles[p_device*2], vib);
    nn::hid::SendVibrationValue(vibration_handles[(p_device*2) + 1], vib);
    npadStates[p_device].vibrating = false;
}

void OS_NX::set_main_loop(MainLoop *p_main_loop) {
    main_loop = p_main_loop;
}

void OS_NX::delete_main_loop() {
    if (main_loop) {
		memdelete(main_loop);
	}
	main_loop = nullptr;
}

void OS_NX::finalize() {
    if (main_loop) {
		memdelete(main_loop);
	}
	main_loop = nullptr;
}

void OS_NX::finalize_core() {
    unmountRom();
}

bool OS_NX::_check_internal_feature_support(const String &p_feature) {
    return p_feature == "mobile";
}

String OS_NX::get_stdin_string() {
	char buff[1024];
	return String::utf8(fgets(buff, 1024, stdin));
}

Vector<String> OS_NX::get_video_adapter_driver_info() const {
	return Vector<String>();
}

Error OS_NX::get_entropy(uint8_t *r_buffer, int p_bytes) {
    nn::crypto::GenerateCryptographicallyRandomBytes(r_buffer, size_t(p_bytes));
    return OK;
}

Error OS_NX::execute(const String &p_path, const List<String> &p_arguments, String *r_pipe, int *r_exitcode, bool read_stderr, Mutex *p_pipe_mutex, bool p_open_console) {
    return ERR_UNAVAILABLE;
}

Error OS_NX::create_process(const String &p_path, const List<String> &p_arguments, ProcessID *r_child_id, bool p_open_console) {
    return ERR_UNAVAILABLE;
}

Error OS_NX::kill(const ProcessID &p_pid) {
    return ERR_UNAVAILABLE;
}

bool OS_NX::is_process_running(const ProcessID &p_pid) const {
    return false;
}

bool OS_NX::has_environment(const String &p_var) const {
    return false;
}

String OS_NX::get_environment(const String &p_var) const {
    return String();
}

void OS_NX::set_environment(const String &p_var, const String &p_value) const {
	return;
}
void OS_NX::unset_environment(const String &p_var) const {
	return;
}

String OS_NX::get_name() const {
    return "Nintendo Switch";
}

String OS_NX::get_distribution_name() const
{
	return "";
}
String OS_NX::get_version() const
{
	return "";
}

MainLoop *OS_NX::get_main_loop() const {
    return main_loop;
}

OS::DateTime OS_NX::get_datetime(bool p_utc) const {
	time_t t = time(nullptr);
	struct tm lt;
	if (p_utc) {
		gmtime_r(&t, &lt);
	} else {
		localtime_r(&t, &lt);
	}
	DateTime ret;
	ret.year = 1900 + lt.tm_year;
	// Index starting at 1 to match OS_Unix::get_date
	//   and Windows SYSTEMTIME and tm_mon follows the typical structure
	//   of 0-11, noted here: http://www.cplusplus.com/reference/ctime/tm/
	ret.month = (Month)(lt.tm_mon + 1);
	ret.day = lt.tm_mday;
	ret.weekday = (Weekday)lt.tm_wday;
	ret.hour = lt.tm_hour;
	ret.minute = lt.tm_min;
	ret.second = lt.tm_sec;
	ret.dst = lt.tm_isdst;

	return ret;
}

OS::TimeZoneInfo OS_NX::get_time_zone_info() const {
	time_t t = time(NULL);
	struct tm *lt = localtime(&t);
	char name[16];
	strftime(name, 16, "%Z", lt);
	name[15] = 0;
	TimeZoneInfo ret;
	ret.name = name;

	char bias_buf[16];
	strftime(bias_buf, 16, "%z", lt);
	int bias;
	bias_buf[15] = 0;
	sscanf(bias_buf, "%d", &bias);

	// convert from ISO 8601 (1 minute=1, 1 hour=100) to minutes
	int hour = (int)bias / 100;
	int minutes = bias % 100;
	if (bias < 0)
		ret.bias = hour * 60 - minutes;
	else
		ret.bias = hour * 60 + minutes;

	return ret;
}

void OS_NX::delay_usec(uint32_t p_usec) const {
    struct timespec rem = { static_cast<time_t>(p_usec / 1000000), static_cast<long>((p_usec % 1000000) * 1000) };
	while (nanosleep(&rem, &rem) == EINTR) {
	}
}

uint64_t OS_NX::get_ticks_usec() const {
	// Unchecked return. Static analyzers might complain.
	// If _setup_clock() succeded, we assume clock_gettime() works.

	nn::os::Tick longtime = nn::os::GetSystemTick();
	longtime -= _clock_start;

	return nn::os::ConvertToTimeSpan(longtime).GetMicroSeconds();
}

OS_NX::OS_NX() {
    AudioDriverManager::add_driver(&audio_driver);
    DisplayServerNX::register_nx_driver();
}

void OS_NX::run() {
    force_quit = false;
    if (!main_loop)
		return;

	main_loop->initialize();

	while (!force_quit) {

		process_input();

		if (Main::iteration() == true) 
			break;
	};

	main_loop->finalize();
}

String OS_NX::get_locale() const {
    nn::settings::LanguageCode code = nn::oe::GetDesiredLanguage();
	return String(code.string);
}

int OS_NX::get_processor_count() const {
    return 3;
}

String OS_NX::get_user_data_dir() const {
	// SAVE directory
	return "user://";
}

String OS_NX::get_resource_dir() const {
	// ROM directory
	return "res://";
}
