#
# QGit main CMakeLists.txt
#
cmake_minimum_required( VERSION 2.6 )

if( POLICY CMP0020 )
    cmake_policy( SET CMP0020 OLD )
endif()

if( POLICY CMP0043 )
    cmake_policy( SET CMP0043 OLD )
endif()

if( POLICY CMP0048 )
    cmake_policy( SET CMP0048 OLD )
endif()

if( POLICY CMP0064 )
    cmake_policy( SET CMP0064 OLD )
endif()

set( PROJECT_NAME "qgit" )

#
# Default to "Release" build type
# NOTE: user-provided value for CMAKE_BUILD_TYPE must be checked before the PROJECT() call
#
if( DEFINED CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Type of build" )
else()
   set( CMAKE_BUILD_TYPE Release CACHE STRING "Type of build" )
endif()

#
# A project name is needed for CPack
#
project( "${PROJECT_NAME}" )

#
# Set conventional loops
#
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )

#
# Search path for cmake modules
#
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake )

#
# Define a proper install path
#
if( UNIX )
    if( APPLE )
        #
        # Mac OS X
        # Due to the special bundle structure ignore the prefix eventually set by the user
        #
        set( CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release )
    else()
        #
        # Linux / BSD
        #
        if( CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT )
            #
            # Fix package build
            #
            if( PREFIX )
                set( CMAKE_INSTALL_PREFIX ${PREFIX} )
            else()
                set( CMAKE_INSTALL_PREFIX /usr/local )
            endif()
        endif()
    endif()
elseif( WIN32 )
    set( CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/release )
endif()

#
# Define proper compilation flags
#
if( MSVC )
    #
	# Visual Studio: maximum optimization
    #
    set( CMAKE_CXX_FLAGS_RELEASE "/Ox /MD" )
    #
	# Generate complete debugging information
    #
    #set(CMAKE_CXX_FLAGS_DEBUG "/Zi")
elseif( CMAKE_COMPILER_IS_GNUCXX )
    #
    # Linux/GCC, BSD/GCC, Windows/MinGW
    #
    include( CheckCXXCompilerFlag )

    set( CMAKE_CXX_FLAGS_RELEASE "-s -O2" )
    set( CMAKE_CXX_FLAGS_DEBUG "-ggdb -O0 -Wall -Wextra -Werror" )

    set( ADDITIONAL_DEBUG_FLAGS -Wcast-align -Wmissing-declarations -Wno-long-long -Wno-error=extra -Wno-error=delete-non-virtual-dtor -Wno-error=sign-compare -Wno-error=missing-declarations )

    foreach( FLAG ${ADDITIONAL_DEBUG_FLAGS} )
        check_cxx_compiler_flag( "${FLAG}" CXX_HAS_WARNING_${FLAG} )
        if( CXX_HAS_WARNING_${FLAG} )
            set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${FLAG}" )
        endif()
    endforeach()
else()
    #
    # Other: Mac OS X/LLVM, BSD/LLVM
    #
    set( CMAKE_CXX_FLAGS_RELEASE "-O2" )
    set( CMAKE_CXX_FLAGS_DEBUG "-g -O0" )
endif()

#
# Find Qt and enable the needed features
# Default is Qt5 unless WITH_QT4 option is enabled
#
option( WITH_QT4 "Force the use of Qt4 libraries" OFF )
option( UPDATE_TRANSLATIONS "Update translations on compile" OFF )
message( STATUS "Update translations mode: ${UPDATE_TRANSLATIONS}" )

if( NOT WITH_QT4 )
    #
    # First known not-broken Qt5 version
    # NOTE: 5.0.2 available on old Ubuntu distros is buggy
    #
    find_package( Qt5Widgets 5.0.3 )
endif()

if( Qt5Widgets_FOUND )
    message( STATUS "Found Qt ${Qt5Widgets_VERSION_STRING}" )

    #
    # FIX: Qt was built with -reduce-relocations
    #
    if( Qt5_POSITION_INDEPENDENT_CODE )
        set( CMAKE_POSITION_INDEPENDENT_CODE ON )
    endif()

    find_package( Qt5LinguistTools )
    if( UPDATE_TRANSLATIONS )
        if( NOT Qt5_LUPDATE_EXECUTABLE )
            message( WARNING "Qt's lupdate tool not found!" )
        endif()
    endif()

    if( NOT Qt5_LRELEASE_EXECUTABLE )
        message( WARNING "Qt's lrelease tool not found!" )
    endif()
