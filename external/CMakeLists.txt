if(POLICY CMP0054)
   # skip policies not known to earlier cmake versions
   cmake_policy(SET CMP0054 NEW)
endif(POLICY CMP0054)

find_package(PkgConfig)

# #### externals that are build with normal compiler flags ####

#project specific setup for gtest/gmock
set(gtest_force_shared_crt true CACHE BOOL "Must be enabled for linking our unit tests" FORCE)
IF(ramses-sdk_BUILD_TESTS AND NOT TARGET gmock)
    pkg_check_modules(gmock_main gmock)
    pkg_check_modules(gmock gmock)

    IF (gmock_FOUND AND gmock_main_FOUND)
        ACME_INFO("+ googletest (platform)")
    ELSE()
        set(INSTALL_GTEST OFF CACHE INTERNAL "")
        set(INSTALL_GMOCK OFF CACHE INTERNAL "")

        ADD_SUBDIRECTORY(googletest)
        target_include_directories(gmock INTERFACE "googletest/googletest/include")
        target_include_directories(gmock_main INTERFACE "googletest/googletest/include")
        target_compile_options(gtest PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)
        target_compile_options(gmock PRIVATE $<$<CXX_COMPILER_ID:Clang>:-Wno-missing-field-initializers>)

        SET_PROPERTY(TARGET gmock      PROPERTY FOLDER "external")
        SET_PROPERTY(TARGET gmock_main PROPERTY FOLDER "external")
        SET_PROPERTY(TARGET gtest      PROPERTY FOLDER "external")
        SET_PROPERTY(TARGET gtest_main PROPERTY FOLDER "external")
        ACME_INFO("+ googletest (custom)")
    ENDIF()

ELSE()
    ACME_INFO("- googletest")
ENDIF()

ADD_SUBDIRECTORY(capu)

include(../cmake/ramses/adaptCompilerFlagsForExternals.cmake)

# these directories will be built with
# modified compiler flags

