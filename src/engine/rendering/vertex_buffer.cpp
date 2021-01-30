#include "vertex_buffer.h"

namespace engine {
    unsigned int datatype_size(const ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Bool:
                return 1;
            case ShaderDataType::Float:
                return 4;
            case ShaderDataType::Float2:
                return 4 * 2;
            case ShaderDataType::Float3:
                return 4 * 3;
            case ShaderDataType::Float4:
                return 4 * 4;
            case ShaderDataType::Int:
                return 4;
            case ShaderDataType::Int2:
                return 4 * 2;
            case ShaderDataType::Int3:
                return 4 * 3;
            case ShaderDataType::Int4:
                return 4 * 4;
            case ShaderDataType::Mat3:
                return 4 * 3 * 3;
            case ShaderDataType::Mat4:
                return 4 * 4 * 4;
            case ShaderDataType::None:
                fmt::print("[VERTEX BUFFER] ShaderDataType is not recognized and size was not found.");
        }
        return 0;
    }

    GLenum datatype_to_GLenum_type(const ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Bool:
                return GL_BOOL;
            case ShaderDataType::Float:
                return GL_FLOAT;
            case ShaderDataType::Float2:
                return GL_FLOAT;
            case ShaderDataType::Float3:
                return GL_FLOAT;
            case ShaderDataType::Float4:
                return GL_FLOAT;
            case ShaderDataType::Int:
                return GL_INT;
            case ShaderDataType::Int2:
                return GL_INT;
            case ShaderDataType::Int3:
                return GL_INT;
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Mat3:
                return GL_FLOAT;
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::None:
                fmt::print("[VERTEX BUFFER] Unknown ShaderDataType, it could not be converted to GLenum type.");
        }
        return 0;
    }

    // Vertex Buffer Element implementation

    VertexBufferElement::VertexBufferElement(const ShaderDataType elem_type, std::string elem_name,
                                             const bool normalize)
            : name{std::move(elem_name)}, type{elem_type},
              size{datatype_size(type)}, offset{0},
              normalized{normalize} {};

    unsigned int VertexBufferElement::component_count() const {
        switch (type) {
            case ShaderDataType::Bool:
                return 1;
            case ShaderDataType::Float:
                return 1;
            case ShaderDataType::Float2:
                return 2;
            case ShaderDataType::Float3:
                return 3;
            case ShaderDataType::Float4:
                return 4;
            case ShaderDataType::Int:
                return 1;
            case ShaderDataType::Int2:
                return 2;
            case ShaderDataType::Int3:
                return 3;
            case ShaderDataType::Int4:
                return 4;
            case ShaderDataType::Mat3:
                return 3; // to be intended as 3 * float3
            case ShaderDataType::Mat4:
                return 4; // to be intended as 4 * float4
            case ShaderDataType::None:
                fmt::print("[VERTEX BUFFER] No components for this ShaderDataType; irregular type!");
        }
        return 0;
    }

    // Vertex Buffer Layout implementation

    VertexBufferLayout::VertexBufferLayout() : stride { 0 } {};


    VertexBufferLayout::VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elem_list) :
    elements { elem_list }, stride{0} {
        calculate_stride_and_offsets();
    }

    unsigned int VertexBufferLayout::get_stride() const{ return stride; }

    const std::vector<VertexBufferElement>& VertexBufferLayout::get_elements() const { return elements; }

    std::vector<VertexBufferElement>::iterator VertexBufferLayout::begin() {
        return elements.begin();
    }

    std::vector<VertexBufferElement>::iterator VertexBufferLayout::end() {
        return elements.end();
    }

    std::vector<VertexBufferElement>::const_iterator VertexBufferLayout::begin() const {
        return elements.cbegin();
    }

    std::vector<VertexBufferElement>::const_iterator VertexBufferLayout::end() const {
        return elements.cend();
    }

    void VertexBufferLayout::calculate_stride_and_offsets() {
        unsigned int offset = 0;
        stride = 0;
        for(auto& e : elements) {
            e.offset = offset;
            offset += e.size;
            stride += e.size;
        }
    }

    // Vertex Buffer Object implementation

    VertexBuffer::VertexBuffer(const unsigned int data_size, const void* vertex_data) {
        glGenBuffers(1, &id);
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferData(GL_ARRAY_BUFFER, data_size, vertex_data, GL_STATIC_DRAW);
    }

    VertexBuffer::VertexBuffer(const unsigned int data_size)
    : VertexBuffer(data_size, nullptr) {};

    template<typename T>
    VertexBuffer::VertexBuffer(const std::vector<T>& data)
    : VertexBuffer(data.size() * sizeof(T), data.data()) {};

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &id);
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void VertexBuffer::unbind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    const VertexBufferLayout& VertexBuffer::get_buffer_layout() const {
        return layout;
    }

    void VertexBuffer::set_buffer_layout(const VertexBufferLayout& l) {
        layout = l;
    }

    //   Using glBufferSubDdata instead of glBufferData as docs.gl recommends.
    //  "When replacing the entire data store, consider using glBufferSubData
    //  rather than completely recreating the data store with glBufferData."
    void VertexBuffer::set_data(const unsigned int size, const void* data) const {
        glBindBuffer(GL_ARRAY_BUFFER, id);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }
}