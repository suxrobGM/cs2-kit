#pragma once

#include <CS2Kit/Core/ActiveService.hpp>
#include <CS2Kit/Core/MetamodPluginBase.hpp>
#include <memory>

namespace CS2Kit::Core
{

/**
 * @brief MetamodPluginBase that also owns the plugin's manager container.
 *
 * Constructs @p TManagers after the kit Services are live (so member initializers may call
 * Engine()), publishes it through ActiveService so `App()` works anywhere, and destroys it on
 * unload after the Defer() cleanups ran - the same lifetime the kit's own Services follow.
 *
 * @code
 * class MyPlugin : public CS2Kit::PluginBase<MySystem::Managers> { ... };
 * // anywhere in the plugin:
 * inline MySystem::Managers& App() { return MyPlugin::App(); }
 * @endcode
 */
template <class TManagers>
class PluginBase : public MetamodPluginBase
{
public:
    /** The live manager container. Valid only between OnCreateInstances and unload. */
    static TManagers& App() { return ActiveService<TManagers>::Get(); }

    /** The live manager container, or nullptr - for teardown paths that may outlive it. */
    static TManagers* AppOrNull() { return ActiveService<TManagers>::GetOrNull(); }

protected:
    void OnCreateInstances() override
    {
        _managers = std::make_unique<TManagers>();
        ActiveService<TManagers>::Set(_managers.get());
    }

    void OnDestroyInstances() override
    {
        ActiveService<TManagers>::Set(nullptr);
        _managers.reset();
    }

private:
    std::unique_ptr<TManagers> _managers;
};

}  // namespace CS2Kit::Core
