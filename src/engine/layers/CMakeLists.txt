set(layers_SOURCES 	layer.h
			layer_container.h layer_container.cpp
			imgui_layer.h imgui_layer.cpp
			)

include(${CMAKE_CURRENT_LIST_DIR}/box_layers/CMakeLists.txt)

list(TRANSFORM layers_SOURCES PREPEND "layers/")