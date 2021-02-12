#ifndef RANDOM_NUMBERS_H
#define RANDOM_NUMBERS_H

#include <glm/glm.hpp>

#include <algorithm>
#include <iterator>
#include <random>
#include <vector>

namespace random_num{
    std::vector<float> uniform_samples_in_unit_interval(const unsigned int number_of_samples);
    std::vector <glm::vec3> random_polar_offsets(const unsigned int number_of_offsets);
};

#endif //RANDOM_NUMBERS_H
