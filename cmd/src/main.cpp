//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <docopt.h>
#include <loris/Analyzer.h>
#include <loris/Channelizer.h>
#include <loris/Distiller.h>
#include <loris/FrequencyReference.h>
#include <loris/PartialList.h>
#include <loris/Synthesizer.h>
#include <utu/utu.h>
#include <utu/version.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "AudioFile.h"
#include "AudioPlayer.h"
#include "Marshal.h"

using Args = std::map<std::string, docopt::value>;

std::optional<double> vtod(const docopt::value& v) noexcept;

template <typename T, typename Predicate>
T check(std::optional<T> n, Predicate predicate, const char* message);
template <typename T>
T checkRangeInclusive(std::optional<T> n, T minimum, T maximum, const char* message);
template <typename T>
T checkAboveZero(std::optional<T> n, const char* message);

int AnalyzeCommand(Args& args);
int SynthCommand(Args& args);
int SynthCommandListOutputDevices(Args& args);

static const char USAGE[] =
    R"(utu

    Usage:
      utu analyze <audio_file> [options] [--output=<file>]
      utu synth <partial_file> [options] [--output=<file>]
      utu synth --list-devices
      utu (-h | --help)
      utu --version

    General Options:
      -o, --output=<file>          write analysis/synthesis result
      -h --help                    Show this screen.
      --quiet                      Suppress normal output.
      --version                    Show version.

    Analyze Options:
      --freq-res=<res_hz>          minimum instantaneous frequency
                                   difference [default: 332]
      --freq-drift=<drift_hz>      maximum allowable frequency difference
                                   between consecutive breakpoints [default: 30]
      --freq-floor=<floor_hz>      minimum instantaneous partial frequency
      --amp-floor=<floor_db>       lowest detected spectral amplitude [default: -90]
      --crop-time=<ct>
      --hop-time=<ht>              approximate average density of breakpoints
      --lobe-level=<lobe_db>       sidelobe attenuation level for Kaiser analysis
                                   window in positive dB
      --window-width=<win_hz>      frequency domain lobe width [default: 664]
      --no-phase-correct

    Synth Options:
      --pitch-shift=<cents>        shift the pitch partials [default: 0]
      --sample-rate=<rate>         sample rate [default: 44100]
      --audition                   play result out given audio interface
      --device=<device_num>        play out device other than default output
      --list-devices               list output devices for auditioning
)";

int main(int argc, const char** argv)
{
  Args args = docopt::docopt(USAGE, {argv + 1, argv + argc},
                             true,              // show help if requested
                             PROJECT_VERSION);  // version string

#if 0
  for (auto const& arg : args)
  {
    std::cout << arg.first << arg.second << std::endl;
  }
#endif

  if (args["analyze"].asBool()) {
    return AnalyzeCommand(args);
  } else if (args["synth"].asBool()) {
    return SynthCommand(args);
  }

  return -1;
}

//
// analyze subcommand
//

int AnalyzeCommand(Args& args)
{
  bool quietOutput = args["--quiet"].asBool();

  docopt::value outputPath = args["--output"];
  if (outputPath && outputPath.asString() == "-") {
    // if writing to std::out suppress any logging
    quietOutput = true;
  }

  double resolutionHz =
      checkAboveZero(vtod(args["--freq-res"]), "--freq-res must be greater than 0");
  double windowWidthHz = vtod(args["--window-width"]).value();

  Loris::Analyzer a(resolutionHz, windowWidthHz);

  //
  // configure analysis options
  //

  auto freqDrift = args["--freq-drift"];
  if (freqDrift) {
    a.setFreqDrift(checkAboveZero(vtod(freqDrift), "--freq-drift must be greater than 0"));
  }

  auto freqFloor = args["--freq-floor"];
  if (freqFloor) {
    a.setFreqFloor(checkAboveZero(vtod(freqFloor), "--freq-floor must be greater than 0"));
  }

  auto ampFloor = args["--amp-floor"];
  if (ampFloor) {
    a.setAmpFloor(vtod(ampFloor).value());
  }

  auto hopTime = args["--hop-time"];
  if (hopTime) {
    a.setHopTime(vtod(hopTime).value());
  }

  auto cropTime = args["--crop-time"];
  if (cropTime) {
    a.setCropTime(vtod(cropTime).value());
  }

  auto lobeLevel = args["--lobe-level"];
  if (lobeLevel) {
    a.setSidelobeLevel(vtod(lobeLevel).value());
  }

  if (args["--no-phase-correct"]) {
    a.setPhaseCorrect(false);
  }

  std::string sourcePath = args["<audio_file>"].asString();
  AudioFile f = AudioFile::forRead(sourcePath);
  if (!quietOutput) {
    std::cout << "Source: " << sourcePath << " ch: " << f.channels() << " sr: " << f.sampleRate()
              << " frames: " << f.frames() << std::endl;
  }

  //
  // perform analysis
  //

  Loris::PartialList partials = a.analyze(f.samples(), f.sampleRate());
  Loris::FrequencyReference partialsRef(partials.begin(), partials.end(), 415 * 0.8, 415 * 1.2, 50);
  Loris::Channelizer::channelize(partials, partialsRef, 1);
  Loris::Distiller::distill(partials, 0.001);

  if (!quietOutput) {
    std::cout << "Partials: " << partials.size() << std::endl;
  }

  //
  // output results
  //

  if (outputPath) {
    utu::PartialData data = Marshal::from(partials);
    data.source = utu::PartialData::Source({std::filesystem::canonical(sourcePath), {}});

    if (outputPath.asString() == "-") {
      utu::PartialWriter::write(data, std::cout);
    } else {
      std::ofstream os(outputPath.asString(), std::ios::binary);
      utu::PartialWriter::write(data, os);
      if (!quietOutput) {
        std::cout << "Wrote: " << outputPath << std::endl;
      }
    }
  }

  return 0;
}

