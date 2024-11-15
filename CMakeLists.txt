macro(make_source_group SRCS)
    foreach(file ${SRCS})
        get_filename_component(file_dir ${file} DIRECTORY)
        file(RELATIVE_PATH file_dir ${CMAKE_CURRENT_SOURCE_DIR} ${file_dir})
        get_filename_component(file_path ${file} ABSOLUTE)
        file(RELATIVE_PATH file_path ${CMAKE_CURRENT_SOURCE_DIR} ${file_path})
        string(REPLACE "/" "\\" group ${file_dir})
        source_group("${group}" FILES ${file_path})
    endforeach()
endmacro()

#===========================================================

set(VITRO_USE_INTERNAL_QUICK_JS ON CACHE BOOL "")

if(VITRO_USE_INTERNAL_QUICK_JS)

    add_library(quickjs STATIC)

    file(GLOB_RECURSE quickjs_src
        ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/quickjs/*.c
    )

    make_source_group("${quickjs_src}")

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
            VITRO_USE_INTERNAL_QUICK_JS=1
            _CRT_SECURE_NO_WARNINGS
            JS_STRICT_NAN_BOXING=1
            # CONFIG_BIGNUM=1  # Can be enabled after JUCE 8.0.3
            CONFIG_JSX=1
    )

endif()

#===========================================================

juce_add_modules(
    ALIAS_NAMESPACE juce
    vitro
)

target_include_directories(vitro
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/vitro
)

if(VITRO_USE_INTERNAL_QUICK_JS)
    target_link_libraries(vitro
        INTERFACE
            quickjs
    )

    target_compile_definitions(vitro
        INTERFACE
            VITRO_USE_INTERNAL_QUICK_JS=1
    )
endif()

file(GLOB_RECURSE vitro_src
    ${CMAKE_CURRENT_SOURCE_DIR}/vitro/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vitro/*.cpp
)

add_custom_target(VITRO SOURCES ${vitro_src})

make_source_group("${vitro_src}")
