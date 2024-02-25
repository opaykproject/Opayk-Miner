include_directories(BEFORE ..)

add_library(rx randomx.cpp randomx.c)
target_link_libraries(rx randomx)
