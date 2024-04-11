#ifndef SLED_LANG_ATTRIBUTES_H
#define SLED_LANG_ATTRIBUTES_H
#pragma once

// #define SLED_DEPRECATED __attribute__((deprecated))
#define SLED_NODISCARD [[nodiscard]]
#define SLED_DEPRECATED [[deprecated]]

#if defined(__clang__) && (!defined(SWIG))
#define SLED_THREAD_ANNOTATION_ATTRIBUTE__(x) __attribute__((x))
#else
#define SLED_THREAD_ANNOTATION_ATTRIBUTE__(x)// no-op
#endif

#define SLED_UNUSED(x) (void) (x)

#define SLED_CAPABILITY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SLED_SCOPED_CAPABILITY SLED_THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)
#define SLED_LOCKABLE SLED_THREAD_ANNOTATION_ATTRIBUTE__(lockable)

#define SLED_GUARDED_BY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define SLED_PT_GUARDED_BY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define SLED_ACQUIRED_BEFORE(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define SLED_ACQUIRED_AFTER(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define SLED_REQUIRES(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define SLED_REQUIRES_SHARED(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define SLED_ACQUIRE(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define SLED_ACQUIRE_SHARED(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define SLED_RELEASE(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define SLED_RELEASE_SHARED(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define SLED_TRY_ACQUIRE(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define SLED_TRY_ACQUIRE_SHARED(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define SLED_EXCLUDES(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define SLED_ASSERT_CAPABILITY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define SLED_ASSERT_SHARED_CAPABILITY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define SLED_RETURN_CAPABILITY(x) SLED_THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define SLED_NO_THREAD_SAFETY_ANALYSIS SLED_THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

#define SLED_EXCLUSIVE_LOCK_FUNCTION(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(exclusive_lock_function(__VA_ARGS__))
#define SLED_SHARED_LOCK_FUNCTION(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(shared_lock_function(__VA_ARGS__))
#define SLED_EXCLUSIVE_TRYLOCK_FUNCTION(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(exclusive_trylock_function(__VA_ARGS__))
#define SLED_SHARED_TRYLOCK_FUNCTION(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(shared_trylock_function(__VA_ARGS__))
#define SLED_UNLOCK_FUNCTION(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(unlock_function(__VA_ARGS__))
#define SLED_ASSERT_EXCLUSIVE_LOCK(...) SLED_THREAD_ANNOTATION_ATTRIBUTE__(assert_exclusive_lock(__VA_ARGS__))

#define SLED_NO_INSTRUMENT_FUNCTION SLED_THREAD_ANNOTATION_ATTRIBUTE__(no_instrument_function)

#endif//  SLED_LANG_ATTRIBUTES_H
