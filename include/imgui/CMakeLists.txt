set(PROJECT_IMGUI_FILES imconfig.h
                        imgui.cpp imgui.h
                        imgui_demo.cpp
                        imgui_draw.cpp
                        imgui_internal.h
                        imgui_stdlib.cpp imgui_stdlib.h
                        imgui_tables.cpp
                        imgui_widgets.cpp
                        imstb_rectpack.h
                        imstb_textedit.h
                        imstb_truetype.h

                        imgui_impl_glfw.cpp imgui_impl_glfw.h
                        imgui_impl_opengl3.cpp imgui_impl_opengl3.h
        )

list(TRANSFORM PROJECT_IMGUI_FILES PREPEND "include/imgui/")
