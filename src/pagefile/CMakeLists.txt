set(pagefile_source file_manager.h file_manager.cpp buffer_manager.h buffer_manager.cpp lru.h hashmap.h)

add_library(pagefile ${pagefile_source})