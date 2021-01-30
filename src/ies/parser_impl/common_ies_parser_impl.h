#ifndef REWORKED_PARSER_COMMON_IES_PARSER_IMPL_H
#define REWORKED_PARSER_COMMON_IES_PARSER_IMPL_H

#include <concepts>
#include <optional>
#include <sstream>

#include "../ies_document.h"

namespace ies::parser_impl {
    std::vector<std::string_view> delimited_views_on(const std::string_view& view_on_data, const char& delimiter);

    std::vector<std::string_view> data_as_lines(const std::string& read_data);

    [[maybe_unused]] std::vector<std::string_view> data_as_lines(const std::string&& read_data) = delete;

    template<typename T>
    auto parse_element(const std::vector<std::string_view>::const_iterator& line) -> T;

    template<typename T>
    auto parse_elements(const std::size_t N, const std::vector<std::string_view>::const_iterator& line,
                        const std::vector<std::string_view>::const_iterator& end_of_document) -> std::vector<T>;

    std::vector<std::string_view>::const_iterator
    find_tilt_line(const std::vector<std::string_view>& lines);

    TILT parse_tilt_line(const std::vector<std::string_view>::const_iterator& tilt_line,
                         const std::vector<std::string_view>::const_iterator& last_line);

    TILT_Data retrieve_tilt_data(const std::vector<std::string_view>::const_iterator& tilt_line,
                                 const std::vector<std::string_view>::const_iterator& last_line);

    template<typename TrimmingFunction, typename StopParsingPredicate>
    requires (std::invocable<TrimmingFunction, std::string_view>
              && std::invocable<StopParsingPredicate, const std::string_view&>)
    void parse_label_entries(const std::vector<std::string_view>::const_iterator& begin_line,
                             const std::vector<std::string_view>::const_iterator& end_line,
                             std::insert_iterator<label_map> inserter,
                             const char delimiter,
                             TrimmingFunction trimmer,
                             StopParsingPredicate stop_condition);

    std::vector<float> parse_photometric_header(const std::vector<std::string_view>::const_iterator& header_line,
                                                const std::vector<std::string_view>::const_iterator& end_line);

    std::vector<float> parse_ballast_line(const std::vector<std::string_view>::const_iterator& ballast_line,
                                          const std::vector<std::string_view>::const_iterator& end_line);

    Photometric_Angles parse_photometric_data(const std::vector<std::string_view>::const_iterator& angles_line,
                                              const std::vector<std::string_view>::const_iterator& end_line,
                                              const std::size_t vertical_angles,
                                              const std::size_t horizontal_angles,
                                              const float multiplying_factor);

    //  Explicit template declaration for parse_label_entries
    extern template void parse_label_entries(const std::vector<std::string_view>::const_iterator&,
                                             const std::vector<std::string_view>::const_iterator&,
                                             std::insert_iterator<label_map>,
                                             const char,
                                             std::string_view (*)(std::string_view),
                                             bool (*)(const std::string_view&));

}

#endif //REWORKED_PARSER_COMMON_IES_PARSER_IMPL_H