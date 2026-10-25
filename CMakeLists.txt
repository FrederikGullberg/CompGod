cmake_minimum_required(VERSION 3.22)
if(CMAKE_HOST_APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
endif()
project(CompGod VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(COMPGOD_DSP_ONLY "Build only the dependency-free DSP tests" OFF)
enable_testing()
add_executable(compgod_dsp_tests Tests/DspTests.cpp)
target_include_directories(compgod_dsp_tests PRIVATE Source)
add_test(NAME dsp COMMAND compgod_dsp_tests)
if(COMPGOD_DSP_ONLY)
    return()
endif()
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
set(COMPGOD_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND COMPGOD_FORMATS AU)
endif()
juce_add_plugin(CompGod
    COMPANY_NAME "CompGod Audio"
    BUNDLE_ID "audio.compgod.plugin"
    PLUGIN_MANUFACTURER_CODE CgAu
    PLUGIN_CODE Cg01
    PRODUCT_NAME "CompGod"
    FORMATS ${COMPGOD_FORMATS}
    VST3_CATEGORIES Fx Dynamics
    AU_MAIN_TYPE kAudioUnitType_Effect
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "CompGod needs audio input access to compress live audio in the standalone app."
    IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)
juce_generate_juce_header(CompGod)
target_sources(CompGod PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(CompGod PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0 JUCE_PLUGINHOST_VST3=1)
target_link_libraries(CompGod PRIVATE juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
add_executable(compgod_plugin_tests Tests/PluginTests.cpp)
target_link_libraries(compgod_plugin_tests PRIVATE CompGod)
target_include_directories(compgod_plugin_tests PRIVATE Source "$<TARGET_PROPERTY:CompGod,INCLUDE_DIRECTORIES>")
target_compile_definitions(compgod_plugin_tests PRIVATE "$<TARGET_PROPERTY:CompGod,COMPILE_DEFINITIONS>")
target_compile_definitions(compgod_plugin_tests PRIVATE
    COMPGOD_VST3_PATH="$<GENEX_EVAL:$<TARGET_PROPERTY:CompGod_VST3,JUCE_PLUGIN_ARTEFACT_FILE>>")
add_dependencies(compgod_plugin_tests CompGod_VST3)
add_test(NAME plugin COMMAND compgod_plugin_tests)
set_tests_properties(plugin PROPERTIES TIMEOUT 120)
