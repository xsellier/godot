/*************************************************************************/
/*  audio_driver_osx.h                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2020 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2020 Godot Engine contributors (cf. AUTHORS.md).   */
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
#ifdef OSX_ENABLED

#ifndef AUDIO_DRIVER_OSX_H
#define AUDIO_DRIVER_OSX_H

#include "servers/audio/audio_server_sw.h"

#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/AudioHardware.h>

class AudioDriverOSX : public AudioDriverSW {

	AudioComponentInstance audio_unit;
	AudioObjectPropertyAddress outputDeviceAddress;
	bool active;
	Mutex *mutex;

	int mix_rate;
	int channels;
	int buffer_frames;

	Vector<int32_t> samples_in;

	static OSStatus output_callback(void *inRefCon,
			AudioUnitRenderActionFlags *ioActionFlags,
			const AudioTimeStamp *inTimeStamp,
			UInt32 inBusNumber, UInt32 inNumberFrames,
			AudioBufferList *ioData);

	Error initDevice();
	Error finishDevice();

public:
	const char *get_name() const {
		return "AudioUnit";
	};

	virtual Error init();
	virtual void start();
	virtual int get_mix_rate() const;
	virtual OutputFormat get_output_format() const;
	virtual void lock();
	virtual void unlock();
	virtual void finish();

	bool try_lock();
	Error reopen();

	AudioDriverOSX();
	~AudioDriverOSX();
};

#endif

#endif