else()
    find_package( Qt4 4.8.0 REQUIRED )
    if( QT4_FOUND )
        #
        # Old FindQt4.cmake modules used the same flag with different case
        #
        set( Qt4_FOUND TRUE )
    endif()
    if( Qt4_FOUND )
        message( STATUS "Found Qt ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}" )

        if( UPDATE_TRANSLATIONS )
            if( NOT QT_LUPDATE_EXECUTABLE)
                message( WARNING "Qt's lupdate tool not found!" )
            endif()
        endif()

        if( NOT QT_LRELEASE_EXECUTABLE )
            message( WARNING "Qt's lrelease tool not found!" )
        endif()
    else()
        message( FATAL_ERROR "No Qt4 or Qt5 found! Aborting" )
    endif()
endif()

set( CMAKE_AUTOMOC TRUE )
#set( CMAKE_AUTOUIC TRUE )

#
# Package builder
#
set( CPACK_PACKAGE_CONTACT              "axill777@gmail.com" )
set( CPACK_PACKAGE_DESCRIPTION_SUMMARY  ${PROJECT_NAME} )
set( CPACK_PACKAGE_VENDOR               "Alexander Kamyshnikov" )
set( CPACK_PACKAGE_DESCRIPTION_FILE     "${PROJECT_SOURCE_DIR}/README.md" )
set( CPACK_RESOURCE_FILE_LICENSE        "${PROJECT_SOURCE_DIR}/LICENSE" )
set( CPACK_PACKAGE_VERSION_MAJOR        "${PROJECT_VERSION_MAJOR}" )
set( CPACK_PACKAGE_VERSION_MINOR        "${PROJECT_VERSION_MINOR}" )
set( CPACK_PACKAGE_VERSION_PATCH        "${PROJECT_VERSION_PATCH}" )

if( UNIX )
    if( APPLE )
        set( CPACK_GENERATOR           DragNDrop ${CPACK_GENERATOR} )
        set( CPACK_GENERATOR          "DragNDrop" )
        set( CPACK_DMG_FORMAT         "UDBZ" )
        set( CPACK_DMG_VOLUME_NAME    "${PROJECT_NAME}" )
        set( CPACK_SYSTEM_NAME        "OSX" )
        set( CPACK_PACKAGE_FILE_NAME  "${PROJECT_NAME}-${PROJECT_VERSION}" )
        set( CPACK_PACKAGE_ICON       "${CMAKE_CURRENT_SOURCE_DIR}/src/resources/qgit.icns" )
    else()
        #
        # Linux/BSD
        # FIXME: add at least RPM package generation
        #
        set( CPACK_GENERATOR                 DEB ${CPACK_GENERATOR})
        set( CPACK_PACKAGE_FILE_NAME        "${PROJECT_NAME}-${PROJECT_VERSION}")
    	set( CPACK_DEBIAN_PACKAGE_SHLIBDEPS  ON)
    	set( CPACK_DEBIAN_PACKAGE_SECTION   "vcs")
    	set( CPACK_DEBIAN_PACKAGE_HOMEPAGE  "http://github.com/eraxillan/qgit5")
    endif()
elseif( WIN32 )
    set( CPACK_GENERATOR          NSIS ${CPACK_GENERATOR} )
    set( CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}" )

    #
    # Configure file with custom definitions for NSIS
    #
    configure_file(
        ${CMAKE_MODULE_PATH}/NSIS.definitions.nsh.in
        ${PROJECT_BINARY_DIR}/NSIS.definitions.nsh
    )
endif()

include(CPack)

###############################################################################

set( qgit_HEADERS
     src/annotate.h
     src/cache.h
     src/commitimpl.h
     src/common.h
     src/config.h
     src/consoleimpl.h
     src/customactionimpl.h
     src/dataloader.h
     src/domain.h
     src/exceptionmanager.h
     src/filecontent.h
     src/filelist.h
     src/fileview.h
     src/git.h
     src/help.h
     src/lanes.h
     src/listview.h
     src/mainimpl.h
     src/myprocess.h
     src/patchcontent.h
     src/patchview.h
     src/rangeselectimpl.h
     src/revdesc.h
     src/revsview.h
     src/settingsimpl.h
     src/smartbrowse.h
     src/treeview.h
     src/FileHistory.h
)

