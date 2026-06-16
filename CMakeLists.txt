cmake_minimum_required(VERSION 3.22)

project(MosScaleCircleComponent VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# If JUCE hasn't been added by the parent project, add it here
if(NOT TARGET juce::juce_core)
    set(JUCE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../juce-framework/JUCE" CACHE PATH "Path to JUCE framework")
    add_subdirectory(${JUCE_PATH} ${CMAKE_CURRENT_BINARY_DIR}/JUCE)
endif()

add_library(MosScaleCircleComponent STATIC
    Source/ScaleStructure.cpp
    Source/ScaleStructureComponent.cpp
    Source/GroupingCircle.cpp
    Source/GroupHandle.cpp
    Source/NumberSelector.cpp
    Source/NoteNames.cpp
)

target_include_directories(MosScaleCircleComponent
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
        $<INSTALL_INTERFACE:include>
)

target_compile_definitions(MosScaleCircleComponent
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)

target_link_libraries(MosScaleCircleComponent
    PUBLIC
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
)
