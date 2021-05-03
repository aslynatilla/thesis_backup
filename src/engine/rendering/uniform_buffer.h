#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "vertex_buffer.h"

namespace engine{
    class UniformBuffer{
    public:
        explicit UniformBuffer(const int buffer_size, const void* buffer_initial_data);
        explicit UniformBuffer(const int buffer_size);
        UniformBuffer(const UniformBuffer& other) = delete;
        UniformBuffer(UniformBuffer&& other) = delete;
        UniformBuffer& operator=(const UniformBuffer& other) = delete;
        UniformBuffer& operator=(UniformBuffer&& other) = delete;
        ~UniformBuffer();

        void bind_to_binding_point(const unsigned int binding_point);
        void bind_to_uniform_buffer_target();
        void unbind_from_uniform_buffer_target();

        void copy_to_buffer(const int starting_offset, const int copy_size, const void* data_source);


    private:
        unsigned int id;
        unsigned int size;
    };
}



#endif //UNIFORM_BUFFER_H
