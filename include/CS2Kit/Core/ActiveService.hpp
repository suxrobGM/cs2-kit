#pragma once

#include <cassert>

namespace CS2Kit::Core
{

/**
 * Holds the one active instance of @p T for the current Load/Unload cycle, behind a process-wide
 * accessor. Lets a service struct (the cs2-kit @ref Services, a plugin's Managers) be reached from
 * anywhere without a per-class singleton: construct it on Load, `Set` the pointer, `Set(nullptr)`
 * on Unload. `inline static` gives exactly one slot per @p T across all translation units (source
 * inclusion + static lib).
 */
template <class T>
class ActiveService
{
public:
    static void Set(T* instance) { _active = instance; }

    /** The active instance. Asserts if called outside a Load/Unload window. */
    static T& Get()
    {
        assert(_active && "ActiveService<T>::Get() called with no active instance (outside Load/Unload)");
        return *_active;
    }

    /** The active instance, or nullptr — for teardown paths that may run after Set(nullptr). */
    static T* GetOrNull() { return _active; }

private:
    inline static T* _active = nullptr;
};

}  // namespace CS2Kit::Core
