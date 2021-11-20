##
##  This is the cmake intro
cmake_minimum_required(VERSION 3.10)
##
##  Abort if in source build
if ( ${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR} )
    message( FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt." )
endif()

project(openparsec CXX)
##
##  Load values from the os-release information.
if (EXISTS "/etc/os-release")
    file(STRINGS "/etc/os-release" OSINFO)
    foreach(NameAndValue ${OSINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        string(REPLACE "\"" "" Value ${Value})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
else()
    message(WARNING "os-release info not found. Assuming a windows build.")
endif()
##
##  Load openparsec version number
if (EXISTS ${CMAKE_SOURCE_DIR}/VERSION)
    file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION VINFO)
    foreach(NameAndValue ${VINFO})
        # Strip leading spaces
        string(REGEX REPLACE "^[ ]+" "" NameAndValue ${NameAndValue})
        # Find variable name
        string(REGEX MATCH "^[^=]+" Name ${NameAndValue})
        # Find the value
        string(REPLACE "${Name}=" "" Value ${NameAndValue})
        # Set the variable
        set(${Name} "${Value}")
    endforeach()
    set(OPENPARSEC_VERSION_MAJOR "${VERSION_MAJOR}")
    set(OPENPARSEC_VERSION_MINOR "${VERSION_MINOR}")
    set(OPENPARSEC_VERSION_PATCH "${VERSION_PATCH}")
else()
    message(WARNING "version info not found")
    set(OPENPARSEC_VERSION_MAJOR "0")
    set(OPENPARSEC_VERSION_MINOR "2")
    set(OPENPARSEC_VERSION_PATCH "1")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(
    CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
)
##
##  This is the application install dir. Set fixed for now. Should be changed.
set (XDG_APPS_INSTALL_DIR /usr/local/bin)


set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(FeatureSummary)
include(GNUInstallDirs)
##
##  
add_compile_options (-O0 -g -Wall -std=c++14 -fPIC)

include_directories(src ${CMAKE_SOURCE_DIR})

add_subdirectory(src)

add_custom_target(the_whole ALL)

add_dependencies(the_whole parsec)

##
##
##  Here we describe the files and positions of
##  to be installed files.
## install( PROGRAMS ${CMAKE_BINARY_DIR}/povmodeler DESTINATION  ${CMAKE_INSTALL_BINDIR} )
##
##  desktop file
## install( FILES povmodeler.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
##
##  application specific files
## install( FILES povraydocmap.xml baseinsertrules.xml questionmark.png DESTINATION  ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler)
##
## Examples and libraries.
## install( DIRECTORY examples/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler/examples )
## install( DIRECTORY library/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/povmodeler/library )
##
##  Icon files in various resolutions
## install( FILES hi48-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps/ RENAME povmodeler.png)
## install( FILES hi32-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps/ RENAME povmodeler.png)
## install( FILES hi22-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/22x22/apps/ RENAME povmodeler.png)
## install( FILES hi16-app-povmodeler.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps/ RENAME povmodeler.png)
##
##  Packaging setup.
##
##  Only supported on unix systems.
## IF (UNIX)
##
##  Debian specific switches
## SET (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
## SET (CPACK_DEBIAN_PACKAGE_DEBUG ON)
##
##  RPM specific switches
## set (CPACK_RPM_PACKAGE_DEBUG OFF)
## set (CPACK_RPM_PACKAGE_AUTOREQ ON)
##
##  Some systems do not like it to have the directories in the files to create
## set (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/applications"
##                                                    "/usr/share/icons"
##                                                    "/usr/share/icons/hicolor"
##                                                    "/usr/share/icons/hicolor/16x16"
##                                                    "/usr/share/icons/hicolor/16x16/apps"
##                                                    "/usr/share/icons/hicolor/22x22"
##                                                    "/usr/share/icons/hicolor/22x22/apps"
##                                                    "/usr/share/icons/hicolor/32x32"
##                                                    "/usr/share/icons/hicolor/32x32/apps"
##                                                    "/usr/share/icons/hicolor/48x48"
##                                                   "/usr/share/icons/hicolor/48x48/apps"
##                                                    )

##
##
## SET(CPACK_CMAKE_GENERATOR "Unix Makefiles")
## SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "This is the former KPovModeler now PovModeler because stripped off the KDE stuff")
## SET(CPACK_PACKAGE_VENDOR "Simulated-Universe")
## SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
## SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
## SET(CPACK_PACKAGE_VERSION_MAJOR ${POVMODELER_VERSION_MAJOR})
## SET(CPACK_PACKAGE_VERSION_MINOR ${POVMODELER_VERSION_MINOR})
## set(CPACK_PACKAGE_VERSION_PATCH ${POVMODELER_VERSION_PATCH})
## SET(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
## SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}")
## SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION}-${ID}-${VERSION_ID}")
## SET(CPACK_PACKAGE_CONTACT "Hans-Juergen Lange <hjl@simulated-universe.de>")
## SET(CPACK_PACKAGE_SECTION "graphics")
## INCLUDE(CPack)
## ENDIF (UNIX)
