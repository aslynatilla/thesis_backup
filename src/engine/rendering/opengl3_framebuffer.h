#ifndef OPENGL3_FRAMEBUFFER_H
#define OPENGL3_FRAMEBUFFER_H

#include <glad/glad.h>

namespace engine {
    class OpenGL3_FrameBuffer {
    public:
        OpenGL3_FrameBuffer();
        ~OpenGL3_FrameBuffer();
        OpenGL3_FrameBuffer(const OpenGL3_FrameBuffer& other) = delete;
        OpenGL3_FrameBuffer(OpenGL3_FrameBuffer&& other) = delete;
        OpenGL3_FrameBuffer& operator=(const OpenGL3_FrameBuffer& other) = delete;
        OpenGL3_FrameBuffer& operator=(OpenGL3_FrameBuffer&& other) = delete;

        void bind_as(const GLenum binding_mode = GL_FRAMEBUFFER);
        void unbind(const GLenum binding_mode = GL_FRAMEBUFFER);
        void attach_texture_image_to(const GLenum target_binding_mode, const GLenum target_attachment_point,
                                     const GLenum texture_type, const unsigned int texture_id,
                                     const int mipmap_level = 0) noexcept;
        //  consider overloading by getting a texture, or refactoring in structs

    private:
        unsigned int id;
    };
}

#endif //OPENGL3_FRAMEBUFFER_H
