juce_add_modules(
    ALIAS_NAMESPACE juce
    vitro
)

target_include_directories(vitro INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/vitro)
