include(ExternalProject)

set(loris_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dep/loris)

ExternalProject_Add(loris
    SOURCE_DIR ${loris_SOURCE_DIR}
    UPDATE_DISCONNECTED true # do not attempt to update source on rebuild
    CONFIGURE_COMMAND ${loris_SOURCE_DIR}/configure --prefix=<INSTALL_DIR> --with-python=NO --with-csound=NO --with-utils=NO
    BUILD_COMMAND make -j${Ncpu}
    BUILD_BYPRODUCTS ${loris_LIBRARY}
)

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

add_library(loris::loris INTERFACE IMPORTED GLOBAL)
target_include_directories(loris::loris INTERFACE
    ${PROJECT_BINARY_DIR}/loris-prefix/include
)
target_link_libraries(loris::loris INTERFACE
    ${PROJECT_BINARY_DIR}/loris-prefix/lib/${CMAKE_STATIC_LIBRARY_PREFIX}loris${CMAKE_STATIC_LIBRARY_SUFFIX}
)
# set_target_properties didn't work, but target_link_libraries did work

add_dependencies(loris::loris loris)
