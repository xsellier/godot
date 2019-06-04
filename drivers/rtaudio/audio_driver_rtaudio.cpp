/*************************************************************************/
/*  audio_driver_rtaudio.cpp                                             */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "audio_driver_rtaudio.h"

#include "os/os.h"
#include "project_settings.h"

#ifdef RTAUDIO_ENABLED

const char *AudioDriverRtAudio::get_name() const {

#ifdef OSX_ENABLED
	return "RtAudio-OSX";
#elif defined(UNIX_ENABLED)
	return "RtAudio-ALSA";
#elif defined(WINDOWS_ENABLED)
	return "RtAudio-DirectSound";
#else
	return "RtAudio-None";
#endif
}

int AudioDriverRtAudio::callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData) {

	if (status) {
		if (status & RTAUDIO_INPUT_OVERFLOW) {
			WARN_PRINT("RtAudio input overflow!");
		}
		if (status & RTAUDIO_OUTPUT_UNDERFLOW) {
			WARN_PRINT("RtAudio output underflow!");
		}
	}
	int32_t *buffer = (int32_t *)outputBuffer;

	AudioDriverRtAudio *self = (AudioDriverRtAudio *)userData;

	if (self->mutex->try_lock() != OK) {
		// what should i do..
		for (unsigned int i = 0; i < nBufferFrames; i++)
			buffer[i] = 0;

		return 0;
	}

	self->audio_server_process(nBufferFrames, buffer);

	self->mutex->unlock();

	return 0;
}

Error AudioDriverRtAudio::init() {

	active = false;
	mutex = Mutex::create(true);
	dac = memnew(RtAudio);

	ERR_EXPLAIN("Cannot initialize RtAudio audio driver: No devices present.")
	ERR_FAIL_COND_V(dac->getDeviceCount() < 1, ERR_UNAVAILABLE);

	RtAudio::StreamParameters parameters;
	parameters.deviceId = dac->getDefaultOutputDevice();
	RtAudio::StreamOptions options;

	// set the desired numberOfBuffers
	options.numberOfBuffers = 4;

	parameters.firstChannel = 0;
	mix_rate = GLOBAL_DEF("audio/mix_rate", 44100);

	int latency = GLOBAL_DEF("audio/output_latency", 25);
	// calculate desired buffer_size
	unsigned int buffer_size = closest_power_of_2(latency * mix_rate / 1000);

	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("audio buffer size: " + itos(buffer_size));
	}

	short int tries = 3;

	while (tries >= 0) {
		switch (output_format) {
			case OUTPUT_STEREO: parameters.nChannels = 2; break;
			case OUTPUT_QUAD: parameters.nChannels = 4; break;
			case OUTPUT_5_1: parameters.nChannels = 6; break;
			case OUTPUT_7_1: parameters.nChannels = 8; break;
			default:
				parameters.nChannels = 2; break;
		};

		try {
			dac->openStream(&parameters, NULL, RTAUDIO_SINT32, mix_rate, &buffer_size, &callback, this, &options);
			active = true;

			break;
		} catch (RtAudioError &e) {
			// try with less channels
			ERR_PRINT("Unable to open audio, retrying with fewer channels..");

			switch (output_format) {
				case OUTPUT_QUAD: output_format = OUTPUT_STEREO; break;
				case OUTPUT_5_1: output_format = OUTPUT_QUAD; break;
				case OUTPUT_7_1: output_format = OUTPUT_5_1; break;
				default:
					output_format = OUTPUT_STEREO; break;
			}

			tries--;
		}
	}

	return OK;
}

int AudioDriverRtAudio::get_mix_rate() const {

	return mix_rate;
}

AudioDriverSW::OutputFormat AudioDriverRtAudio::get_output_format() const {
	return output_format;
}

void AudioDriverRtAudio::start() {

	if (active)
		dac->startStream();
}

void AudioDriverRtAudio::lock() {

	if (mutex)
		mutex->lock();
}

void AudioDriverRtAudio::unlock() {

	if (mutex)
		mutex->unlock();
}

void AudioDriverRtAudio::finish() {

	lock();
	if (active && dac->isStreamOpen()) {
		dac->closeStream();
		active = false;
	}
	unlock();

	if (mutex) {
		memdelete(mutex);
		mutex = NULL;
	}
	if (dac) {
		memdelete(dac);
		dac = NULL;
	}
}

AudioDriverRtAudio::AudioDriverRtAudio() {

	mutex = NULL;
	dac = NULL;
	mix_rate = 44100;
	output_format = OUTPUT_STEREO;
}

#endif
