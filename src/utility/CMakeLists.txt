set(utility_SOURCES		file_reader.h file_reader.cpp
				)

list(TRANSFORM utility_SOURCES PREPEND "utility/")
list(APPEND cornell_box_SOURCES ${utility_SOURCES})