#include <iostream>

#include "utu/utu.h"
#include "utu/version.h"

namespace utu
{

void herald() { std::cout << PROJECT_NAME << " " << PROJECT_VERSION << std::endl; }

}  // namespace utu