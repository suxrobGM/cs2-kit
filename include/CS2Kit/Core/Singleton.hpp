#pragma once

namespace CS2Kit::Core
{

/**
 * @brief Transitional CRTP base bridging the old `Instance()` access to the new
 * composition-root ownership model.
 *
 * The instance is no longer a process-lifetime static local — it is a member owned
 * by the plugin's service container (CS2Kit::Core::Services for kit services, the
 * plugin's own Managers struct for plugin services), constructed on Load and
 * destroyed on Unload. That fixes state leaking across `meta unload`/`meta reload`.
 *
 * Each derived object registers itself as the active instance on construction, so
 * legacy `T::Instance()` call sites keep resolving to the single owned object while
 * they are migrated to `Kit().Member` / `Sys().Member`. Once all call sites are
 * migrated, the base (and this header) are removed.
 *
 * @tparam T The derived class type (CRTP parameter).
 */
template <typename T>
class Singleton
{
public:
    /** The instance currently owned by the composition root. */
    static T& Instance() { return *_active; }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    Singleton() { _active = static_cast<T*>(this); }
    ~Singleton()
    {
        if (_active == static_cast<T*>(this))
            _active = nullptr;
    }

private:
    inline static T* _active = nullptr;
};

}  // namespace CS2Kit::Core
