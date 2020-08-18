# CMakeList.txt : CMake project for MiniSQL, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project("MiniSQL")

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
	add_compile_options(/GR)
endif()

include_directories(inc)

set(project_sources 
    "src/block_mgr.cpp"
	"src/catalog_mgr.cpp"
	"src/common.cpp"
	"src/console_util.cpp"
	"src/expression.cpp"
	"src/index.cpp"
	"src/index_mgr.cpp"
	"src/query_executor.cpp"
	"src/query_lexer.cpp"
	"src/query_parser.cpp"
	"src/record_mgr.cpp"
	"src/scanner.cpp"
	"src/storage_engine.cpp"
)

set(tests
	"test/01_catalog.cpp"
	"test/02_record.cpp"
	"test/03_scanner.cpp"
	"test/04_parser.cpp"
	"test/05_executor.cpp"
	"test/06_sqltest.cpp"
)

add_executable (MiniSQL "src/main.cpp" ${project_sources})

# Tests
add_executable (TestEntry "test/test.cpp" ${project_sources} ${tests})

add_custom_command(
		TARGET TestEntry POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
				${CMAKE_SOURCE_DIR}/sql
				${CMAKE_CURRENT_BINARY_DIR}/sql)