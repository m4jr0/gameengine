// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_CORE_WINDOWS_H_
#define COMET_CORE_WINDOWS_H_

#ifdef COMET_MSVC
#define NOMINMAX
#include <windows.h>

// Undef annoying Windows macros...
#undef CopyMemory
#undef MoveMemory
#undef CreateFile
#undef CreateDirectory
#undef GetCurrentDirectory
#undef GetCurrentTime
#undef Yield

#ifdef COMET_DEBUG
#include <dbghelp.h>
#endif  // COMET_DEBUG
#endif  // COMET_MSVC

#endif  // COMET_CORE_WINDOWS_H_
