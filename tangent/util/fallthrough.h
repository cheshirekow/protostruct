#pragma once
// Copyright 2021 Josh Bialkowski <josh.bialkowski@gmail.com>

#ifdef __cplusplus

#if defined(__clang__) && defined(__has_cpp_attribute) && \
    __has_cpp_attribute(clang::fallthrough)

#define TANGENT_FALLTHROUGH [[clang::fallthrough]];  // NOLINT

#elif defined(__GNUC__) && __GNUC__ > 6

#define TANGENT_FALLTHROUGH [[gnu::fallthrough]];  // NOLINT

#endif  // defined(__clang__), defined(__GNUC__)

#else  // __cplusplus

#if __GNUC__ >= 7
#define TANGENT_FALLTHROUGH __attribute__((fallthrough));
#endif

#endif  // __cplusplus

#ifndef TANGENT_FALLTHROUGH
#define TANGENT_FALLTHROUGH
#endif
