include_directories("${BOOST_INCLUDE_DIR}")

set(SOURCE_FILES
	OryxTests.cpp
	OryxColorTest.cpp
	OryxStringUtilsTest.cpp
	)

set(HEADER_FILES
	)

add_executable(OryxTests ${SOURCE_FILES} ${HEADER_FILES} )
set_target_properties(OgreSubsystem PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries (OryxTests ${ORYX_LIB} ${BOOST_TEST_LIB})

add_test(OryxTests ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/OryxTests)
