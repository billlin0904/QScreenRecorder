#pragma once

#include <qsystemdetection.h>

#if defined(Q_OS_WIN32)
#include "fastscreencapture_win.h"
#elif defined(Q_OS_LINUX)
#include "fastscreencapture_linux.h"
#endif
