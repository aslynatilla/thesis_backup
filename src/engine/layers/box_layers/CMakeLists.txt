set(box_layers_SOURCES      scene_layer.h scene_layer.cpp
                            camera_layer.h camera_layer.cpp
                            )

list(TRANSFORM box_layers_SOURCES PREPEND box_layers/)
list(APPEND layers_SOURCES ${box_layers_SOURCES})