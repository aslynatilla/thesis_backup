#include "material.h"

namespace engine {
    const void* MaterialData::raw() const {
        return static_cast<const void*>(this);
    }

    std::string Material::default_name = "Unnamed Material";
}