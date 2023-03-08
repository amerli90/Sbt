set(Yaml_FOUND FALSE)

if(NOT YAML)
    set(YAML "/usr")
endif()

set(Yaml_LIBDIR ${YAML}/lib)
set(Yaml_INCDIR ${YAML}/include)

link_directories(${Yaml_LIBDIR})
include_directories(${Yaml_INCDIR})

if(EXISTS "${Yaml_INCDIR}/yaml-cpp/yaml.h")
    message(STATUS "Found YAML in ${YAML}")
    set(Yaml_FOUND TRUE)
else()
    message(FATAL_ERROR "yaml-cpp is a required dependency. Call cmake with -DYAML=path/to/yaml")
endif()