set( qgit_SOURCES
     src/annotate.cpp
     src/cache.cpp
     src/commitimpl.cpp
     src/consoleimpl.cpp
     src/customactionimpl.cpp
     src/dataloader.cpp
     src/domain.cpp
     src/exceptionmanager.cpp
     src/filecontent.cpp
     src/filelist.cpp
     src/fileview.cpp
     src/git.cpp
     src/lanes.cpp
     src/listview.cpp
     src/mainimpl.cpp
     src/myprocess.cpp
     src/namespace_def.cpp
     src/patchcontent.cpp
     src/patchview.cpp
     src/qgit.cpp
     src/rangeselectimpl.cpp
     src/revdesc.cpp
     src/revsview.cpp
     src/settingsimpl.cpp
     src/smartbrowse.cpp
     src/treeview.cpp
     src/FileHistory.cc
     src/common.cpp
)

set( qgit_FORMS
     src/commit.ui
     src/console.ui
     src/customaction.ui
     src/fileview.ui
     src/help.ui
     src/mainview.ui
     src/patchview.ui
     src/rangeselect.ui
     src/revsview.ui
     src/settings.ui
)

set( qgit_RESOURCES src/icons.qrc )

#
# FIXME: add translations
#
#if( UPDATE_TRANSLATIONS )
#    file( GLOB_RECURSE translate_SRCS ${CMAKE_SOURCE_DIR}/src/*.cpp ${CMAKE_SOURCE_DIR}/src/*.h )
#    set( qgit_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/qgit_en.ts" )
#else()
#    file( GLOB qgit_TS "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts" )
#endif()

if( WIN32 )
    set( qgit_SOURCES ${qgit_SOURCES} src/app_icon.rc )
endif()

if( APPLE )
    set( MACOSX_BUNDLE_ICON_FILE src/resources/qgit.icns )
    set_source_files_properties( ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/qgit.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources )
    set( qgit_SOURCES ${qgit_SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/qgit.icns )
endif()

set( QGIT_LIBS )

#
# Qt4 stuff
#
if( Qt4_FOUND )
    #
    # Include directories
    #
    include( ${QT_USE_FILE} )
    include_directories( ${QT_INCLUDES} )
    list( APPEND QGIT_LIBS ${QT_LIBRARIES} )
    list( APPEND QGIT_LIBS ${QT_QTMAIN_LIBRARY} )

    #
    # FIXME: Let CMake chew Qt4's translations and resource files
    # NOTE: header files are MOC-ed automatically by CMake
    #
#    if( UPDATE_TRANSLATIONS )
#        qt4_create_translation( qgit_QM ${translate_SRCS} ${qgit_TS} )
#    else()
#        qt4_add_translation( qgit_QM ${qgit_TS} )
#    endif()
    
    qt4_wrap_ui( qgit_FORMS_UIC ${qgit_FORMS} )
    qt4_add_resources( qgit_RESOURCES_RCC ${qgit_RESOURCES} )
endif()

#
# Qt5 stuff
#
if( Qt5Widgets_FOUND )
    include_directories( ${Qt5Widgets_INCLUDE_DIRS} )
    list( APPEND QGIT_LIBS Widgets )

    # FIXME: Qt5LinguistTools
#    find_package( Qt5LinguistTools )
#    if( Qt5LinguistTools_FOUND )
#        include_directories( ${Qt5LinguistTools_INCLUDE_DIRS} )
#        list( APPEND QGIT_LIBS LinguistTools )
#    endif()

    #
    # FIXME: Let CMake chew Qt5's translations and resource files
    # NOTE: header files are MOC-ed automatically by CMake
    #
#    if( UPDATE_TRANSLATIONS )
#        qt5_create_translation( qgit_QM ${translate_SRCS } ${qgit_TS} )
#    else()
#        qt5_add_translation( qgit_QM ${qgit_TS} )
#    endif()

    qt5_wrap_ui( qgit_FORMS_UIC ${qgit_FORMS} )
    qt5_add_resources( qgit_RESOURCES_RCC ${qgit_RESOURCES} )

    #
    # Guess plugins and libraries directory
    #
    set( QT_PLUGINS_DIR "${Qt5Widgets_DIR}/../../../plugins" )
    get_target_property( QT_LIBRARY_DIR Qt5::Core LOCATION )
    get_filename_component( QT_LIBRARY_DIR ${QT_LIBRARY_DIR} PATH )
endif()

#
# Declare path variables
#
set( ICONDIR share/icons CACHE STRING "Icon directory" )
set( DESKTOPDIR share/applications CACHE STRING "Desktop file destination" )

#
# Include directories
# NOTE: *_ui.h files will be generated in CMAKE_CURRENT_BINARY_DIR directory
#
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

#
# Build QGit binary and link it
#
add_executable( qgit WIN32 MACOSX_BUNDLE
    ${qgit_HEADERS}
    ${qgit_SOURCES}
    ${qgit_FORMS}
    # ${qgit_QM}
    ${qgit_RESOURCES_RCC}
    ${qgit_FORMS_UIC}
)
set_property( TARGET qgit PROPERTY CXX_STANDARD 11 )
set_property( TARGET qgit PROPERTY CXX_STANDARD_REQUIRED ON )

if( Qt4_FOUND )
    if( MSVC )
        set( QT_USE_QTMAIN true )
    endif()
    target_link_libraries( qgit ${QGIT_LIBS} )
endif()
if( Qt5Widgets_FOUND )
    if( MSVC )
        target_link_libraries( qgit Qt5::WinMain )
    endif()
    qt5_use_modules( qgit ${QGIT_LIBS} )
endif()

if( UNIX )
    if( APPLE )
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
        set(MACOSX_BUNDLE_GUI_IDENTIFIER "net.sourceforge.${PROJECT_NAME}")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME}-${PROJECT_VERSION}")
        set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION})
        set_target_properties(qgit PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist)

        install( TARGETS qgit BUNDLE DESTINATION ./ )
        # FIXME:
