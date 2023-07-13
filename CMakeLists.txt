add_library(quickjs STATIC)

file(GLOB_RECURSE quickjs_src
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.c
)

target_include_directories(quickjs
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/quickjs
)

target_sources(quickjs
    PRIVATE
        ${quickjs_src}
)

target_compile_definitions(quickjs
    PUBLIC
        _CRT_SECURE_NO_WARNINGS
        JS_STRICT_NAN_BOXING=1
        CONFIG_BIGNUM=1
        CONFIG_JSX=1
)

juce_add_modules(
    ALIAS_NAMESPACE juce
    vitro
)

target_include_directories(vitro
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/vitro
)

target_link_libraries(vitro
    INTERFACE
        quickjs
)

file(GLOB_RECURSE vitro_src
    ${CMAKE_CURRENT_SOURCE_DIR}/vitro/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vitro/*.cpp
)

add_custom_target(VITRO SOURCES ${vitro_src})
