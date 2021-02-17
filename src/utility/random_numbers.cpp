#include "random_numbers.h"

namespace random_num{
    std::vector<float> uniform_samples_in_unit_interval(const unsigned int number_of_samples) {
        std::random_device seeder;
        std::mt19937 generator(seeder());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        std::vector<float> flattened_points;
        flattened_points.reserve(number_of_samples);
        std::generate_n(std::back_inserter(flattened_points), number_of_samples,
                        [&](){ return distribution(generator); });

        return flattened_points;
    }

    std::vector<glm::vec3> random_polar_offsets(const unsigned int number_of_offsets) {
        const auto random_couples_of_floats = uniform_samples_in_unit_interval(number_of_offsets * 2);
        std::vector<glm::vec3> result;
        result.reserve(number_of_offsets);
        constexpr float two_pi = 2.0f * 3.1415926f;
        for(auto i = 0u; i < number_of_offsets; ++i){
            const float& xi_one = random_couples_of_floats[2*i];
            const float& xi_two = random_couples_of_floats[2*i+1];
            result.emplace_back(glm::vec3( xi_one * std::sin(two_pi * xi_two),
                                           xi_one * std::cos(two_pi * xi_two),
                                           xi_one * xi_one));
        }
        return result;
    }
}

