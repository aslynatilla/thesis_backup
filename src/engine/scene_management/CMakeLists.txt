set(scene_management_SOURCES    scene_object.h scene_object.cpp
                                scene_loading.h scene_loading.cpp
        )

list(TRANSFORM scene_management_SOURCES PREPEND "scene_management/")