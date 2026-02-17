#pragma once

namespace CS2Kit::Core
{

/**
 * CRTP singleton base using the pass-key idiom.
 *
 * Derived classes inherit via `class Foo : public Singleton<Foo>` and declare
 * a public constructor taking Token: `explicit Foo(Token) {}`.
 *
 * @tparam T The derived class type (CRTP parameter).
 */
template <typename T>
class Singleton
{
public:
    static T& Instance()
    {
        static T instance{Token{}};
        return instance;
    }

    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

protected:
    struct Token
    {};
    Singleton() = default;
    ~Singleton() = default;
};

}  // namespace CS2Kit::Core
