# General purpose functions

#########################
# ROOT utilities
#########################

# Generation of the dictionaries
# @DNAME  Dictionary name
# @LDNAME LinkDef file name, ex: LinkDef.h
# @DHDRS  Dictionary headers
# @DHDRS_DEPS  Dictionary header files used as dependencies to the rootmap target
# @DINCDIR Include folders that need to be passed to cint/cling
macro(_generate_dictionary DNAME LDNAME DHDRS DHDRS_DEPS DINCDIRS)

    # Creating the INCLUDE path for cint/cling
    foreach(_dir ${DINCDIRS})
        set(INCLUDE_PATH -I${_dir} ${INCLUDE_PATH})
    endforeach()

    # Get the list of definitions from the directory to be sent to CINT
    get_directory_property(tmpdirdefs COMPILE_DEFINITIONS)
    foreach(dirdef ${tmpdirdefs})
        string(REPLACE "\"" "\\\"" dirdef_esc ${dirdef})
        set(GLOBALDEFINITIONS -D${dirdef_esc} ${GLOBALDEFINITIONS})
    endforeach()

    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/lib${DNAME}.rootmap ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}.cpp ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}_rdict.pcm
                    COMMAND
                        ${ROOT_CINT}
                    ARGS
                        -f ${CMAKE_CURRENT_BINARY_DIR}/G__${DNAME}.cpp
                        -rmf ${CMAKE_CURRENT_BINARY_DIR}/lib${DNAME}.rootmap -rml lib${DNAME}
                        ${GLOBALDEFINITIONS}  ${INCLUDE_PATH} ${DHDRS} ${LDNAME}
                    DEPENDS
                        ${DHDRS_DEPS} ${LDNAME} ${ROOT_CINT}
                    WORKING_DIRECTORY
                        ${CMAKE_CURRENT_BINARY_DIR}
                    )

endmacro(_generate_dictionary)


# Same as generate_dictionary, but flattens the list of headers and sets additional include paths
# with include_directories
macro(generate_dictionary DNAME LDNAME DHDRS DINCDIRS)

    set(_dhdrs "")
    set(_daddincdirs "")
    foreach(_itm ${DHDRS})
      string(FIND "${_itm}" "/" _idx)
      if(_idx GREATER -1)
        # Has a subdirectory specified
        get_filename_component(_itmdir "${_itm}" DIRECTORY)
        get_filename_component(_itmbase "${_itm}" NAME)
        list(APPEND _dhdrs "${_itmbase}")
        list(APPEND _daddincdirs "${CMAKE_CURRENT_SOURCE_DIR}/${_itmdir}")
      else()
        # No subdirectory specified
        list(APPEND _dhdrs "${_itm}")
      endif()
    endforeach()
    list(REMOVE_DUPLICATES _daddincdirs)
    if(NOT "${_daddincdirs}" STREQUAL "")
      foreach(_dir "${_daddincdirs}")
        include_directories("${_dir}")
      endforeach()
    endif()
    _generate_dictionary("${DNAME}" "${LDNAME}" "${_dhdrs}" "${DHDRS}" "${DINCDIRS};${_daddincdirs};${CMAKE_CURRENT_SOURCE_DIR}" "${ARGV4}")

endmacro(generate_dictionary)

#########################
# Static utilities
#########################

# Generate the static dependecies from dynamic list
# @ shared_list - list of shared libraries
# @ static_list - the name of the variable that will contain the list of static libraries
macro(generate_static_dependencies shared_list static_list)
    set(static_list_tmp "")
    foreach(shared_lib ${shared_list})
        set(static_list_tmp ${static_list_tmp} "${shared_lib}-static")
    endforeach()

    # create the variable with the name received by the macro
    set(${static_list} ${static_list_tmp})
    # set the scope to parent in order to be visible in the parent
    set(${static_list} PARENT_SCOPE)
endmacro(generate_static_dependencies)

# Prepend prefix to every element in the list. Note: this function modifies the input variable: this
# does not work for macros in CMake, only for functions. Also note that it does NOT automatically
# add a / between prefix and list item as it does not assume that we are dealing with directories
function(prepend_prefix INLIST PREFIX)
    foreach(_ITEM ${${INLIST}})
        list(APPEND _OUTLIST ${PREFIX}${_ITEM})
    endforeach()
    set(${INLIST} ${_OUTLIST} PARENT_SCOPE)
endfunction()
