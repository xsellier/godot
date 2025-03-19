#include "os_nx.h"
#include "netsocket_nx.h"
#include "ip_nx.h"
#include "dir_access_nx.h"
#include "file_access_nx.h"
#include "display_server_nx.h"

#include "main/main.h"
#include "core/config/project_settings.h"

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

#ifdef PROFILER_ENABLED
#include <nn/profiler.h>
static char *profiler_buffer = nullptr;
#endif

#define MAX_FILE_SIZE (0x400000)
static uint8_t *core_dynamic_modules_nrr = nullptr;
nn::ro::RegistrationInfo core_dynamic_modules_info;

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

void OS_NX::mountCache() {
	NN_LOG("Mount cache data\n");
	nn::Result result = nn::fs::MountCacheStorage("cache");
	NN_ABORT_UNLESS_RESULT_SUCCESS(result);
}

void OS_NX::unmountCache() {
	NN_LOG("Unmount cache data\n");
	nn::fs::Unmount("cache");
}

void OS_NX::getTouchscreenEvents()
{
    nn::hid::GetTouchScreenState(&state);
	if (state.count != last_touch_count) {
		if (state.count > last_touch_count) {
			for (int i = last_touch_count; i < state.count; i++) {
				Vector2 pos(state.touches[i].x, state.touches[i].y);
				Ref<InputEventScreenTouch> st;
				st.instantiate();
				st->set_index(i);
				st->set_position(pos);
				st->set_pressed(true);
				queue_event(st);
			}
		} else {
			for (int i = state.count; i < last_touch_count; i++) {
				Ref<InputEventScreenTouch> st;
				st.instantiate();
				st->set_index(i);
				st->set_position(last_touch_pos[i]);
				st->set_pressed(false);
				queue_event(st);
			}
		}
	} else {
		for (int i = 0; i < state.count; i++) {
			Vector2 pos(state.touches[i].x, state.touches[i].y);
			Ref<InputEventScreenDrag> sd;
			sd.instantiate();
			sd->set_index(i);
			sd->set_position(pos);
			sd->set_relative(pos - last_touch_pos[i]);
			last_touch_pos[i] = pos;
			queue_event(sd);
		}
	}

	last_touch_count = state.count;
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

void OS_NX::getNpadEvents()
{
	auto input = Input::get_singleton();
	for (int i = 0; i < npadIdCountMax; ++i) {
		nn::hid::NpadStyleSet npadStyleSet = nn::hid::GetNpadStyleSet(s_NpadIds[i]);
		npadStates[i].previous_connected = npadStates[i].connected;
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
				if (!npadInit)
					npadStates[i].previous_connected = npadStates[i].connected;
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
		npadInit = true;
	}

	getNpadSupport();

}

void OS_NX::getNpadSupport()
{

	for (int i = 0; i < npadIdCountMax; ++i) {
		if (npadStates[i].previous_connected != npadStates[i].connected && s_NpadIds[i] != nn::hid::NpadId::Handheld){
			if(!ignore_changed_connect_status)
				nn::hid::ShowControllerSupport(controllerArg);
			nn::hid::NpadStyleSet npadStyleSet = nn::hid::GetNpadStyleSet(s_NpadIds[i]);
			if (npadStyleSet.IsAllOff()) {
				if (npadStates[i].connected == true) {
					ignore_changed_connect_status = true;
				} else{
					ignore_changed_connect_status = false;
				}
			} else {
				if (npadStates[i].connected == false) {
					ignore_changed_connect_status = true;
				} else{
					ignore_changed_connect_status = false;
				}
			}
			npadStates[i].previous_connected = npadStates[i].connected;
			break;
		}
	}

	if(nn::oe::GetOperationMode() == nn::oe::OperationMode_Handheld){
		if(!handHeldMode){
			ignore_changed_connect_status = false;
		}
		handHeldMode = true;
	}
	else{
		if(handHeldMode){
			nn::hid::ShowControllerSupport(controllerArg);
			ignore_changed_connect_status = true;
		}

		handHeldMode = false;
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
	Input::get_singleton()->flush_buffered_events();
}

void OS_NX::process_joy_buttons(int deviceIndex, const nn::hid::NpadButtonSet& currentState) {
    auto input = Input::get_singleton();

	if (currentState.Test<nn::hid::NpadJoyButton::B>()) {
		input->joy_button(deviceIndex, JoyButton::A, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::A, false);
	} 
	
	if (currentState.Test<nn::hid::NpadJoyButton::A>()) {
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
	
	if (currentState.Test<nn::hid::NpadJoyButton::LeftSL>()) {
		input->joy_button(deviceIndex, JoyButton::PADDLE1, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::PADDLE1, false);	
	}
	
	if (currentState.Test<nn::hid::NpadJoyButton::LeftSR>()) {
		input->joy_button(deviceIndex, JoyButton::PADDLE2, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::PADDLE2, false);	
	}
	
	if (currentState.Test<nn::hid::NpadJoyButton::RightSL>()) {
		input->joy_button(deviceIndex, JoyButton::PADDLE3, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::PADDLE3, false);	
	}
	if (currentState.Test<nn::hid::NpadJoyButton::RightSR>()) {
		input->joy_button(deviceIndex, JoyButton::PADDLE4, true);
	} else {
		input->joy_button(deviceIndex, JoyButton::PADDLE4, false);	
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

#ifdef PROFILER_ENABLED
    profiler_buffer = new char[nn::profiler::MinimumBufferSize];
    nn::Result result = nn::profiler::Initialize(profiler_buffer, nn::profiler::MinimumBufferSize);
    NN_ABORT_UNLESS_RESULT_SUCCESS(result);
#endif

	// mount rom
	mountRom();

	// mount cache (if used)
	// if (ProjectSettings::get_singleton()->get("rendering/shader_compiler/shader_cache/enabled")) {
	// 	mountCache();
	// }

	// mount savedata

	FileAccess::make_default<FileAccessNX>(FileAccess::ACCESS_RESOURCES);
	FileAccess::make_default<FileAccessNX>(FileAccess::ACCESS_USERDATA);
	FileAccess::make_default<FileAccessNX>(FileAccess::ACCESS_FILESYSTEM);
	DirAccess::make_default<DirAccessNX>(DirAccess::ACCESS_RESOURCES);
	DirAccess::make_default<DirAccessNX>(DirAccess::ACCESS_USERDATA);
	DirAccess::make_default<DirAccessNX>(DirAccess::ACCESS_FILESYSTEM);

	npadIdCountMax = sizeof(s_NpadIds) / sizeof(nn::hid::NpadIdType);
	nn::hid::InitializeTouchScreen();
	nn::hid::InitializeNpad();
	nn::hid::SetNpadJoyHoldType(nn::hid::NpadJoyHoldType_Horizontal);
	nn::hid::SetSupportedNpadStyleSet(nn::hid::NpadStyleFullKey::Mask | nn::hid::NpadStyleJoyDual::Mask | nn::hid::NpadStyleHandheld::Mask);
	nn::hid::SetSupportedNpadIdType(s_NpadIds, npadIdCountMax);

	controllerArg.SetDefault();
    controllerArg.playerCountMax = 1;
    controllerArg.enableSingleMode = true;

	for (int i = 0; i < npadIdCountMax; ++i)
    {
        nn::hid::NpadStyleSet mask = (s_NpadIds[i] == nn::hid::NpadId::Handheld) ? nn::hid::NpadStyleHandheld::Mask : nn::hid::NpadStyleFullKey::Mask;
        vibration_device_count += nn::hid::GetVibrationDeviceHandles(&vibration_handles[2*i], 2, s_NpadIds[i], mask);
    }

    for (int i = 0; i < vibration_device_count; ++i)
    {
        nn::hid::InitializeVibrationDevice(vibration_handles[i]);
    }

	handHeldMode = nn::oe::GetOperationMode() == nn::oe::OperationMode_Handheld ? true : false;

    main_loop = nullptr;
    _setup_clock();
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
#ifdef PROFILER_ENABLED
    nn::profiler::ProfilerStatus status = nn::profiler::GetProfilerStatus();
	while (status == nn::profiler::ProfilerStatus_Profiling ||
        status == nn::profiler::ProfilerStatus_Transferring)
    {
		nn::os::SleepThread(nn::TimeSpan::FromMilliSeconds(1));
		status = nn::profiler::GetProfilerStatus();
	}
	
    // Finalize the profiler
    nn::Result result = nn::profiler::Finalize();
    NN_ABORT_UNLESS_RESULT_SUCCESS(result);
	delete [] profiler_buffer;
#endif

	if (core_dynamic_modules_nrr) {
		nn::ro::UnregisterModuleInfo(&core_dynamic_modules_info);
		nn::ro::Finalize();
		free(core_dynamic_modules_nrr);
		core_dynamic_modules_nrr = nullptr;
	}
	// if (ProjectSettings::get_singleton()->get("rendering/shader_compiler/shader_cache/enabled")) {
	// 	unmountCache();
	// }
    unmountRom();
}

bool OS_NX::_check_internal_feature_support(const String &p_feature) {
    return p_feature == "mobile";
}

String OS_NX::get_stdin_string(int64_t p_buffer_size) {
	Vector<uint8_t> data;
	data.resize(p_buffer_size);
	return String::utf8(fgets((char *)data.ptrw(), p_buffer_size, stdin));
}

PackedByteArray OS_NX::get_stdin_buffer(int64_t p_buffer_size) {
	Vector<uint8_t> data;
	data.resize(p_buffer_size);
	fgets((char *)data.ptrw(), p_buffer_size, stdin);
	return data;
}

Vector<String> OS_NX::get_video_adapter_driver_info() const {
	return Vector<String>();
}

Error OS_NX::get_entropy(uint8_t *r_buffer, int p_bytes) {
    nn::crypto::GenerateCryptographicallyRandomBytes(r_buffer, size_t(p_bytes));
    return OK;
}

// Handle object for Switch's dynamic modules (libraries)
// See also: ${NINTENDO_SDK_ROOT}\Samples\Sources\Applications\RoSimple\RoStaticApplication\RoStaticApplication.cpp
struct DynamicLibraryNX {
	uint8_t *nro = nullptr;
	uint8_t *bss = nullptr;
	nn::ro::Module module;
	~DynamicLibraryNX() {
		free(nro); // Never null!
		nro = nullptr;
		if (bss) {
			free(bss);
		}
		bss = nullptr;
	}
};

Error OS_NX::open_dynamic_library(const String &p_path, void *&p_library_handle, GDExtensionData *p_data) {
	ERR_FAIL_COND_V(p_path.get_extension() != "nro", ERR_FILE_UNRECOGNIZED);
	if (!core_dynamic_modules_nrr) {
		// Initialize dynamic modules if applicable.
		String dynamic_modules_path = "res:/.nrr/CoreDynamicModules.nrr";
		ERR_FAIL_COND_V(!FileAccess::exists(dynamic_modules_path), ERR_CANT_CREATE);
		nn::ro::Initialize();
		core_dynamic_modules_nrr = (uint8_t *) aligned_alloc(nn::os::MemoryPageSize, MAX_FILE_SIZE);
		Error err;
		Ref<FileAccess> f = FileAccess::open(dynamic_modules_path, FileAccess::READ, &err);
		if (err != OK) {
			NN_LOG("Failed to open core dynamic module NRR!");
			NN_ABORT();
		}
		size_t core_dynamic_modules_nrr_size = core_dynamic_modules_nrr_size = f->get_buffer(core_dynamic_modules_nrr, MAX_FILE_SIZE);
		NN_UNUSED(core_dynamic_modules_nrr_size);
		nn::Result result = nn::ro::RegisterModuleInfo(&core_dynamic_modules_info, core_dynamic_modules_nrr);
		if (result.IsFailure()) {
			NN_LOG("Failed to register core dynamic module NRR!");
			NN_ABORT();
		}
		f->close();
	}

	Error err;
    uint8_t *nro;
	{
		Ref<FileAccess> f = FileAccess::open(p_path, FileAccess::READ, &err);
		ERR_FAIL_COND_V(err != OK, err);
		nro = (uint8_t *) aligned_alloc(nn::os::MemoryPageSize, MAX_FILE_SIZE);
		size_t nro_size = f->get_buffer(nro, MAX_FILE_SIZE);
		NN_UNUSED(nro_size);
		f->close();
	}

	DynamicLibraryNX *lib = memnew(DynamicLibraryNX);
	lib->nro = nro;
	size_t buffer_size;
	{
		nn::Result result = nn::ro::GetBufferSize(&buffer_size, nro);
		if (result.IsFailure()) {
			memdelete(lib);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("Failed to load NRO module \"%s\" (Description:%d)", p_path, result.GetDescription()));
		}
		if (buffer_size != 0)
		{
			lib->bss = (uint8_t *) aligned_alloc(nn::os::MemoryPageSize, buffer_size);
		}
	}
    
    {
        // Load the NRO file (delayed resolution for the symbols).
		// TODO: Make it possible to control the bind flag here via compile flags?
        nn::Result result = nn::ro::LoadModule(&lib->module, lib->nro, lib->bss, buffer_size, nn::ro::BindFlag_Lazy);
        if (result.IsFailure()) {
			memdelete(lib);
			ERR_FAIL_V_MSG(ERR_CANT_CREATE, vformat("Failed to load NRO module \"%s\" (Description:%d)", p_path, result.GetDescription()));
		}
        // You can now call the symbols inside the dynamic module.
    }

	if (p_data) {
		// Implementation based on OS_Unix::open_dynamic_library.
		// TODO: Should there be special path resolution?
		if (p_data->r_resolved_path != nullptr) {
			*p_data->r_resolved_path = p_path;
		}
	}

	p_library_handle = lib;
	return OK;
}

Error OS_NX::close_dynamic_library(void *p_library_handle) {
	DynamicLibraryNX *lib = reinterpret_cast<DynamicLibraryNX *>(p_library_handle);
	nn::ro::UnloadModule(&lib->module);
	memdelete(lib);
	return OK;
}

Error OS_NX::get_dynamic_library_symbol_handle(void *p_library_handle, const String &p_name, void *&p_symbol_handle, bool p_optional) {
	DynamicLibraryNX *lib = reinterpret_cast<DynamicLibraryNX *>(p_library_handle);
	uintptr_t symbol;
	nn::Result result = nn::ro::LookupModuleSymbol(&symbol, &lib->module, p_name.utf8().get_data());
	ERR_FAIL_COND_V_MSG(result.IsFailure(), ERR_CANT_RESOLVE, vformat("Failed to get dynamic library symbol \"%s\" (Description:%d)", p_name, result.GetDescription()));
	p_symbol_handle = reinterpret_cast<void *>(symbol);
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
#ifdef PROFILER_ENABLED
		nn::profiler::RecordHeartbeat(nn::profiler::Heartbeats_Main);
#endif
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

String OS_NX::get_cache_path() const {
	// CACHE directory
	
	if(!GLOBAL_GET("rendering/shader_compiler/shader_cache/read_only")) {
		return get_user_data_dir();
	} else {
		return get_resource_dir();
	}
}

int OS_NX::get_process_exit_code(const ProcessID &p_pid) const {
	return -1;
}