#include <docopt.h>
#include <loris/AiffFile.h>
#include <loris/Analyzer.h>
#include <loris/Channelizer.h>
#include <loris/Distiller.h>
#include <loris/FrequencyReference.h>
#include <loris/PartialList.h>
#include <loris/SdifFile.h>

#include <iostream>

#include "utu/utu.h"
#include "utu/version.h"

using Args = std::map<std::string, docopt::value>;

int AnalyzeCommand(Args& args);
int SynthCommand(Args& args);

static const char USAGE[] =
    R"(utu

    Usage:
      utu analyze <audio_file> [--output=<sdif_file>]
      utu synth <partials_file>
      utu (-h | --help)
      utu --version

    Options:
      -h --help     Show this screen.
      --version     Show version.
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

  Loris::Analyzer a(415 * 0.8, 415 * 1.6);
  a.setFreqDrift(30);
  a.setAmpFloor(-90);

  Loris::PartialList partials = a.analyze(f.samples(), f.sampleRate());
  Loris::FrequencyReference partialsRef(partials.begin(), partials.end(), 415 * 0.8, 415 * 1.2, 50);
  Loris::Channelizer::channelize(partials, partialsRef, 1);
  Loris::Distiller::distill(partials, 0.001);

  std::cout << "Partial count: " << partials.size() << std::endl;

  docopt::value outputPath = args["--output"];
  if (outputPath)
  {
    Loris::SdifFile::Export(outputPath.asString(), partials);
    std::cout << "Wrote: " << outputPath << std::endl;
  }

  return 0;
}

int SynthCommand(Args& args) { return 0; }