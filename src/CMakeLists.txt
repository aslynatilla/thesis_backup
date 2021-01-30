set(cornell_box_SOURCES main.cpp 
                        glad.c)

list(TRANSFORM cornell_box_SOURCES PREPEND "src/")
