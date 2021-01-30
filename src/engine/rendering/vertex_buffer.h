#ifndef IES_PARSER_VERTEX_BUFFER_H
#define IES_PARSER_VERTEX_BUFFER_H

#include <fmt/core.h>
#include <fmt/color.h>
#include <glad/glad.h>

#include <string>
#include <vector>

namespace engine {

    enum class ShaderDataType : unsigned int {
        None = 0,
        Bool,
        Float,
        Float2,
        Float3,
        Float4,
        Int,
        Int2,
        Int3,
        Int4,
        Mat3,
        Mat4,
    };

    uint32_t datatype_size(const ShaderDataType type);


    GLenum datatype_to_GLenum_type(const ShaderDataType type);


    class VertexBufferElement{
    public:
        std::string    name;
        ShaderDataType type;
        uint32_t       size;
        uint32_t       offset;
        bool           normalized;

        VertexBufferElement(const ShaderDataType elem_type, std::string elem_name, const bool normalize = false);
        [[nodiscard]] uint32_t component_count() const;
    };

    class VertexBufferLayout{
    public:
        VertexBufferLayout();
        VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elem_list);


        [[nodiscard]] unsigned int                           get_stride() const;
        [[nodiscard]] const std::vector<VertexBufferElement>& get_elements() const;

        [[nodiscard]] std::vector<VertexBufferElement>::iterator       begin();
        [[nodiscard]] std::vector<VertexBufferElement>::iterator       end();
        [[nodiscard]] std::vector<VertexBufferElement>::const_iterator begin() const;
        [[nodiscard]] std::vector<VertexBufferElement>::const_iterator end() const;

    private:
        std::vector<VertexBufferElement> elements;
        unsigned int                   stride;

        void calculate_stride_and_offsets();
    };

    class VertexBuffer {
    public:
        explicit VertexBuffer(const unsigned int data_size, const void* vertex_data);
        VertexBuffer(const unsigned int data_size);

        VertexBuffer(const VertexBuffer& other) = delete;
        VertexBuffer(VertexBuffer&& other) noexcept = delete;
        VertexBuffer& operator=(const VertexBuffer& other) = delete;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept = delete;

        template <typename T>
        explicit VertexBuffer(const std::vector<T>& data);
        ~VertexBuffer();

        void bind() const;
        void unbind() const;

        [[nodiscard]] const VertexBufferLayout& get_buffer_layout() const;
        void set_buffer_layout(const VertexBufferLayout& l);

        void set_data(const unsigned int size, const void* data) const;

    private:
        unsigned int      id;
        VertexBufferLayout layout;
    };
}

#endif //IES_PARSER_VERTEX_BUFFER_H
