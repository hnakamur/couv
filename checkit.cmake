# ADD_CHECKIT_TEST
# Runs Lua script `_testfile` under checkit tester.
# Under LuaDist, set test=true in config.lua to enable testing.
# USE: add_checkit_test(lua test/test1.lua)
macro(add_checkit_test lua _testfile)
	if (NOT SKIP_TESTING)
		get_filename_component(CHECKITABS "tool/checkit" ABSOLUTE)
		get_filename_component(TESTFILEABS ${_testfile} ABSOLUTE)
    add_custom_target(test COMMAND ${lua} ${CHECKITABS} ${TESTFILEABS})
	endif()
endmacro()
