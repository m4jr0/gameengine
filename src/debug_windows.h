// Copyright 2021 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_DEBUG_WINDOWS_H_
#define COMET_DEBUG_WINDOWS_H_

#ifdef _WIN32
#define _CRTDBG_MAP_ALLOC

#include <crtdbg.h>

#include <cstdlib>

#ifdef _DEBUG
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif  // DBG_NEW
#endif  // _DEBUG
#endif  // _WIN32

#endif  // COMET_DEBUG_WINDOWS_H_
