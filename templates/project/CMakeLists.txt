cmake_minimum_required(VERSION 4.3)

project($project VERSION 1.0.0 LANGUAGES CXX)

include(CTest)

# Postgres support: uncomment and add "libpqxx/7.10.0" to conanfile.py requires.
# set(CS2KIT_ENABLE_POSTGRES ON)

add_subdirectory(vendor/cs2-kit)
