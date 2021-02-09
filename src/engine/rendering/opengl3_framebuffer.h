#ifndef OPENGL3_FRAMEBUFFER_H
#define OPENGL3_FRAMEBUFFER_H

#include "opengl3_cubemap.h"
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

        void bind_as(const GLenum framebuffer_mode = GL_FRAMEBUFFER);
        void unbind_from(const GLenum framebuffer_mode = GL_FRAMEBUFFER);
        bool texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                         const GLenum texture_type, const unsigned int texture_id,
                                         const int mipmap_level = 0) noexcept;
        bool texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                         const OpenGL3_Texture& texture, const int mipmap_level = 0) noexcept;
        bool texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                         const OpenGL3_Cubemap& cubemap, const int mipmap_level = 0) noexcept;
        unsigned int id;
    };
}

#endif //OPENGL3_FRAMEBUFFER_H
