#include "layer_container.h"

namespace engine {

    LayerContainer::~LayerContainer() {
        std::for_each(layers.rbegin(), layers.rend(), [](const LayerPtr& layer) { layer->on_detach(); });
    }

    Layer* LayerContainer::push_layer(LayerContainer::LayerPtr&& layer) {
        layer->on_attach();
        layers.emplace(std::begin(layers) + insertion_index, std::move(layer));
        insertion_index++;
        return layers.at(insertion_index-1).get();
    }

    Layer* LayerContainer::push_back_layer(LayerContainer::LayerPtr&& new_top_layer) {
        new_top_layer->on_attach();
        layers.emplace_back(std::move(new_top_layer));
        return layers.back().get();
    }

    void LayerContainer::pop_layer(Layer* layer) {
        const auto find_begin = std::begin(layers);
        const auto find_end = find_begin + insertion_index;
        find_and_remove(find_begin, find_end, layer, true);
    }

    void LayerContainer::pop_back_layer(Layer* layer) {
        const auto find_begin = std::begin(layers) + insertion_index;
        const auto find_end = std::end(layers);
        find_and_remove(find_begin, find_end, layer);
    }

    std::vector<LayerContainer::LayerPtr>::iterator LayerContainer::begin() {
        return layers.begin();
    }

    std::vector<LayerContainer::LayerPtr>::iterator LayerContainer::end() {
        return layers.end();
    }

    std::vector<LayerContainer::LayerPtr>::reverse_iterator LayerContainer::rbegin() {
        return layers.rbegin();
    }

    std::vector<LayerContainer::LayerPtr>::reverse_iterator LayerContainer::rend() {
        return layers.rend();
    }

    std::vector<LayerContainer::LayerPtr>::const_iterator LayerContainer::begin() const {
        return layers.begin();
    }

    std::vector<LayerContainer::LayerPtr>::const_iterator LayerContainer::end() const {
        return layers.end();
    }

    std::vector<LayerContainer::LayerPtr>::const_reverse_iterator LayerContainer::rbegin() const {
        return layers.rbegin();
    }

    std::vector<LayerContainer::LayerPtr>::const_reverse_iterator LayerContainer::rend() const {
        return layers.rend();
    }

    void LayerContainer::find_and_remove(const LayerVec::iterator begin, const LayerVec::iterator end,
                                         const Layer* const to_be_removed, bool decrement_insertion_index) {
        auto find_result = std::find_if(begin, end,
                                        [&to_be_removed](const LayerPtr& layer) {
                                            return layer.get() == to_be_removed;
                                        });
        if (find_result != end) {
            (*find_result)->on_detach();
            layers.erase(find_result);
            if (decrement_insertion_index) {
                insertion_index--;
            }
        }
    }
}