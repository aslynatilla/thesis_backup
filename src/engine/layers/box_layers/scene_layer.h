#ifndef SCENE_LAYER_H
#define SCENE_LAYER_H

#include "../layer.h"

#pragma warning(disable : 4061)
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/material.h>
#pragma warning(error : 4061)

namespace engine{
    class SceneLayer : public Layer{
    public:
        void on_attach() override;
        void on_detach() override;
        void on_event(Event& event) override;
    };
}

#endif //SCENE_LAYER_H
