cmake_minimum_required(VERSION 3.14.7)  # Least supporting empty endif
###############################################################################

project(albert VERSION 0.16.3)
###############################################################################

include(GNUInstallDirs)

###############################################################################

add_definitions(-DALBERT_VERSION="${PROJECT_VERSION}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Set standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set additional compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic") #-Wconversion -Wunused
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DQT_QML_DEBUG")

# Color make output
SET(CMAKE_COLOR_MAKEFILE ON)

# Put the binaries in dedicated toplevel directories
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

###############################################################################

#add_subdirectory(lib/qhotkey)
#add_subdirectory(lib/globalshortcut)
add_subdirectory(lib/xdg)
add_subdirectory(plugins/)

#################################### Lib  #####################################

set(TARGET_NAME_LIB ${PROJECT_NAME}-lib)

file(GLOB_RECURSE LIB_SRC include/* src/lib/*)

add_library(${TARGET_NAME_LIB} SHARED ${LIB_SRC})
add_library(albert::lib ALIAS ${TARGET_NAME_LIB})

set_target_properties(${TARGET_NAME_LIB} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN"
)

target_compile_definitions(${TARGET_NAME_LIB} PRIVATE -DCORE) #export branch

target_include_directories(${TARGET_NAME_LIB}
    PUBLIC
        include
    PRIVATE
        include/albert
        include/albert/util
        src/lib
)

find_package(Qt5 5.5.0
    COMPONENTS
        Gui
        Sql
)

target_link_libraries(${TARGET_NAME_LIB}
    PRIVATE
        Qt5::Gui
        Qt5::Sql
)


install(TARGETS ${TARGET_NAME_LIB} ${TARGET_NAME_BIN}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


#################################### App  #####################################

option(BUILD_WITH_QTCHARTS "Use QtCharts to plot stats." ON)
set(TARGET_NAME_BIN ${PROJECT_NAME}-bin)

file(GLOB_RECURSE BIN_SRC src/app/*)
if(NOT BUILD_WITH_QTCHARTS)  # TODO: remove Apr 2020
    list(REMOVE_ITEM BIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/app/settingswidget/statswidget.h")
    list(REMOVE_ITEM BIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/app/settingswidget/statswidget.cpp")
endif()

add_executable(${TARGET_NAME_BIN} ${BIN_SRC} resources.qrc)

set_target_properties(${TARGET_NAME_BIN} PROPERTIES
    OUTPUT_NAME ${PROJECT_NAME}
    INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}/albert/"  # Set the RPATH for the library lookup
)

target_include_directories(${TARGET_NAME_BIN} PRIVATE src/app src/app/settingswidget)

if (DEFINED CMAKE_LIBRARY_ARCHITECTURE)
    target_compile_definitions(${TARGET_NAME_BIN} PRIVATE -DMULTIARCH_TUPLE="${CMAKE_LIBRARY_ARCHITECTURE}")
endif()

find_package(Qt5 5.5.0
    COMPONENTS
        Concurrent
        Network
        Sql
        Widgets
    OPTIONAL_COMPONENTS
        Charts
)

target_link_libraries(${TARGET_NAME_BIN}
    PRIVATE
        Qt5::Concurrent
        Qt5::Network
        Qt5::Sql
        Qt5::Widgets
        albert::lib
        #globalshortcut
        xdg
)
if(BUILD_WITH_QTCHARTS)
    target_link_libraries(${TARGET_NAME_BIN} PRIVATE Qt5::Charts)
    target_compile_definitions(${TARGET_NAME_BIN} PRIVATE -DBUILD_WITH_QTCHARTS) #export branch
endif()


install(TARGETS ${TARGET_NAME_LIB} ${TARGET_NAME_BIN}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/albert
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


######################### Platform specific stuff #############################
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")

    # Discriminate display server type
    set(DISPLAY_SERVER "x11" CACHE STRING "Linux display server to build against (wayland or x11)." FORCE)
    if(${DISPLAY_SERVER} MATCHES "x11")
        find_package(Qt5 5.5.0 COMPONENTS X11Extras)
        target_link_libraries(${TARGET_NAME_BIN} PRIVATE Qt5::X11Extras)
    elseif(${DISPLAY_SERVER} MATCHES "wayland")
    else()
        message( FATAL_ERROR "Invalid display server specified: ${DISPLAY_SERVER}" )
    endif()

   # Install desktop entry and icon
   install(FILES resources/albert.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications )
   install(FILES resources/icons/albert.svg DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

endif()

