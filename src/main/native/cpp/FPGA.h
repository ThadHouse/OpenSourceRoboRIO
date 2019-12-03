#pragma once

#include "NiFpga_OpenSourceRIO.h"
#include "NiFpga.h"

namespace hal {
namespace internal {
  extern NiFpga_Session FPGASession;
}
}

using namespace hal::internal;
