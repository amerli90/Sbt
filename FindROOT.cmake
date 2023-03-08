# Checking for a proper ROOT installation
# If proper root installation it is setting the following variables
# - ROOT_VERSION - ROOT version as reported by root-config
# - ROOT_VERSION_MAJOR
# - ROOT_VERSIOM_MINOR
# - ROOT_VERSION_PATCH
# - ROOT_CONFIG - path to root-config script
# - ROOT_CINT - path to rootcint executable
# - ROOT_LIBMAP - path to rlibmap executable
# - ROOT_FEATURES - list of build features for ROOT
# - ROOT_LIBDIR - full path to ROOT library folder
# - ROOT_ETCDIR - full path to the ROOT's configuration directory
# - ROOT_LIBRARIES - libraries needed for the package to be used
# - ROOT_GLIBRARIES - regular + GUI ROOT libraries + path to be used during linking
# - ROOT_INCLUDE_DIR - full path to ROOT include folder
# - ROOT_CXX_FLAGS - flags used by the C++ compiler for building ROOT
# - ROOT_HAS_LIBCXX - TRUE if ROOT was built against libc++ (as opposed to libstdc++)
# - ROOT_HAS_CXX11 - TRUE if ROOT was built with C++11 support

set(ROOT_FOUND FALSE)

# Check for root-config scripts
find_program(ROOT_CONFIG NAMES root-config)

