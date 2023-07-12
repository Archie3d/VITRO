# VITRO
Declarative UI module for JUCE.

## Compilation

This module must be used via CMake (VITRO is not compatible with Projucer).

Add [JUCE](https://github.com/juce-framework/JUCE) and [VITRO](https://github.com/Archie3d/VITRO) submodules to your project, and then link `juce::vitro` with your target:

```CMake
add_subdirectory(JUCE)
add_subdirectory(VITRO)

target_link_libraries(${TARGET}
    PRIVATE
        juce::juce_core
        juce::juce_data_structures
        juce::juce_gui_basics

        juce::vitro

        juce::juce_recommended_config_flags
)
```

[See the test projects](https://github.com/Archie3d/VITRO-test) for examples.
