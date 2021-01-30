#ifndef ELEMENT_BUFFER_H
#define ELEMENT_BUFFER_H

#include <glad/glad.h>

namespace engine {
    class ElementBuffer {
    public:
        explicit ElementBuffer(const unsigned int* indices, const unsigned int count)
                : id{0}, size{count} {
            glGenBuffers(1, &id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size * sizeof(unsigned int), indices, GL_STATIC_DRAW);
        }

        explicit ElementBuffer(const std::vector <unsigned int>& indices)
                : ElementBuffer(indices.data(),
                                indices.size()) {};

        explicit ElementBuffer(const std::initializer_list <unsigned int>& indices)
                : ElementBuffer(std::data(indices),
                                indices.size()) {};


        ~ElementBuffer() { glDeleteBuffers(1, &id); }

        void bind() const {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
        }

        void unbind() const {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        unsigned int get_count() const {
            return size;
        }

    private:
        unsigned int id;
        unsigned int size;
    };
}

#endif //ELEMENT_BUFFER_H
