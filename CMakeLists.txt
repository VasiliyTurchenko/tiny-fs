cmake_minimum_required(VERSION 2.8)

project(tiny-fs)
add_executable(${PROJECT_NAME}	"main.c"
	                        "TEST-tiny-fs"
				"tiny-fs.c"
				"mock.c"
				"ls.c"
				"random_test.c"
				"TEST_mediaIO.c"
				"TEST_f_checkFS.c"
				"testhelpers.c"
				)
