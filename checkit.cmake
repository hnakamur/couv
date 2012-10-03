# ADD_CHECKIT_TEST
# Runs Lua script `_testfile` under checkit tester.
# Under LuaDist, set test=true in config.lua to enable testing.
# USE: add_checkit_test(test/test1.lua)
macro(add_checkit_test _testfile)
	if (NOT SKIP_TESTING)
		get_filename_component(CHECKITABS "tool/checkit" ABSOLUTE)
		get_filename_component(TESTFILEABS ${_testfile} ABSOLUTE)
    add_custom_target(test COMMAND ${CHECKITABS} ${TESTFILEABS})
	endif()
endmacro()
