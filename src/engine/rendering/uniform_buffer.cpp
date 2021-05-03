#include "uniform_buffer.h"

namespace engine {
    UniformBuffer::UniformBuffer(const int buffer_size, const void* buffer_initial_data) {
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, buffer_size, buffer_initial_data, GL_STATIC_DRAW);
        size = buffer_size;
    }

    UniformBuffer::UniformBuffer(const int buffer_size) {
        glGenBuffers(1, &id);
        glBindBuffer(GL_UNIFORM_BUFFER, id);
        glBufferData(GL_UNIFORM_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);
        size = buffer_size;
    }

    UniformBuffer::~UniformBuffer() {
        glDeleteBuffers(1, &id);
    }

    void UniformBuffer::bind_to_binding_point(const unsigned int binding_point) {
        glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, id);
    }

    void UniformBuffer::bind_to_uniform_buffer_target() {
        glBindBuffer(GL_UNIFORM_BUFFER, id);
    }

    void UniformBuffer::unbind_from_uniform_buffer_target() {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void UniformBuffer::copy_to_buffer(const int starting_offset, const int copy_size,
                                       const void* data_source) {
        glBufferSubData(GL_UNIFORM_BUFFER, starting_offset, copy_size, data_source);
    }
}
