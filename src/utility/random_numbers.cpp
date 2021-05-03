#include "random_numbers.h"

namespace random_num {
    std::vector<float> uniform_samples_in_unit_interval(const unsigned int number_of_samples) {
        std::random_device seeder;
        std::mt19937 generator(seeder());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        std::vector<float> flattened_points;
        flattened_points.reserve(number_of_samples);
        std::generate_n(std::back_inserter(flattened_points), number_of_samples,
                        [&]() { return distribution(generator); });

        return flattened_points;
    }

    std::vector<float>
    uniform_samples_in_interval(const unsigned int number_of_samples, const float min_value, const float max_value) {
        std::random_device seeder;
        std::mt19937 generator(seeder());
        std::uniform_real_distribution<float> distribution(min_value, max_value);
        std::vector<float> flattened_points;
        flattened_points.reserve(number_of_samples);
        std::generate_n(std::back_inserter(flattened_points), number_of_samples,
                        [&]() { return distribution(generator); });

        return flattened_points;
    }

    std::vector<glm::vec3> random_polar_offsets(const unsigned int number_of_offsets) {
        const auto random_couples_of_floats = uniform_samples_in_unit_interval(number_of_offsets * 2);
        std::vector<glm::vec3> result;
        result.reserve(number_of_offsets);
        constexpr float two_pi = 2.0f * 3.1415926f;
        for (auto i = 0u; i < number_of_offsets; ++i) {
            const float xi_one = random_couples_of_floats[2 * i];
            const float xi_two = random_couples_of_floats[2 * i + 1];
            result.emplace_back(glm::vec3(xi_one * std::sin(two_pi * xi_two),
                                          xi_one * std::cos(two_pi * xi_two),
                                          xi_one * xi_one));
        }
        return result;
    }

    std::vector<glm::vec3> random_directions(const unsigned int number_of_directions) {
        const auto random_triples_of_floats = uniform_samples_in_interval(number_of_directions * 3, -1.0f, +1.0f);
        std::vector<glm::vec3> result;
        result.reserve(number_of_directions);
        for (auto i = 0u; i < number_of_directions; ++i) {
            glm::vec3 new_direction(random_triples_of_floats[3 * i],
                                    random_triples_of_floats[3 * i + 1],
                                    random_triples_of_floats[3 * i + 2]);
            result.emplace_back(glm::normalize(new_direction));
        }
        return result;
    }

    std::vector<glm::vec3> uniform_samples_on_unit_sphere(const unsigned int number_of_samples) {
        const auto random_couples_of_floats = uniform_samples_in_unit_interval(number_of_samples * 2);
        std::vector<glm::vec3> result;
        result.reserve(number_of_samples);
        constexpr float two_pi = 2.0f * 3.1415926f;
        for (auto i = 0u; i < number_of_samples; ++i) {
            const auto& u = random_couples_of_floats[2 * i];
            const auto& v = random_couples_of_floats[2 * i + 1];
            const float theta = two_pi * u;
            const float phi = std::acos(2.0f * v - 1.0f);
            result.emplace_back(glm::vec3(std::cos(theta) * std::sin(phi),
                                          std::sin(theta) * std::sin(phi),
                                          std::cos(phi)));
        }
        return result;
    }
}

