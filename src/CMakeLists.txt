project(sqldb_exe)

add_subdirectory(util)
add_subdirectory(pagefile)

add_executable(sqldb main.cpp)

target_link_libraries(sqldb pagefile util)