//
// synth subcommand
//

int SynthCommand(Args& args)
{
  // special case, --list-devices
  if (args["--list-devices"].asBool()) {
    return SynthCommandListOutputDevices(args);
  }

  std::string partialPath = args["<partial_file>"].asString();

  bool quietOutput = args["--quiet"].asBool();

  std::optional<utu::PartialData> data;
  if (partialPath == "-") {
    data = utu::PartialReader::read(std::cin);
  } else {
    std::ifstream is(partialPath, std::ios::binary);
    data = utu::PartialReader::read(is);
  }

  if (!quietOutput) {
    std::cout << "Partials: " << data->partials.size() << std::endl;
  }

  // convert into native Loris representation
  Loris::PartialList partials = Marshal::from(*data);

  std::optional<double> pitchShift = vtod(args["--pitch-shift"]);
  if (pitchShift && *pitchShift != 0) {
    if (!quietOutput) {
      std::cout << "Shifting pitch by " << *pitchShift << " cents\n";
    }
    Loris::PartialUtils::shiftPitch(partials.begin(), partials.end(), *pitchShift);
  }

  auto sr = static_cast<uint32_t>(args["--sample-rate"].asLong());

  // configure Loris synthesizer paramters
  Loris::Synthesizer::Parameters params;
  params.sampleRate = sr;
  // TODO: fade time

  // perform synthesis
  std::vector<double> samples;
  Loris::Synthesizer synth(params, samples);
  synth.synthesize(partials.begin(), partials.end());

  if (!quietOutput) {
    std::cout << "Calculated: " << samples.size() << " frames, sr: " << sr << std::endl;
  }

  docopt::value outputPath = args["--output"];
  if (outputPath) {
    std::optional<AudioFile::Format> format = AudioFile::inferFormat(outputPath.asString());
    if (!format) {
      std::cout << "error: Unsupported output format; must be .wav, .aiff, or .caf\n";
      return -1;
    }

    AudioFile f = AudioFile::forWrite(outputPath.asString(), sr, 1 /* channel */, *format);
    f.write(samples);

    if (!quietOutput) {
      std::cout << "Wrote: " << outputPath.asString() << std::endl;
    }
  }

  docopt::value audition = args["--audition"];
  if (audition.asBool()) {
    std::optional<uint8_t> outputDevice;
    if (args["--device"]) {
      outputDevice = static_cast<uint8_t>(args["--device"].asLong());
    }

    AudioPlayer player(samples, sr);
    player.play(outputDevice);
  }

  return 0;
}

int SynthCommandListOutputDevices(Args& /* args */)
{
  auto descriptions = AudioPlayer::getOutputDeviceDescriptions();

  if (descriptions.size() == 0) {
    std::cerr << "error: No output devices found\n";
    return -1;
  }

  std::cout << "Available devices:\n\n";
  for (unsigned int index = 0; index < descriptions.size(); ++index) {
    std::cerr << "  " << index << ": " << descriptions[index] << std::endl;
  }

  return 0;
}

//
// Helpers
//

std::optional<double> vtod(const docopt::value& v) noexcept
{
  try {
    return std::stod(v.asString());
  } catch (...) {
  }
  return {};
}

template <typename T, typename Predicate>
T check(std::optional<T> n, Predicate predicate, const char* message)
{
  if (n && predicate(n.value())) {
    return n.value();
  }
  std::cerr << message << std::endl;
  exit(-1);
}

template <typename T>
T checkRangeInclusive(std::optional<T> n, T minimum, T maximum, const char* message)
{
  if (n && n.value() >= minimum && n.value() <= maximum) {
    return n.value();
  }
  std::cerr << message << std::endl;
  exit(-1);
}

template <typename T>
T checkAboveZero(std::optional<T> n, const char* message)
{
  return check(
      n, [](const T& v) { return v > 0; }, message);
}
