#pragma once

#include <qsystemdetection.h>

#if defined(Q_OS_WIN32)
#include "module_win.h"
#elif defined(Q_OS_LINUX)
#include "module_linux.h"
#endif
