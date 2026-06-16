cmake_minimum_required(VERSION 3.22)

project(MosScaleCircle VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# JUCE
#
# When this repo is consumed by a parent project, JUCE has usually already been
# added (juce::juce_core exists) and we just register our module. For standalone
# builds, point JUCE_PATH (or the juce_DIR env var) at a JUCE checkout.
# ------------------------------------------------------------------------------
if(NOT TARGET juce::juce_core)
    set(JUCE_PATH "$ENV{juce_DIR}" CACHE PATH "Path to a JUCE checkout")
    if(NOT JUCE_PATH OR NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
        foreach(candidate
                "${CMAKE_CURRENT_SOURCE_DIR}/../juce"
                "${CMAKE_CURRENT_SOURCE_DIR}/../JUCE"
                "$ENV{HOME}/Programming/juce")
            if(EXISTS "${candidate}/CMakeLists.txt")
                set(JUCE_PATH "${candidate}")
                break()
            endif()
        endforeach()
    endif()
    if(NOT EXISTS "${JUCE_PATH}/CMakeLists.txt")
        message(FATAL_ERROR "Could not locate JUCE. Set -DJUCE_PATH=/path/to/JUCE.")
    endif()
    add_subdirectory("${JUCE_PATH}" "${CMAKE_CURRENT_BINARY_DIR}/JUCE")
endif()

# ------------------------------------------------------------------------------
# The reusable module. Consumers link the `mos_scale_circle` target; its JUCE
# dependencies (declared in mos_scale_circle.h) propagate transitively.
# ------------------------------------------------------------------------------
juce_add_module("${CMAKE_CURRENT_SOURCE_DIR}/mos_scale_circle" ALIAS_NAMESPACE mos)

# ------------------------------------------------------------------------------
# Optional demo / verification app.
# ------------------------------------------------------------------------------
option(MOSSC_BUILD_DEMO "Build the MOS Scale Circle demo app" OFF)

if(MOSSC_BUILD_DEMO)
    juce_add_gui_app(MosScaleCircleDemo
        PRODUCT_NAME "MOS Scale Circle Demo"
        NEEDS_WEB_BROWSER FALSE
        NEEDS_CURL FALSE)

    target_sources(MosScaleCircleDemo PRIVATE
        examples/Main.cpp
        examples/DemoComponent.cpp)

    target_compile_definitions(MosScaleCircleDemo PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MosScaleCircleDemo,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MosScaleCircleDemo,JUCE_VERSION>")

    target_link_libraries(MosScaleCircleDemo PRIVATE
        mos_scale_circle
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
endif()
