from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain


class CS2KitConan(ConanFile):
    name = "cs2-kit"
    version = "1.0.0"
    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    requires = (
        "cpr/1.11.2",
        "nlohmann_json/3.11.3",
    )

    default_options = {
        "*:shared": False,
        "openssl/*:no_apps": True,
        "openssl/*:no_fips": True,
    }

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.user_presets_path = False
        toolchain.variables["CMAKE_POSITION_INDEPENDENT_CODE"] = True
        toolchain.generate()
