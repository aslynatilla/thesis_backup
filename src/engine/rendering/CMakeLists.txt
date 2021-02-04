set(renderer_SOURCES 	camera.h camera.cpp
			window.h window.cpp
			renderer.h renderer.cpp
			shader.h shader.cpp
			shader_loading.h shader_loading.cpp
			material.h material.cpp
			vertex_array.h vertex_array.cpp
			vertex_buffer.h vertex_buffer.cpp
			element_buffer.h
			glfw_window_impl.h glfw_window_impl.cpp
			opengl3_context.h opengl3_context.cpp
			rendering_context.h rendering_context.cpp
			)

list(TRANSFORM renderer_SOURCES PREPEND "rendering/")
