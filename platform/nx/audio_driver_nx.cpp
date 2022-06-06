#include "audio_driver_nx.h"
#include "core/config/project_settings.h"

#include <nn/init/init_Malloc.h>


AudioDriverNX::AudioDriverNX()
{
    
}

AudioDriverNX::~AudioDriverNX()
{

}

void AudioDriverNX::thread_func(void *userData)
{
    AudioDriverNX *ad = static_cast<AudioDriverNX *>(userData);
    nn::audio::StartAudioOut(&ad->audioOut);
    while (!ad->exit_thread) {

        // Wait for a audioOutBuffer to get released
        ad->systemEvent.Wait();

        // Get the buffer that completed playback.
        nn::audio::AudioOutBuffer* pAudioOutBuffer = nullptr;

        pAudioOutBuffer = nn::audio::GetReleasedAudioOutBuffer(&ad->audioOut);
        while (pAudioOutBuffer) {
            ad->lock();
            ad->start_counting_ticks();

            int buffer_length = ad->period_size * ad->channels;
            int16_t *outBuffer = static_cast<int16_t*>(nn::audio::GetAudioOutBufferDataPointer(pAudioOutBuffer));
            // do work
            if (!ad->active) {
                // just write 0's to the output buffer
                for (unsigned int i = 0; i < buffer_length; i++) {
                    *(outBuffer + i) = 0;
			    }
            } else {
                // Get the data from the audio_server_process
                // Then write them to the output buffer
                ad->audio_server_process(ad->period_size, ad->samples_in.ptrw());

			    for (unsigned int i = 0; i < buffer_length; i++) {
                    *(outBuffer + i) = ad->samples_in[i] >> 16;
			    }
            }
            nn::audio::AppendAudioOutBuffer(&ad->audioOut, pAudioOutBuffer);
            pAudioOutBuffer = nn::audio::GetReleasedAudioOutBuffer(&ad->audioOut);
            ad->stop_counting_ticks();
            ad->unlock();
        }
    }
    nn::audio::StopAudioOut(&ad->audioOut);
    ad->thread_exited = true;
}

const char *AudioDriverNX::get_name() const
{
    return "NintendoSwitchAudio";
}

Error AudioDriverNX::init()
{
    // Get list of available audio outputs
    {
        nn::audio::AudioOutInfo audioOutInfos[nn::audio::AudioOutCountMax];
        const int count = nn::audio::ListAudioOuts(audioOutInfos, sizeof(audioOutInfos) / sizeof(*audioOutInfos));
        for (int i = 0; i < count; ++i)
        {
            // Verify that open and close can be performed.
            nn::audio::AudioOutParameter parameter;
            nn::audio::InitializeAudioOutParameter(&parameter);
            if (nn::audio::OpenAudioOut(&audioOut, audioOutInfos[i].name, parameter).IsSuccess()) {
                audioOutputDeviceList.push_back(audioOutInfos[i].name);
            }
            nn::audio::CloseAudioOut(&audioOut);
        }
    }

    mix_rate = GLOBAL_GET("audio/driver/mix_rate");

    // Open audio output with the specified sampling rate and number of channels.
    // If the specified sampling rate is not supported and the process fails, the default sampling rate is used.
    nn::audio::AudioOutParameter parameter;
    nn::audio::InitializeAudioOutParameter(&parameter);
    parameter.sampleRate = mix_rate;
    parameter.channelCount = 0;
    // parameter.channelCount = 6;  // For 5.1ch output, specify 6 for the number of channels.
    if (nn::audio::OpenDefaultAudioOut(&audioOut, &systemEvent, parameter).IsFailure()) {
        
        parameter.sampleRate = 0;
        parameter.channelCount = 0;
        if (nn::audio::OpenDefaultAudioOut(&audioOut, &systemEvent, parameter).IsFailure()) {
            return ERR_CANT_OPEN;
        };
    }

    // Get the audio output properties.
    channels = nn::audio::GetAudioOutChannelCount(&audioOut);
    mix_rate = nn::audio::GetAudioOutSampleRate(&audioOut);
    sampleFormat = nn::audio::GetAudioOutSampleFormat(&audioOut);

    // Setup the output buffers
    int latency = GLOBAL_GET("audio/driver/output_latency");
    const int frameSampleCount = latency * mix_rate / 1000;
    period_size = frameSampleCount;
    const size_t dataSize = channels * frameSampleCount * nn::audio::GetSampleByteSize(sampleFormat);
    const size_t bufferSize = nn::util::align_up(dataSize, nn::audio::AudioOutBuffer::SizeGranularity);

	samples_in.resize(frameSampleCount * channels);

    for (int i = 0; i < AUDIO_BUFFERS; ++i) {
        outBuffer[i] = static_cast<int16_t*>(nn::init::GetAllocator()->Allocate(bufferSize, nn::audio::AudioOutBuffer::AddressAlignment));
        nn::audio::SetAudioOutBufferInfo(&audioOutBuffer[i], outBuffer[i], bufferSize, dataSize);
        memset(outBuffer[i], 0, bufferSize);
        nn::audio::AppendAudioOutBuffer(&audioOut, &audioOutBuffer[i]);
    }

    active = false;
    thread_exited = false;
    exit_thread = false;
    thread.start(AudioDriverNX::thread_func, this);

    return OK;
}

void AudioDriverNX::start()
{
    active = true;
}

void AudioDriverNX::finish()
{
    if (thread.is_started()) {
        exit_thread = true;
        thread.wait_to_finish();
    }

    // Cleanup device
    //nn::audio::StopAudioOut(&audioOut);
    nn::audio::CloseAudioOut(&audioOut);
    for (int i = 0; i < AUDIO_BUFFERS; ++i)
        nn::init::GetAllocator()->Free(outBuffer[i]);

    nn::os::DestroySystemEvent(systemEvent.GetBase());
}

int AudioDriverNX::get_mix_rate() const
{
    return mix_rate;
}

AudioDriver::SpeakerMode AudioDriverNX::get_speaker_mode() const
{
    return get_speaker_mode_by_total_channels(channels);
}

void AudioDriverNX::lock()
{
    if (!thread.is_started())
        return;
    mutex.lock();
}

void AudioDriverNX::unlock()
{
    if (!thread.is_started())
        return;
    mutex.unlock();
}
