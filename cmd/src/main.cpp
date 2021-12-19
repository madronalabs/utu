//
// Copyright (c) 2021 Greg Wuller.
//
// SPDX-License-Identifier: GPL-2.0-or-later
//

#include <docopt.h>
#include <loris/AiffFile.h>
#include <loris/Analyzer.h>
#include <loris/Channelizer.h>
#include <loris/Distiller.h>
#include <loris/FrequencyReference.h>
#include <loris/PartialList.h>
#include <loris/SdifFile.h>

#include <iostream>
#include <optional>
#include <string>

#include "utu/utu.h"
#include "utu/version.h"

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

static const char USAGE[] =
    R"(utu

    Usage:
      utu analyze <audio_file> [options] [--output=<partial_file>]
      utu synth <partials_file>
      utu (-h | --help)
      utu --version

    General Options:
      -h --help     Show this screen.
      --version     Show version.

    Analyze Options:
      -o, --output=<partial_file>  write analysis result
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

  if (args["analyze"])
  {
    return AnalyzeCommand(args);
  }
  else if (args["synth"])
  {
    return SynthCommand(args);
  }

  return -1;
}

int AnalyzeCommand(Args& args)
{
  auto sourcePath = args["<audio_file>"].asString();
  Loris::AiffFile f(sourcePath);

  double resolutionHz =
      checkAboveZero(vtod(args["--freq-res"]), "--freq-res must be greater than 0");
  double windowWidthHz = vtod(args["--window-width"]).value();

  Loris::Analyzer a(resolutionHz, windowWidthHz);

  //
  // configure analysis options
  //

  auto freqDrift = args["--freq-drift"];
  if (freqDrift)
  {
    a.setFreqDrift(checkAboveZero(vtod(freqDrift), "--freq-drift must be greater than 0"));
  }

  auto freqFloor = args["--freq-floor"];
  if (freqFloor)
  {
    a.setFreqFloor(checkAboveZero(vtod(freqFloor), "--freq-floor must be greater than 0"));
  }

  auto ampFloor = args["--amp-floor"];
  if (ampFloor)
  {
    a.setAmpFloor(vtod(ampFloor).value());
  }

  auto hopTime = args["--hop-time"];
  if (hopTime)
  {
    a.setHopTime(vtod(hopTime).value());
  }

  auto cropTime = args["--crop-time"];
  if (cropTime)
  {
    a.setCropTime(vtod(cropTime).value());
  }

  auto lobeLevel = args["--lobe-level"];
  if (lobeLevel)
  {
    a.setSidelobeLevel(vtod(lobeLevel).value());
  }

  if (args["--no-phase-correct"])
  {
    a.setPhaseCorrect(false);
  }

  //
  // perform analysis
  //

  Loris::PartialList partials = a.analyze(f.samples(), f.sampleRate());
  Loris::FrequencyReference partialsRef(partials.begin(), partials.end(), 415 * 0.8, 415 * 1.2, 50);
  Loris::Channelizer::channelize(partials, partialsRef, 1);
  Loris::Distiller::distill(partials, 0.001);

  std::cout << "Partial count: " << partials.size() << std::endl;

  //
  // output results
  //

  docopt::value outputPath = args["--output"];
  if (outputPath)
  {
    Loris::SdifFile::Export(outputPath.asString(), partials);
    std::cout << "Wrote: " << outputPath << std::endl;
  }

  return 0;
}

int SynthCommand(Args& args)
{
  auto partialPath = args["<sdif>"].asString();

  return 0;
}

std::optional<double> vtod(const docopt::value& v) noexcept
{
  try
  {
    return std::stod(v.asString());
  }
  catch (...)
  {
  }
  return {};
}

template <typename T, typename Predicate>
T check(std::optional<T> n, Predicate predicate, const char* message)
{
  if (n && predicate(n.value()))
  {
    return n.value();
  }
  std::cerr << message << std::endl;
  exit(-1);
}

template <typename T>
T checkRangeInclusive(std::optional<T> n, T minimum, T maximum, const char* message)
{
  if (n && n.value() >= minimum && n.value() <= maximum)
  {
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
