set(utility_SOURCES		file_reader.h file_reader.cpp
						random_numbers.h random_numbers.cpp
				)

list(TRANSFORM utility_SOURCES PREPEND "utility/")
list(APPEND cornell_box_SOURCES ${utility_SOURCES})