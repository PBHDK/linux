/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Macros and attributes for compiler-based static capability analysis.
 */

#ifndef _LINUX_COMPILER_CAPABILITY_ANALYSIS_H
#define _LINUX_COMPILER_CAPABILITY_ANALYSIS_H

#if defined(WARN_CAPABILITY_ANALYSIS)

/*
 * The below attributes are used to define new capability types. Internal only.
 */
# define __cap_type(name)			__attribute__((capability(#name)))
# define __reentrant_cap			__attribute__((reentrant_capability))
# define __acquires_cap(...)			__attribute__((acquire_capability(__VA_ARGS__)))
# define __acquires_shared_cap(...)		__attribute__((acquire_shared_capability(__VA_ARGS__)))
# define __try_acquires_cap(ret, var)		__attribute__((try_acquire_capability(ret, var)))
# define __try_acquires_shared_cap(ret, var)	__attribute__((try_acquire_shared_capability(ret, var)))
# define __releases_cap(...)			__attribute__((release_capability(__VA_ARGS__)))
# define __releases_shared_cap(...)		__attribute__((release_shared_capability(__VA_ARGS__)))
# define __assumes_cap(...)			__attribute__((assert_capability(__VA_ARGS__)))
# define __assumes_shared_cap(...)		__attribute__((assert_shared_capability(__VA_ARGS__)))
# define __returns_cap(var)			__attribute__((lock_returned(var)))

/*
 * The below are used to annotate code being checked. Internal only.
 */
# define __excludes_cap(...)		__attribute__((locks_excluded(__VA_ARGS__)))
# define __requires_cap(...)		__attribute__((requires_capability(__VA_ARGS__)))
# define __requires_shared_cap(...)	__attribute__((requires_shared_capability(__VA_ARGS__)))

/**
 * __guarded_by - struct member and globals attribute, declares variable
 *                protected by capability
 *
 * Declares that the struct member or global variable must be guarded by the
 * given capabilities. Read operations on the data require shared access,
 * while write operations require exclusive access.
 *
 * .. code-block:: c
 *
 *	struct some_state {
 *		spinlock_t lock;
 *		long counter __guarded_by(&lock);
 *	};
 */
# define __guarded_by(...)		__attribute__((guarded_by(__VA_ARGS__)))

/**
 * __pt_guarded_by - struct member and globals attribute, declares pointed-to
 *                   data is protected by capability
 *
 * Declares that the data pointed to by the struct member pointer or global
 * pointer must be guarded by the given capabilities. Read operations on the
 * data require shared access, while write operations require exclusive access.
 *
 * .. code-block:: c
 *
 *	struct some_state {
 *		spinlock_t lock;
 *		long *counter __pt_guarded_by(&lock);
 *	};
 */
# define __pt_guarded_by(...)		__attribute__((pt_guarded_by(__VA_ARGS__)))

/**
 * struct_with_capability() - declare or define a capability struct
 * @name: struct name
 *
 * Helper to declare or define a struct type with capability of the same name.
 *
 * .. code-block:: c
 *
 *	struct_with_capability(my_handle) {
 *		int foo;
 *		long bar;
 *	};
 *
 *	struct some_state {
 *		...
 *	};
 *	// ... declared elsewhere ...
 *	struct_with_capability(some_state);
 *
 * Note: The implementation defines several helper functions that can acquire,
 * release, and assert the capability.
 */
