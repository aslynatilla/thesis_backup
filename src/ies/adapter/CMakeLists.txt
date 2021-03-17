set(adapter_SOURCES		ies_mesh.h ies_mesh.cpp
				ies_mesh_adapter.h ies_mesh_adapter.cpp
				ies_mesh_type_specific_adapter.h ies_mesh_type_specific_adapter.cpp
				)

list(TRANSFORM adapter_SOURCES PREPEND "adapter/")

