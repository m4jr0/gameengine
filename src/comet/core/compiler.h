// Copyright 2025 m4jr0. All Rights Reserved.
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

#ifdef COMET_GCC
#ifdef __x86_64__
#define COMET_ARCH_X86_64
#define COMET_ARCH_X86
#elif defined(__i386__)
#define COMET_ARCH_X86_32
#define COMET_ARCH_X86
#elif defined(__arm__)
#define COMET_ARCH_ARM
#else
static_assert(false, "Unsupported architecture.");
#endif  // __x86_64__
#endif  // COMET_GCC

#ifdef COMET_MSVC
#ifdef _M_X64
#define COMET_ARCH_X86_64
#define COMET_ARCH_X86
#elif defined(_M_IX86)
#define COMET_ARCH_X86_32
#define COMET_ARCH_X86
#elif defined(_M_ARM)
#define COMET_ARCH_ARM
#else
static_assert(false, "Unsupported architecture.");
#endif  // _M_X64
#endif  // COMET_MSVC

#ifdef COMET_MSVC
#define COMET_FORCE_NOT_INLINE __declspec(noinline)
#else
#if __has_attribute(optnone)
#define COMET_FORCE_NOT_INLINE __attribute__((optnone))
#elif __has_attribute(noinline)
#define COMET_FORCE_NOT_INLINE __attribute__((noinline))
#else
#define COMET_FORCE_NOT_INLINE
#endif
#endif  // COMET_MSVC

#endif  // COMET_COMET_CORE_COMPILER_H_