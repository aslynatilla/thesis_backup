#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#include "../../rendering/camera.h"
#include "../../rendering/shader.h"
#include "../../rendering/renderer.h"
#include "../../../utility/file_reader.h"
#include "../../rendering/vertex_array.h"
#include <glm/glm.hpp>

#pragma warning(disable : 4061)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#pragma warning(error : 4061)

namespace engine{
    struct ModelData{
        VertexArray vao;
        glm::mat4 transform;

        void set_transform_matrix(aiMatrix4x4 model_matrix);
    };

    class SceneLayer : public Layer{
    public:
        void on_attach() final;
        void on_detach() final ;
        void on_event(Event& event) final;
        virtual void update(float delta_time) final;
        virtual void on_imgui_render() final;

    private:
        Camera view_camera;
        std::shared_ptr<Shader> shader;
        std::vector<std::unique_ptr<ModelData>> models;
        glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

        void load_from_scene(const aiScene* scene);
    };
}

#endif //SCENE_LAYER_H
