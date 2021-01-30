set(layers_SOURCES 	layer.h
			layer_container.h layer_container.cpp
			imgui_layer.h imgui_layer.cpp
			)

list(TRANSFORM layers_SOURCES PREPEND "layers/")