# define struct_with_capability(name, ...)								\
	struct __cap_type(name) __VA_ARGS__ name;							\
	static __always_inline void __acquire_cap(const struct name *var)				\
		__attribute__((overloadable)) __no_capability_analysis __acquires_cap(var) { }		\
	static __always_inline void __acquire_shared_cap(const struct name *var)			\
		__attribute__((overloadable)) __no_capability_analysis __acquires_shared_cap(var) { }	\
	static __always_inline bool __try_acquire_cap(const struct name *var, bool ret)			\
		__attribute__((overloadable)) __no_capability_analysis __try_acquires_cap(1, var)	\
	{ return ret; }											\
	static __always_inline bool __try_acquire_shared_cap(const struct name *var, bool ret)		\
		__attribute__((overloadable)) __no_capability_analysis __try_acquires_shared_cap(1, var) \
	{ return ret; }											\
	static __always_inline void __release_cap(const struct name *var)				\
		__attribute__((overloadable)) __no_capability_analysis __releases_cap(var) { }		\
	static __always_inline void __release_shared_cap(const struct name *var)			\
		__attribute__((overloadable)) __no_capability_analysis __releases_shared_cap(var) { }	\
	static __always_inline void __assume_cap(const struct name *var)				\
		__attribute__((overloadable)) __assumes_cap(var) { }					\
	static __always_inline void __assume_shared_cap(const struct name *var)				\
		__attribute__((overloadable)) __assumes_shared_cap(var) { }				\
	struct name

/**
 * disable_capability_analysis() - disables capability analysis
 *
 * Disables capability analysis. Must be paired with a later
 * enable_capability_analysis().
 */
# define disable_capability_analysis()				\
	__diag_push();						\
	__diag_ignore_all("-Wunknown-warning-option", "")	\
	__diag_ignore_all("-Wthread-safety", "")		\
	__diag_ignore_all("-Wthread-safety-pointer", "")

/**
 * enable_capability_analysis() - re-enables capability analysis
 *
 * Re-enables capability analysis. Must be paired with a prior
 * disable_capability_analysis().
 */
# define enable_capability_analysis() __diag_pop()

/**
 * __no_capability_analysis - function attribute, disables capability analysis
 *
 * Function attribute denoting that capability analysis is disabled for the
 * whole function. Prefer use of `capability_unsafe()` where possible.
 */
# define __no_capability_analysis	__attribute__((no_thread_safety_analysis))

#else /* !WARN_CAPABILITY_ANALYSIS */

# define __cap_type(name)
# define __reentrant_cap
# define __acquires_cap(...)
# define __acquires_shared_cap(...)
# define __try_acquires_cap(ret, var)
# define __try_acquires_shared_cap(ret, var)
# define __releases_cap(...)
# define __releases_shared_cap(...)
# define __assumes_cap(...)
# define __assumes_shared_cap(...)
# define __returns_cap(var)
# define __guarded_by(...)
# define __pt_guarded_by(...)
# define __excludes_cap(...)
# define __requires_cap(...)
# define __requires_shared_cap(...)
# define __acquire_cap(var)			do { } while (0)
# define __acquire_shared_cap(var)		do { } while (0)
# define __try_acquire_cap(var, ret)		(ret)
# define __try_acquire_shared_cap(var, ret)	(ret)
# define __release_cap(var)			do { } while (0)
# define __release_shared_cap(var)		do { } while (0)
# define __assume_cap(var)			do { (void)(var); } while (0)
# define __assume_shared_cap(var)		do { (void)(var); } while (0)
# define struct_with_capability(name, ...)	struct __VA_ARGS__ name
# define disable_capability_analysis()
# define enable_capability_analysis()
# define __no_capability_analysis

#endif /* WARN_CAPABILITY_ANALYSIS */

/**
 * capability_unsafe() - disable capability checking for contained code
 *
 * Disables capability checking for contained statements or expression.
 *
 * .. code-block:: c
 *
 *	struct some_data {
 *		spinlock_t lock;
 *		int counter __guarded_by(&lock);
 *	};
 *
 *	int foo(struct some_data *d)
 *	{
 *		// ...
 *		// other code that is still checked ...
 *		// ...
 *		return capability_unsafe(d->counter);
 *	}
 */
#define capability_unsafe(...)		\
({					\
	disable_capability_analysis();	\
	__VA_ARGS__;			\
	enable_capability_analysis()	\
})

/**
 * __capability_unsafe() - function attribute, disable capability checking
 * @comment: comment explaining why opt-out is safe
 *
 * Function attribute denoting that capability analysis is disabled for the
 * whole function. Forces adding an inline comment as argument.
 */
#define __capability_unsafe(comment) __no_capability_analysis

