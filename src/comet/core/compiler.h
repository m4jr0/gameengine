// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_COMPILER_H_
#define COMET_COMET_CORE_COMPILER_H_

#ifdef __clang__
#define COMET_CLANG
#elif defined(__GNUC__)
#define COMET_GCC
#ifdef __GNUG__
#define COMET_GCC_CPP
#else
#define COMET_GCC_C
#endif  // __GNUG__
#elif defined(_MSC_VER)
#define COMET_MSVC
#endif  // __clang__

#ifdef __cplusplus
#define COMET_CPP
#else
#define COMET_C
#endif  // __cplusplus

#endif  // COMET_COMET_CORE_COMPILER_H_