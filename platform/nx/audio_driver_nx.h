#ifndef AUDIO_DRIVER_NX_H
#define AUDIO_DRIVER_NX_H

#include "servers/audio_server.h"
#include "core/os/mutex.h"
#include "core/os/thread.h"

#include <nn/audio.h>

class AudioDriverNX : public AudioDriver {

	enum {
		AUDIO_BUFFERS = 2
	};

    Mutex mutex;
    Thread thread;

    bool active = false;
    bool thread_exited = false;
    bool exit_thread = false;
    nn::audio::AudioOut audioOut;
    nn::os::SystemEvent systemEvent;
    nn::audio::AudioOutBuffer audioOutBuffer[AUDIO_BUFFERS];

    Array audioOutputDeviceList;

    int mix_rate = 48000;
	unsigned int channels = 2;
    int period_size = 0;
    nn::audio::SampleFormat sampleFormat;

    Vector<int32_t> samples_in;
    int16_t* outBuffer[AUDIO_BUFFERS];
    

public:
    AudioDriverNX();
    ~AudioDriverNX() override;

	const char *get_name() const override;

	Error init() override;
	void start() override;
	int get_mix_rate() const override;
	AudioDriver::SpeakerMode get_speaker_mode() const override;
	void lock() override;
	void unlock() override;
	void finish() override;

    static void thread_func(void *userData);
};

#endif //AUDIO_DRIVER_NX_H