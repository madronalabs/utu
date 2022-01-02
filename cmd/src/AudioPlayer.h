//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <RtAudio.h>
#pragma GCC diagnostic pop

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>

class AudioPlayer final
{
 public:
  using Samples = std::vector<double>;

  AudioPlayer(const Samples& samples, uint32_t sampleRate)
      : _samples(samples), _sampleRate(sampleRate), _playbackOffset(0), _blocksOutput(0)
  {
  }

  int play(bool verbose = true)
  {
    using namespace std::chrono_literals;

    _playbackOffset = 0;
    _blocksOutput = 0;

    int status = 0;

    RtAudio dac(RtAudio::UNSPECIFIED, &_errorCallback);

    if (dac.getDeviceCount() < 1) {
      std::cout << "error: No audio devices found\n";
      return -1;
    }

    dac.showWarnings();

    RtAudio::StreamParameters params;
    params.deviceId = dac.getDefaultOutputDevice();
    params.nChannels = 1;
    params.firstChannel = 0;

    RtAudio::StreamOptions options;
    options.flags = 0;
    options.flags = RTAUDIO_HOG_DEVICE;
    options.flags |= RTAUDIO_SCHEDULE_REALTIME;

    unsigned int bufferFrames = 512;  // size of output block

    auto info = dac.getDeviceInfo(params.deviceId);
    if (verbose) {
      std::cout << "Output Device: " << info.name << " (sr: " << info.preferredSampleRate
                << " ch: " << info.outputChannels << ")\n";
    }

    if (info.preferredSampleRate != _sampleRate) {
      // TODO: provide sample rate conversion?

      // NOTE: bail if the sample rate of the playback device doesn't match the
      // input. Forcing the audio device to change sample rate causes problems
      // anywhere from disruption of playback in other applications to
      // destabalizing the audio system if the host is not in control of the
      // sample rate for external hardware.
      std::cout << "error: Output device sr: " << info.preferredSampleRate
                << " does not match sample rate of playback material\n";
      return -1;
    }

    if (dac.openStream(&params, nullptr /* input options */, RTAUDIO_FLOAT64,
                       info.preferredSampleRate, &bufferFrames, &_audioCallback, this, &options)) {
      status = -200;
      goto cleanup;
    }

    if (!dac.isStreamOpen()) {
      status = -201;
      goto cleanup;
    }

    if (dac.startStream()) {
      status = -202;
      goto cleanup;
    }

    std::cerr << "Playing...";
    while (dac.isStreamRunning()) {
      std::this_thread::sleep_for(500ms);
      std::cerr << ".";
    }
    std::cerr << "done.\n";

    if (verbose) {
      std::cerr << "Output Blocks: " << _blocksOutput << std::endl;
    }

  cleanup:
    if (dac.isStreamOpen()) {
      dac.closeStream();
    }

    return status;
  }

 private:
  const Samples& _samples;
  const uint32_t _sampleRate;

  uint64_t _playbackOffset;
  uint64_t _blocksOutput;

  int _output(void* outputBuffer, unsigned int nFrames)
  {
    uint64_t framesRemaining = _samples.size() - _playbackOffset;
    uint64_t framesToCopy = std::min(static_cast<uint64_t>(nFrames), framesRemaining);

    std::memcpy(outputBuffer, &_samples[_playbackOffset],
                framesToCopy * sizeof(Samples::value_type));

    _playbackOffset += framesToCopy;
    framesRemaining = _samples.size() - _playbackOffset;
    _blocksOutput += 1;
    return framesRemaining > 0 ? 0 /* keep requesting */ : 1 /* drain the buffer and stop */;
  }

  static void _errorCallback(RtAudioErrorType /* type */, const std::string& error)
  {
    std::cerr << "error: " << error << "\n";
  }

  static int _audioCallback(void* outputBuffer, void* /* inputBuffer */, unsigned int nFrames,
                            double /* streamTime */, RtAudioStreamStatus /* status */,
                            void* userData)
  {
    return reinterpret_cast<AudioPlayer*>(userData)->_output(outputBuffer, nFrames);
  }
};