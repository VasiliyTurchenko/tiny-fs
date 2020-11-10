cmake_minimum_required(VERSION 2.8)

project(tiny-fs)

add_executable(${PROJECT_NAME}	"main.c"
	                        "TEST-tiny-fs"
				"tiny-fs.c"
				"mock.c"
				"ls.c"
				"random_test.c"
                "TEST_mediaIO.c"
                "test_f_checkfs.c"
				"testhelpers.c"
                "ascii_helpers.c"
                "test_ascii_helpers.c"
				)
target_include_directories(${PROJECT_NAME} PUBLIC
                "/tiny-fs")

target_compile_definitions(${PROJECT_NAME} PUBLIC DESKTOP_TARGET)