#==========================================================================
# glslang
#==========================================================================
ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    ramses-glslang
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            glslang/glslang
                            glslang/glslang/MachineIndependent
                            glslang
    FILES_PRIVATE_HEADER    glslang/glslang/Include/*.h
                            glslang/glslang/Public/*.h
                            glslang/glslang/OSDependent/*.h
                            glslang/glslang/MachineIndependent/*.h
                            glslang/OGLCompilersDLL/*.h
    FILES_SOURCE            glslang/glslang/MachineIndependent/*.cpp
                            glslang/glslang/MachineIndependent/preprocessor/*.cpp
                            glslang/glslang/GenericCodeGen/*.cpp
                            glslang/OGLCompilersDLL/*.cpp
                            glslang-os-dep/GenericSingleThreaded/ossource.cpp
                            )
IF("${TARGET_OS}" STREQUAL "Integrity")
    # prevent error because __inline not allowed in c++ code
    TARGET_COMPILE_DEFINITIONS(ramses-glslang PRIVATE "-D__inline=inline")
ENDIF()

set(ENV{FREETYPE_DIR} "../freetype")

# find freetype with harfbuzz support
IF (ramses-sdk_ALLOW_PLATFORM_FREETYPE)
    find_package(FreetypeAndHarfbuzz QUIET)
ENDIF()
IF (ramses-sdk_ALLOW_CUSTOM_FREETYPE AND NOT freetype_FOUND)
    # config and include harfbuzz
    set(SKIP_INSTALL_HEADERS TRUE)
    if (ramses-sdk_INSTALL_STATIC_LIBS)
        set(SKIP_INSTALL_ALL OFF)
    else()
        set(SKIP_INSTALL_ALL ON)
    endif()

    # Set Harfbuzz cache variables to configure its freetype dependencies
    set(FREETYPE_INCLUDE_DIR_ft2build "${CMAKE_CURRENT_SOURCE_DIR}/freetype/include")
    set(FREETYPE_INCLUDE_DIR_freetype2 "${CMAKE_CURRENT_SOURCE_DIR}/freetype/include")
    set(FREETYPE2_ROOT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/freetype")
    set(FREETYPE_LIBRARY freetype)
    set(HB_HAVE_FREETYPE ON CACHE BOOL "Enable freetype interop helpers" FORCE)

    ADD_SUBDIRECTORY(harfbuzz)
    ACME_INFO("+ harfbuzz")

    # config and include freetype
    target_include_directories(harfbuzz INTERFACE "harfbuzz/src" "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz")
    ADD_SUBDIRECTORY(freetype)

    target_include_directories(freetype INTERFACE "freetype/include" "${CMAKE_CURRENT_BINARY_DIR}/harfbuzz/src")

    set_property(TARGET freetype PROPERTY FOLDER "external")
    set_property(TARGET dist PROPERTY FOLDER "external")
    set_property(TARGET harfbuzz PROPERTY FOLDER "external")
    set_property(TARGET harfbuzz-subset PROPERTY FOLDER "external")
    set_property(TARGET hb-ot-tag PROPERTY FOLDER "external")
    set_property(TARGET main PROPERTY FOLDER "external")
    set_property(TARGET test-buffer-serialize PROPERTY FOLDER "external")
    set_property(TARGET test-size-params PROPERTY FOLDER "external")
    set_property(TARGET test-test PROPERTY FOLDER "external")
    set_property(TARGET test-unicode-ranges PROPERTY FOLDER "external")
    set_property(TARGET test-would-substitute PROPERTY FOLDER "external")

    ACME_INFO("+ freetype")
ENDIF()


IF (ramses-sdk_ENABLE_DLT)
    FIND_PACKAGE(automotive-dlt QUIET)
ENDIF()
IF (automotive-dlt_FOUND)
    SET(ramses-sdk_HAS_DLT TRUE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("+ automotive-dlt (platform)")
ELSEIF()
    SET(ramses-sdk_HAS_DLT FALSE CACHE BOOL "dlt found" FORCE)
    ACME_INFO("- automotive-dlt")
ENDIF()


# Must be delayed, so that target can find dependent target like EGL etc.
# For Open Source compliance, source code must remain under external/
# Therefore subdirectory 'wayland-ivi-example-client' is included in toplevel cmake at the end
#ADD_SUBDIRECTORY(wayland-ivi-example-client)

# wayland ivi extension library
IF (NOT ramses-sdk_DISABLE_WAYLAND_IVI_EXTENSION)
    FIND_PACKAGE(wayland-ivi-extension QUIET)
    IF (wayland-ivi-extension_FOUND)
        ACME_INFO("+ wayland-ivi-extension (system)")
    ELSE()
        FIND_PACKAGE(wayland-client QUIET)
        FIND_PACKAGE(wayland-server QUIET)
        IF (wayland-client_FOUND AND wayland-server_FOUND)
            ACME_MODULE(
                NAME                    wayland-ivi-extension
                TYPE                    STATIC_LIBRARY
                ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

                INCLUDE_BASE            wayland-ivi-extension/ivi-extension-protocol
                FILES_PRIVATE_HEADER    wayland-ivi-extension/ivi-extension-protocol/*.h
                FILES_SOURCE            wayland-ivi-extension/ivi-extension-protocol/*.c
                )
        ENDIF()
    ENDIF()
ENDIF()

# LZ4 de-/compression library
OPTION(ramses-sdk_ALLOW_PLATFORM_LZ4 "Enable to search for platform provided lz4" ON)

IF (ramses-sdk_ALLOW_PLATFORM_LZ4)
    pkg_check_modules(lz4 liblz4)
ENDIF()

IF (lz4_FOUND)
    ACME_INFO("+ lz4 (system)")
ELSE()
  SET(LZ4_DIR lz4-r131)
  ACME_MODULE(

    #==========================================================================
    # general module information
    #==========================================================================
    NAME                    lz4
    TYPE                    STATIC_LIBRARY
    ENABLE_INSTALL          ${ramses-sdk_INSTALL_STATIC_LIBS}

    #==========================================================================
    # files of this module
    #==========================================================================
    INCLUDE_BASE            ${LZ4_DIR}/lib
    FILES_PRIVATE_HEADER    ${LZ4_DIR}/lib/*.h
    FILES_SOURCE            ${LZ4_DIR}/lib/*.c
    )
ENDIF()
