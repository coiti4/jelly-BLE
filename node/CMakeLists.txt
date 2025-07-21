cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(NONE)

# NORDIC SDK APP START
# Agrega todos los .c dentro de src/
file(GLOB app_sources src/*.c ../shared/*.c)

# Agrega los archivos al target
target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE include ../shared)

# NORDIC SDK APP END
zephyr_library_include_directories(.)
