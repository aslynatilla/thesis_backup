#ifndef RANDOM_NUMBERS_H
#define RANDOM_NUMBERS_H

#include <glm/glm.hpp>

#include <algorithm>
#include <iterator>
#include <random>
#include <vector>

namespace random_num{
    std::vector<float> uniform_samples_in_unit_interval(const unsigned int number_of_samples);
    std::vector<float> uniform_samples_in_interval(const unsigned int number_of_samples, const float min_value, const float max_value);
    std::vector <glm::vec3> random_polar_offsets(const unsigned int number_of_offsets);
    std::vector<glm::vec3> random_directions(const unsigned int number_of_directions);
    std::vector<glm::vec3> uniform_samples_on_unit_sphere(const unsigned int number_of_samples);
};

#endif //RANDOM_NUMBERS_H
