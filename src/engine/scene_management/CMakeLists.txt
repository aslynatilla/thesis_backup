set(scene_management_SOURCES
                                scene_object.h scene_object.cpp
                                scene_loading.h scene_loading.cpp
                                light.h light.cpp
                                point_light.h point_light.cpp
                                spotlight.h spotlight.cpp
        )

list(TRANSFORM scene_management_SOURCES PREPEND "scene_management/")