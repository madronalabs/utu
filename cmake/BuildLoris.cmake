include(ExternalProject)
include(FindPkgConfig)

set(loris_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dep/loris)

pkg_check_modules(FFTW fftw3 IMPORTED_TARGET)

# The following attempts to work around old crusty autotools, specifically
#
# (1) loris configure does not offer a way to specify an alternate prefix for
# the libraries it wants to leverage, instead one must add set CPPFLAGS/LDFLAGS
# in order for the configure tests to find the needed bits.
#
# (2) Locating fftw is done via pkg-config which will work on macOS (homebrew)
# and Linux, possibly Windows. If pkg-config finds fftw then we inject the
# needed bits into the environment when configuring
#
# (3) "cmake -E env" can't pass environment variables where the values contain
# spaces to a subcommand. The suggested workaround was to generate a script and
# then execute the script.
#
if(FFTW_FOUND)
    ExternalProject_Add(loris
        SOURCE_DIR ${loris_SOURCE_DIR}
        UPDATE_DISCONNECTED true # do not attempt to update source on rebuild
        PATCH_COMMAND ${CMAKE_COMMAND} -E echo "CPPFLAGS=\"${FFTW_STATIC_CFLAGS}\" LDFLAGS=\"${FFTW_STATIC_LDFLAGS}\" ${loris_SOURCE_DIR}/configure --prefix=<INSTALL_DIR> --with-python=NO --with-csound=NO --with-utils=NO" > ${CMAKE_CURRENT_BINARY_DIR}/configure-loris.sh
        CONFIGURE_COMMAND sh ${CMAKE_CURRENT_BINARY_DIR}/configure-loris.sh
        BUILD_COMMAND make -j${Ncpu}
        BUILD_BYPRODUCTS ${loris_LIBRARY}
    )
else()
    ExternalProject_Add(loris
        SOURCE_DIR ${loris_SOURCE_DIR}
        UPDATE_DISCONNECTED true # do not attempt to update source on rebuild
        CONFIGURE_COMMAND ${loris_SOURCE_DIR}/configure --prefix=<INSTALL_DIR> --with-python=NO --with-csound=NO --with-utils=NO --with-fftw=NO
        BUILD_COMMAND make -j${Ncpu}
        BUILD_BYPRODUCTS ${loris_LIBRARY}
    )
endif()

ExternalProject_Get_Property(loris SOURCE_DIR)

ExternalProject_Add_Step(loris
    autoupdate
    COMMAND autoupdate
    DEPENDEES download
    WORKING_DIRECTORY ${SOURCE_DIR}
)

ExternalProject_Add_Step(loris
    touch-changelog
    COMMAND touch ChangeLog
    DEPENDEES download
    WORKING_DIRECTORY ${SOURCE_DIR}
)

ExternalProject_Add_Step(loris
    bootstrap
    COMMAND autoreconf --install
    DEPENDEES autoupdate touch-changelog
    DEPENDERS configure
    WORKING_DIRECTORY ${SOURCE_DIR}
)

# HACK: create the install directories so that the target_include_directories function does not error during configure.
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/loris-prefix/include)  # avoid race condition
file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/loris-prefix/lib)  # avoid race condition

add_library(loris::loris INTERFACE IMPORTED GLOBAL)
target_include_directories(loris::loris INTERFACE
    "${PROJECT_BINARY_DIR}/loris-prefix/include"
)

set(loris_libs
    "${PROJECT_BINARY_DIR}/loris-prefix/lib/${CMAKE_STATIC_LIBRARY_PREFIX}loris${CMAKE_STATIC_LIBRARY_SUFFIX}"
)

# TODO: Figure out how to force static linking of fftw
if(FFTW_FOUND)
    # ensure consumers of the loris target link against the needed libraries
    list(APPEND loris_libs PkgConfig::FFTW)
endif()
target_link_libraries(loris::loris INTERFACE ${loris_libs})

add_dependencies(loris::loris loris)
