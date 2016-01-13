#
# CMake wrapper to call Qt windeployqt tool in Windows
#
function( WinDeployQt )
	cmake_parse_arguments( _deploy
		"COMPILER_RUNTIME;FORCE"
		"TARGET"
		"INCLUDE_MODULES;EXCLUDE_MODULES"
		${ARGN}
		)

	if( NOT _deploy_TARGET )
		message( FATAL_ERROR "A TARGET must be specified" )
	endif()
	if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
		list( APPEND _ARGS --debug )
	elseif( CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" )
		list( APPEND _ARGS --release-with-debug-info )
	elseif( CMAKE_BUILD_TYPE STREQUAL "Release" )
		list( APPEND _ARGS --release )
	endif()
	if( _deploy_COMPILER_RUNTIME )
		list( APPEND _ARGS --compiler-runtime )
	endif()
	if( _deploy_FORCE )
		list( APPEND _ARGS --force )
	endif()
    # FIXME: make option
    list( APPEND _ARGS --no-translations )
    list( APPEND _ARGS --no-system-d3d-compiler )
    list( APPEND _ARGS --no-angle )
    #

	foreach( mod ${_deploy_INCLUDE_MODULES} )
		string( TOLOWER ${mod} mod )
		string( REPLACE "qt5::" "" mod ${mod} )
		list( APPEND _ARGS "--${mod}" )
	endforeach()
	foreach( mod ${_deploy_EXCLUDE_MODULES} )
		string( TOLOWER ${mod} mod )
		string( REPLACE "qt5::" "" mod ${mod} )
		list( APPEND _ARGS "--no-${mod}" )
	endforeach()
    message( STATUS "windeployqt args are: ${_ARGS}" )

	find_program( _deploy_PROGRAM windeployqt PATHS $ENV{QTDIR}/bin/ )
	if( _deploy_PROGRAM )
		message( STATUS "Found ${_deploy_PROGRAM}" )
	else()
		message( FATAL_ERROR "Unable to find windeployqt" )
	endif()

	if( COMPILER_RUNTIME AND NOT $ENV{VVVV} )
		message( WARNING "WinDeployQt.cmake: VVVV environment variable is not not set, the VC++ redistributable installer will NOT be bundled" )
	endif()

	add_custom_target( windeployqt ${_deploy_PROGRAM} ${_ARGS}
		$<TARGET_FILE:${_deploy_TARGET}>
		DEPENDS ${_deploy_TARGET} )
endfunction()