/**
 * capability_unsafe_alias() - helper to insert a capability "alias barrier"
 * @p: pointer aliasing a capability or object containing capabilities
 *
 * No-op function that acts as a "capability alias barrier", where the analysis
 * rightfully detects that we're switching aliases, but the switch is considered
 * safe but beyond the analysis reasoning abilities.
 *
 * This should be inserted before the first use of such an alias.
 *
 * Implementation Note: The compiler ignores aliases that may be reassigned but
 * their value cannot be determined (e.g. when passing a non-const pointer to an
 * alias as a function argument).
 */
#define capability_unsafe_alias(p) _capability_unsafe_alias((void **)&(p))
static inline void _capability_unsafe_alias(void **p) { }

/**
 * token_capability() - declare an abstract global capability instance
 * @name: token capability name
 *
 * Helper that declares an abstract global capability instance @name that can be
 * used as a token capability, but not backed by a real data structure (linker
 * error if accidentally referenced). The type name is `__capability_@name`.
 */
#define token_capability(name, ...)					\
	struct_with_capability(__capability_##name, ##__VA_ARGS__) {};	\
	extern const struct __capability_##name *name

/**
 * token_capability_instance() - declare another instance of a global capability
 * @cap: token capability previously declared with token_capability()
 * @name: name of additional global capability instance
 *
 * Helper that declares an additional instance @name of the same token
 * capability class @name. This is helpful where multiple related token
 * capabilities are declared, as it also allows using the same underlying type
 * (`__capability_@cap`) as function arguments.
 */
#define token_capability_instance(cap, name)		\
	extern const struct __capability_##cap *name

/*
 * Common keywords for static capability analysis. Both Clang's capability
 * analysis and Sparse's context tracking are currently supported.
 */
#ifdef __CHECKER__

/* Sparse context/lock checking support. */
# define __must_hold(x)		__attribute__((context(x,1,1)))
# define __must_not_hold(x)
# define __acquires(x)		__attribute__((context(x,0,1)))
# define __cond_acquires(x)	__attribute__((context(x,0,-1)))
# define __releases(x)		__attribute__((context(x,1,0)))
# define __acquire(x)		__context__(x,1)
# define __release(x)		__context__(x,-1)
# define __cond_lock(x, c)	((c) ? ({ __acquire(x); 1; }) : 0)
/* For Sparse, there's no distinction between exclusive and shared locks. */
# define __must_hold_shared	__must_hold
# define __acquires_shared	__acquires
# define __cond_acquires_shared __cond_acquires
# define __releases_shared	__releases
# define __acquire_shared	__acquire
# define __release_shared	__release
# define __cond_lock_shared	__cond_acquire

#else /* !__CHECKER__ */

/**
 * __must_hold() - function attribute, caller must hold exclusive capability
 * @x: capability instance pointer
 *
 * Function attribute declaring that the caller must hold the given capability
 * instance @x exclusively.
 */
# define __must_hold(x)		__requires_cap(x)

/**
 * __must_not_hold() - function attribute, caller must not hold capability
 * @x: capability instance pointer
 *
 * Function attribute declaring that the caller must not hold the given
 * capability instance @x.
 */
# define __must_not_hold(x)	__excludes_cap(x)

/**
 * __acquires() - function attribute, function acquires capability exclusively
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function acquires the given
 * capability instance @x exclusively, but does not release it.
 */
# define __acquires(x)		__acquires_cap(x)

/**
 * __cond_acquires() - function attribute, function conditionally
 *                     acquires a capability exclusively
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function conditionally acquires the
 * given capability instance @x exclusively, but does not release it.
 */
# define __cond_acquires(x)	__try_acquires_cap(1, x)

/**
 * __releases() - function attribute, function releases a capability exclusively
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function releases the given capability
 * instance @x exclusively. The capability must be held on entry.
 */
# define __releases(x)		__releases_cap(x)

/**
 * __acquire() - function to acquire capability exclusively
 * @x: capability instance pointer
 *
 * No-op function that acquires the given capability instance @x exclusively.
 */
# define __acquire(x)		__acquire_cap(x)

/**
 * __release() - function to release capability exclusively
 * @x: capability instance pointer
 *
 * No-op function that releases the given capability instance @x.
 */
# define __release(x)		__release_cap(x)

/**
 * __cond_lock() - function that conditionally acquires a capability
 *                 exclusively
 * @x: capability instance pinter
 * @c: boolean expression
 *
 * Return: result of @c
 *
 * No-op function that conditionally acquires capability instance @x
 * exclusively, if the boolean expression @c is true. The result of @c is the
 * return value, to be able to create a capability-enabled interface; for
 * example:
 *
 * .. code-block:: c
 *
 *	#define spin_trylock(l) __cond_lock(&lock, _spin_trylock(&lock))
 */
# define __cond_lock(x, c)	__try_acquire_cap(x, c)

/**
 * __must_hold_shared() - function attribute, caller must hold shared capability
 * @x: capability instance pointer
 *
 * Function attribute declaring that the caller must hold the given capability
 * instance @x with shared access.
 */
# define __must_hold_shared(x)	__requires_shared_cap(x)

/**
 * __acquires_shared() - function attribute, function acquires capability shared
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function acquires the given
 * capability instance @x with shared access, but does not release it.
 */
# define __acquires_shared(x)	__acquires_shared_cap(x)

/**
 * __cond_acquires_shared() - function attribute, function conditionally
 *                            acquires a capability shared
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function conditionally acquires the
 * given capability instance @x with shared access, but does not release it.
 */
# define __cond_acquires_shared(x) __try_acquires_shared_cap(1, x)

/**
 * __releases_shared() - function attribute, function releases a
 *                       capability shared
 * @x: capability instance pointer
 *
 * Function attribute declaring that the function releases the given capability
 * instance @x with shared access. The capability must be held on entry.
 */
# define __releases_shared(x)	__releases_shared_cap(x)

/**
 * __acquire_shared() - function to acquire capability shared
 * @x: capability instance pointer
 *
 * No-op function that acquires the given capability instance @x with shared
 * access.
 */
# define __acquire_shared(x)	__acquire_shared_cap(x)

/**
 * __release_shared() - function to release capability shared
 * @x: capability instance pointer
 *
 * No-op function that releases the given capability instance @x with shared
 * access.
 */
# define __release_shared(x)	__release_shared_cap(x)

/**
 * __cond_lock_shared() - function that conditionally acquires a capability
 *                        shared
 * @x: capability instance pinter
 * @c: boolean expression
 *
 * Return: result of @c
 *
 * No-op function that conditionally acquires capability instance @x with shared
 * access, if the boolean expression @c is true. The result of @c is the return
 * value, to be able to create a capability-enabled interface.
 */
# define __cond_lock_shared(x, c) __try_acquire_shared_cap(x, c)

#endif /* __CHECKER__ */

/**
 * __acquire_ret() - helper to acquire capability of return value
 * @call: call expression
 * @ret_expr: acquire expression that uses __ret
 */
#define __acquire_ret(call, ret_expr)		\
	({					\
		__auto_type __ret = call;	\
		__acquire(ret_expr);		\
		__ret;				\
	})

/**
 * __acquire_shared_ret() - helper to acquire capability shared of return value
 * @call: call expression
 * @ret_expr: acquire shared expression that uses __ret
 */
#define __acquire_shared_ret(call, ret_expr)	\
	({					\
		__auto_type __ret = call;	\
		__acquire_shared(ret_expr);	\
		__ret;				\
	})

/*
 * Attributes to mark functions returning acquired capabilities. This is purely
 * cosmetic to help readability, and should be used with the above macros as
 * follows:
 *
 *   struct foo { spinlock_t lock; ... };
 *   ...
 *   #define myfunc(...) __acquire_ret(_myfunc(__VA_ARGS__), &__ret->lock)
 *   struct foo *_myfunc(int bar) __acquires_ret;
 *   ...
 */
#define __acquires_ret		__no_capability_analysis
#define __acquires_shared_ret	__no_capability_analysis

#endif /* _LINUX_COMPILER_CAPABILITY_ANALYSIS_H */
