# CS2Kit - C++23 CS2 Plugin Development Library

Reusable C++23 library for building Counter-Strike 2 server plugins with
Metamod:Source 2.0.

## Tech Stack

- Language: C++23
- Framework: Metamod:Source 2.0 + hl2sdk-cs2
- Build: CMake 4.3.4+ presets + Conan 2.29.1+
- Public target: `CS2Kit::CS2Kit`
- Docs: Doxygen + doxygen-awesome-css

## Project Structure

```text
include/CS2Kit/        Public API headers
src/                   Implementation
gamedata/              Engine signatures and offsets
docs/                  Doxygen pages and guides
vendor/                SDK submodules
CMakeLists.txt         Standalone CMake build
CMakePresets.json      Windows/Linux presets
conanfile.py           Third-party deps
```

## Build Commands

```bash
uv run poe build
uv run poe build windows-msvc-release
uv run poe build-linux
```

Consuming projects should use:

```cmake
add_subdirectory(vendor/cs2-kit)
target_link_libraries(my-plugin PRIVATE CS2Kit::CS2Kit)
```

## Code Conventions

- C++23.
- `.hpp` headers, not `.h`.
- C#-style naming.
- Service container + `Engine()` accessor; no process-lifetime singletons.
- Use `std::format`, designated initializers, and `std::function` callbacks.
- Builder pattern for menus and commands.

## Design Notes

`CS2Kit::Core::MetamodPluginBase` is the recommended plugin entry point. The
base owns ISmmPlugin getters, Load/Unload flow, standard SourceHook hooks, and
the `PlayerManager` add/remove lifecycle.
