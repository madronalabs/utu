#include <iostream>

#include "docopt.h"
#include "utu/utu.h"
#include "utu/version.h"

static const char USAGE[] =
    R"(utu

    Usage:
      utu analyze <audio_file>
      utu synth <partials_file>
      utu (-h | --help)
      utu --version

    Options:
      -h --help     Show this screen.
      --version     Show version.
)";

int main(int argc, const char** argv)
{
  std::map<std::string, docopt::value> args = docopt::docopt(USAGE, {argv + 1, argv + argc},
                                                             true,  // show help if requested
                                                             PROJECT_VERSION);  // version string

  for (auto const& arg : args)
  {
    std::cout << arg.first << arg.second << std::endl;
  }

  return 0;
}
