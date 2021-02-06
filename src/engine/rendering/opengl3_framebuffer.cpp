#include "opengl3_framebuffer.h"

namespace engine{

    OpenGL3_FrameBuffer::OpenGL3_FrameBuffer() {
        glGenFramebuffers(1, &id);
    }

    OpenGL3_FrameBuffer::~OpenGL3_FrameBuffer() {
        glDeleteFramebuffers(1, &id);
    }

    void OpenGL3_FrameBuffer::bind_as(const GLenum binding_mode) {
        glBindFramebuffer(binding_mode, id);
    }

    void OpenGL3_FrameBuffer::unbind(const GLenum binding_mode) {
        glBindFramebuffer(binding_mode, 0);
    }

    /// \param target_binding_mode  one of GL_FRAMEBUFFER, GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER
    /// \param attachment_point     probably GL_COLOR_ATTACHMENTi or GL_DEPTH_ATTACHMENT
    /// \param texture_type         what type of texture is expected in the texture parameter
    ///                             or, for cube map textures, which face is to be attached
    void OpenGL3_FrameBuffer::attach_texture_image_to(const GLenum target_binding_mode, const GLenum target_attachment_point,
                                                      const GLenum texture_type, const unsigned int texture_id,
                                                      const int mipmap_level) noexcept{
        glBindFramebuffer(target_binding_mode, id);
        glFramebufferTexture2D(target_binding_mode, target_attachment_point, texture_type, texture_id, mipmap_level);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
}