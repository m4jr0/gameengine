// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_CORE_TYPE_OS_H_
#define COMET_COMET_CORE_TYPE_OS_H_

#ifdef _WIN32
#define COMET_WINDOWS
#endif  // _WIN32

#ifdef __linux__
#define COMET_LINUX
#endif  // __linux__

#ifdef __APPLE__
#define COMET_APPLE
#endif  // __APPLE__

#if defined __linux__ || defined __APPLE__ || defined __sun || \
    defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__
#define COMET_UNIX
#endif  // __linux__ || defined __APPLE__ || defined __sun || __FreeBSD__ ||
        // defined __NetBSD__ || defined __OpenBSD__

#ifdef COMET_WINDOWS
#ifdef _WIN64
#define COMET_64
#else
#define COMET_32
#endif  // _WIN64
#endif  // COMET_WINDOWS

// Check GCC
#if defined COMET_UNIX || defined COMET_APPLE
#if __x86_64__ || __ppc64__
#define COMET_64
#else
#define COMET_32
#endif
#endif  // defined COMET_UNIX || defined COMET_APPLE

#endif  // COMET_COMET_CORE_TYPE_OS_H_