if(ROOT_CONFIG)
    message(STATUS "Checking for a proper ROOT installation.")
    mark_as_advanced(ROOT_CONFIG)

    # Setting defaults
    execute_process(COMMAND ${ROOT_CONFIG} --libdir OUTPUT_VARIABLE ROOT_LIBDIR ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT lib dir : ${error}")
    endif(error)
    string(STRIP "${ROOT_LIBDIR}" ROOT_LIBDIR)

    execute_process(COMMAND ${ROOT_CONFIG} --incdir OUTPUT_VARIABLE ROOT_INCLUDE_DIR ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT inc dir : ${error}")
    endif(error)
    string(STRIP "${ROOT_INCLUDE_DIR}" ROOT_INCLUDE_DIR)

    # Check for rootcint
    find_program(ROOT_CINT NAMES rootcint)
    if(ROOT_CINT)
        message(STATUS "Found ${ROOT_CINT}")
    else()
        message(FATAL_ERROR "Could not find rootcint executable.")
    endif(ROOT_CINT)
    mark_as_advanced(ROOT_CINT)

    # Checking ROOT version
    execute_process(COMMAND ${ROOT_CONFIG} --version OUTPUT_VARIABLE ROOT_VERSION ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT version : ${error}")
    endif(error)
    string(STRIP "${ROOT_VERSION}" ROOT_VERSION)

    # Extract major, minor, and patch versions from
    string(REGEX REPLACE "^([0-9]+)\\.[0-9][0-9]+\\/[0-9][0-9]+.*" "\\1" ROOT_VERSION_MAJOR "${ROOT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.([0-9][0-9])+\\/[0-9][0-9]+.*" "\\1" ROOT_VERSION_MINOR "${ROOT_VERSION}")
    string(REGEX REPLACE "^[0-9]+\\.[0-9][0-9]+\\/([0-9][0-9]+).*" "\\1" ROOT_VERSION_PATCH "${ROOT_VERSION}")
    string(REGEX REPLACE "/" "." ROOT_VERSION_NORM "${ROOT_VERSION}")
    message(STATUS "Found ROOT version ${ROOT_VERSION_NORM}")

    # Print ROOT features
    execute_process(COMMAND ${ROOT_CONFIG} --features OUTPUT_VARIABLE ROOT_FEATURES ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT features : ${error}")
    else()
        message(STATUS "ROOT was built with the following features: ${ROOT_FEATURES}")
    endif(error)
    string(STRIP "${ROOT_FEATURES}" ROOT_FEATURES)

    # Checking for ROOT etcdir
    execute_process(COMMAND ${ROOT_CONFIG} --etcdir OUTPUT_VARIABLE ROOT_ETCDIR ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT etcdir: ${error}")
    endif(error)
    string(STRIP "${ROOT_ETCDIR}" ROOT_ETCDIR)

    # Checking for ROOT libs
    execute_process(COMMAND ${ROOT_CONFIG} --noldflags --libs OUTPUT_VARIABLE ROOT_LIBS ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT libdir: ${error}")
    endif(error)
    string(STRIP "${ROOT_LIBS}" ROOT_LIBS)

    foreach(lib ${ROOT_LIBS})
        string(REPLACE "-rdynamic" "" new_lib ${lib})
        string(REPLACE "-l" "" lib ${new_lib})
        set(ROOT_LIBRARIES ${ROOT_LIBRARIES} ${lib})
    endforeach()
    string(STRIP "${ROOT_LIBRARIES}" ROOT_LIBRARIES)
    separate_arguments(ROOT_LIBRARIES)

    # Setting ROOT compiler flags (except the -I parts) to a variable
    execute_process(COMMAND ${ROOT_CONFIG} --auxcflags OUTPUT_VARIABLE _ROOT_CXX_FLAGS ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT compiler flags: ${error}")
    endif(error)
    string(STRIP "${_ROOT_CXX_FLAGS}" ROOT_CXX_FLAGS)

    # Check if ROOT was built with libc++ (as opposed to stdlibc++)
    if(ROOT_CXX_FLAGS MATCHES "-stdlib=libc\\+\\+")
        set(ROOT_HAS_LIBCXX TRUE)
    else()
        set(ROOT_HAS_LIBCXX FALSE)
    endif()

    # Check if ROOT was built with C++11 support
    if(ROOT_CXX_FLAGS MATCHES "-std=c\\+\\+11")
        message(STATUS "ROOT was built with C++11 support")
        set(ROOT_HAS_CXX11 TRUE)
    else()
        set(ROOT_HAS_CXX11 FALSE)
    endif()

    # Check if ROOT was built with C++14 support
    if(ROOT_CXX_FLAGS MATCHES "-std=c\\+\\+14")
        message(STATUS "ROOT was built with C++14 support")
        set(ROOT_HAS_CXX14 TRUE)
    else()
        set(ROOT_HAS_CXX14 FALSE)
    endif()

    # Check if ROOT was built with C++17 support
    if(ROOT_CXX_FLAGS MATCHES "-std=c\\+\\+17")
        message(STATUS "ROOT was built with C++17 support")
        set(ROOT_HAS_CXX17 TRUE)
    else()
        set(ROOT_HAS_CXX17 FALSE)
    endif()

    # Checking for glibs
    execute_process(COMMAND ${ROOT_CONFIG} --noldflags --glibs OUTPUT_VARIABLE ROOT_GLIBS ERROR_VARIABLE error OUTPUT_STRIP_TRAILING_WHITESPACE )
    if(error)
        message(FATAL_ERROR "Error retrieving ROOT glibs: ${error}")
    endif(error)

    # Checking for glibs
    string(STRIP "${ROOT_GLIBS}" ROOT_GLIBS)

    foreach(lib ${ROOT_GLIBS})
        string(REPLACE "-rdynamic" "" new_lib "${lib}")
        string(REPLACE "-l" "" lib "${new_lib}")
        set(ROOT_GLIBRARIES ${ROOT_GLIBRARIES} ${lib})
    endforeach()
    string(STRIP "${ROOT_GLIBRARIES}" ROOT_GLIBRARIES)
    separate_arguments(ROOT_GLIBRARIES)

    # adding the libraries and the inc dir
    link_directories(${ROOT_LIBDIR})
    include_directories(SYSTEM ${ROOT_INCLUDE_DIR})
    set(ROOT_FOUND TRUE)

else()
    message(FATAL_ERROR "ROOT installation not found!")
endif()
