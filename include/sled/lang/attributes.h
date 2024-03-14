#pragma once
#ifndef SLED_LANG_ATTRIBUTES_H
#define SLED_LANG_ATTRIBUTES_H

#define SLED_DEPRECATED() __attribute__((deprecated))
#if defined(__GNUC__) && defined(__SUPPORT_TS_ANNOTATION__) && !defined(SWIG)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#elif defined(__clang__)
#define THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)
#endif

#if defined(GUARDED_BY)
#undef GUARDED_BY
#endif
#define GUARDED_BY(x) __attribute__((guarded_by(x)))

#if defined(__clang__)
#define EXCLUSIVE_TRYLOCK_FUNCTION(...)                                        \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))

#define EXCLUSIVE_LOCK_FUNCTION(...)                                           \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))

#define UNLOCK_FUNCTION(...)                                                   \
    THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))

#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#else

#define EXCLUSIVE_TRYLOCK_FUNCTION(...)                                        \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock(__VA_ARGS__))
#define EXCLUSIVE_LOCK_FUNCTION(...)                                           \
    THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock(__VA_ARGS__))
#define UNLOCK_FUNCTION(...) THREAD_ANNOTATION_ATTRIBUTE__(unlock(__VA_ARGS__))
#define PT_GUARDED_BY(x) THREAD_ANNOTATION_ATTRIBUTE__(point_to_guarded_by(x))
#endif

#endif//  SLED_LANG_ATTRIBUTES_H
