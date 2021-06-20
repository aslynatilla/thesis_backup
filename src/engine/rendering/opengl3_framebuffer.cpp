#include "opengl3_framebuffer.h"

namespace engine{

    OpenGL3_FrameBuffer::OpenGL3_FrameBuffer() {
        glGenFramebuffers(1, &id);
    }

    OpenGL3_FrameBuffer::~OpenGL3_FrameBuffer() {
        glDeleteFramebuffers(1, &id);
    }

    void OpenGL3_FrameBuffer::bind_as(const GLenum framebuffer_mode) {
        glBindFramebuffer(framebuffer_mode, id);
    }

    void OpenGL3_FrameBuffer::unbind_from(const GLenum framebuffer_mode) {
        glBindFramebuffer(framebuffer_mode, 0);
    }

    /// \param framebuffer_mode  one of GL_FRAMEBUFFER, GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER
    /// \param attachment_point     probably GL_COLOR_ATTACHMENTi or GL_DEPTH_ATTACHMENT
    /// \param texture_type         what type of texture is expected in the texture parameter
    ///                             or, for cube map textures, which face is to be attached
    bool OpenGL3_FrameBuffer::texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                                          const GLenum texture_type, const unsigned int texture_id,
                                                          const int mipmap_level) noexcept{
        glFramebufferTexture2D(framebuffer_mode, target_attachment_point, texture_type, texture_id, mipmap_level);
        return (glGetError() == 0);
    }

    bool OpenGL3_FrameBuffer::texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                                          const OpenGL3_Texture2D& texture, const int mipmap_level) noexcept {
        glFramebufferTexture2D(framebuffer_mode, target_attachment_point, GL_TEXTURE_2D, texture.id(), mipmap_level);
        return (glGetError() == 0);
    }

    bool OpenGL3_FrameBuffer::texture_to_attachment_point(const GLenum framebuffer_mode, const GLenum target_attachment_point,
                                                          const OpenGL3_Cubemap& cubemap, const int mipmap_level) noexcept {
        glFramebufferTexture(framebuffer_mode, target_attachment_point, cubemap.id(), mipmap_level);
        return (glGetError() == 0);
    }
}