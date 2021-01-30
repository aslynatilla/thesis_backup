#ifndef LAYER_CONTAINER_H
#define LAYER_CONTAINER_H

#include "layer.h"

#include <memory>
#include <vector>

namespace engine {
    class LayerContainer {
    public:
        using LayerPtr = std::unique_ptr<Layer>;
        using LayerVec = std::vector<LayerPtr>;

        LayerContainer() = default;

        ~LayerContainer();

        Layer* push_layer(LayerPtr&& layer);

        Layer* push_back_layer(LayerPtr&& new_top_layer);

        void pop_layer(Layer* layer);

        void pop_back_layer(Layer* layer);

        [[nodiscard]] LayerVec::iterator begin();

        [[nodiscard]] LayerVec::iterator end();

        [[nodiscard]] LayerVec::reverse_iterator rbegin();

        [[nodiscard]] LayerVec::reverse_iterator rend();

        [[nodiscard]] LayerVec::const_iterator begin() const;

        [[nodiscard]] LayerVec::const_iterator end() const;

        [[nodiscard]] LayerVec::const_reverse_iterator rbegin() const;

        [[nodiscard]] LayerVec::const_reverse_iterator rend() const;

    private:

        void find_and_remove(const LayerVec::iterator begin, const LayerVec::iterator end,
                             const Layer* const to_be_removed, bool decrement_insertion_index = false);

        LayerVec layers;
        unsigned int insertion_index = 0;
    };
};


#endif //LAYER_CONTAINER_H