#        install( FILES ${qgit_QM} DESTINATION ./qgit.app/Contents/Resources/translations )
    else()
        #
        # Assume Linux
        # TODO: BSD/Unix-specific?
        #
        install( TARGETS qgit RUNTIME DESTINATION bin/ )
        install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/qgit.png DESTINATION ${ICONDIR}/hicolor/48x48/apps )
        # FIXME:
#        install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/resources/qgit.svg DESTINATION ${ICONDIR}/hicolor/scalable/apps )
#        install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/qgit.desktop DESTINATION ${DESKTOPDIR} )
#        install( FILES ${qgit_QM} DESTINATION share/qgit/translations )
    endif()
elseif( WIN32 )
        install( TARGETS qgit RUNTIME DESTINATION ./ )
        install( FILES ${qgit_QM} DESTINATION ./translations )
endif()

if( APPLE )
    #
    # These needs to be relative to CMAKE_INSTALL_PREFIX
    #
    set( plugin_dest_dir qgit.app/Contents/Plugins )
    set( qtconf_dest_dir qgit.app/Contents/Resources )
    get_filename_component( QT_LIBRARY_DIR "${QT_LIBRARY_DIR}/.." ABSOLUTE )

    #
    # FIXME: replace this ugly manual copying with macdeployqt call, using neovim-qt CMakeLists.txt
    #
    
    #
    # Qt4: iconengines, imageformats
    # Qt5: iconengines, imageformats, platforms, printsupport
    #
    install( DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(iconengines|imageformats|platforms|printsupport)/.*\\.dylib"
        REGEX ".*_debug\\.dylib" EXCLUDE)

    install( CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    " COMPONENT Runtime)

    install( CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dylib\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/qgit.app\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime)
endif()

if( WIN32 )
    #
    # These needs to be relative to CMAKE_INSTALL_PREFIX
    #
    set( plugin_dest_dir Plugins )
    set( qtconf_dest_dir . )
    
    #
    # FIXME: replace this ugly manual copying with windeployqt.exe call, using neovim-qt CMakeLists.txt
    #
    
    #
    # Qt4: iconengines, imageformats
    # Qt5: iconengines, imageformats, platforms, printsupport
    #
    install( DIRECTORY "${QT_PLUGINS_DIR}/" DESTINATION ${plugin_dest_dir} COMPONENT Runtime
        FILES_MATCHING REGEX "(iconengines|imageformats|platforms|printsupport)/.*[^d]\\.dll" )

    install( CODE "
        file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"[Paths]
Plugins = Plugins
Translations = Resources/translations
Data = Resources\")
    " COMPONENT Runtime)

    install( CODE "
        file(GLOB_RECURSE QTPLUGINS
            \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/*.dll\")
        set(BU_CHMOD_BUNDLE_ITEMS ON)
        include(BundleUtilities)
        fixup_bundle(\"\${CMAKE_INSTALL_PREFIX}/qgit.exe\" \"\${QTPLUGINS}\" \"${QT_LIBRARY_DIR}\")
        " COMPONENT Runtime )
endif()
