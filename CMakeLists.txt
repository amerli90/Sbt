cmake_minimum_required (VERSION 3.11)
project (Sbt CXX)

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_MODULE_PATH "${Sbt_SOURCE_DIR}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "6.2")
    message(FATAL_ERROR "Insufficient gcc version, required >= 6.2")
  endif()
endif()

find_package(ROOT REQUIRED)

if (ROOT_VERSION_MAJOR LESS 6)
    message(FATAL_ERROR "ROOT 6 is required.")
endif()

if (NOT ROOT_HAS_CXX11 AND NOT ROOT_HAS_CXX14 AND NOT ROOT_HAS_CXX17)
    message(FATAL_ERROR "C++11 is the minimum requirement.")
endif()

# Enable C++11 by default if found in ROOT
if(ROOT_HAS_CXX11)
    message(STATUS "Enabling C++11")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# Enable C++14 by default if found in ROOT
if(ROOT_HAS_CXX14)
    message(STATUS "Enabling C++14")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
endif()

# Enable C++17 by default if found in ROOT
if(ROOT_HAS_CXX17)
    message(STATUS "Enabling C++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

find_package(Yaml REQUIRED)

set(LIBDEPS "-lyaml-cpp -lGeom -lGeomPainter -lPhysics -lPostscript")

include(CMakeSbt)

set(SRCS    SbtAdvancedMultipleScattering.cpp
            SbtAlignGeom.cpp
            SbtAlignmentAlg.cpp
            SbtAlignment.cpp
            SbtBentCrystalPatRecAlg.cpp
            SbtBentCrystalFittingAlg.cpp
            SbtCluster.cpp
            SbtClusteringAlg.cpp
            SbtConfig.cpp
            SbtConstrainedPatRecAlg.cpp
            SbtCorrelatedMultipleScattering.cpp
            SbtCosmicGenAlg.cpp
            SbtCrystalChanneling.cpp
            SbtDetectorElem.cpp
            SbtDetectorType.cpp
            SbtDigi.cpp
            SbtEvent.cpp
            SbtEventRawReader.cpp
            SbtEventReader.cpp
            SbtFittingAlg.cpp
            SbtGenAlg.cpp
            SbtGenerator.cpp
            SbtHit.cpp
            SbtIO.cpp
            SbtLineSegment.cpp
            SbtMakeClusters.cpp
            SbtMakeHits.cpp
            SbtMakeSpacePoints.cpp
            SbtMakeTracks.cpp
            SbtMultipleScattering.cpp
            SbtNtupleDumper.cpp
            SbtPatRecAlg.cpp
            SbtPixelClusteringAlg.cpp
            SbtPixelDetectorElem.cpp
            SbtRecursivePatRecAlg.cpp
            SbtSimple3DFittingAlg.cpp
            SbtSimpleAlignmentAlg.cpp
            SbtSimpleClusteringAlg.cpp
            SbtSimpleFittingAlg.cpp
            SbtSimpleGenAlg.cpp
            SbtSimpleMultipleScattering.cpp
            SbtSimplePatRecAlg.cpp
            SbtSingleSideDetectorElem.cpp
            SbtSingleSidePatRecAlg.cpp
            SbtSpacePoint.cpp
            SbtStripDetectorElem.cpp
            SbtStripletsDetectorElem.cpp
            SbtTrack.cpp
            SbtTrackViewer.cpp
)

string(REPLACE ".cpp" ".h" HDRS "${SRCS}")
get_directory_property(incdirs INCLUDE_DIRECTORIES)
generate_dictionary("Sbt" "SbtLinkDef.h" "${HDRS}" "${incdirs}")

set(HDRS            ${HDRS}
                    SbtBit_operations.h
                    SbtDef.h SbtEnums.h
                    SbtError_management.h
                    SbtTriggerInfo.h
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(Sbt SHARED ${SRCS} "G__Sbt.cpp")
target_link_libraries(Sbt ${ROOT_LIBRARIES} ${LIBDEPS})

install(TARGETS Sbt DESTINATION lib)
install(FILES ${HDRS} pdt.table DESTINATION include)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libSbt.rootmap DESTINATION lib)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/G__Sbt_rdict.pcm DESTINATION lib)
