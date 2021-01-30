#include "common_ies_parser_impl.h"

namespace ies::parser_impl {

    template<typename T>
    auto parse_element(const std::vector<std::string_view>::const_iterator& line) -> T {
        std::string line_as_string = std::string(*line);
        std::stringstream stringifier(line_as_string);
        T element;
        stringifier >> element;
        return element;
    }

    template<typename T>
    auto parse_elements(const size_t N, const std::vector<std::string_view>::const_iterator& line,
                        const std::vector<std::string_view>::const_iterator& end_of_document) -> std::vector<T> {
        std::vector<T> elements;
        elements.reserve(N);

        auto current_line = line;
        auto current_line_as_string = std::string(*current_line);
        std::stringstream stringifier(current_line_as_string);

        while (elements.size() < N) {
            auto begin = std::istream_iterator<T>(stringifier);
            std::copy_n(begin, 1, std::back_inserter(elements));
            if (stringifier.eof() && (current_line + 1) != end_of_document) {
                std::advance(current_line, 1);
                current_line_as_string = std::string(*current_line);
                stringifier = std::stringstream(current_line_as_string);
            }
        }

        return elements;
    }

    std::vector<std::string_view> delimited_views_on(const std::string_view& view_on_data, const char& delimiter) {
        const auto end = std::cend(view_on_data);

        std::vector<std::string_view> delimited_views;
        auto search_point = std::cbegin(view_on_data);
        while (search_point != end) {
            const auto next_newline = std::find(search_point, end, delimiter);
            delimited_views.emplace_back(std::string_view(search_point, next_newline));
            next_newline != end ? search_point = std::next(next_newline) : search_point = end;
        }

        return delimited_views;
    }

    std::vector<std::string_view> data_as_lines(const std::string& read_data) {
        return delimited_views_on(std::string_view(read_data), '\n');
    }

    std::vector<std::string_view>::const_iterator find_tilt_line(const std::vector<std::string_view>& lines) {
        return std::find_if(lines.cbegin(), lines.cend(),
                            [](const std::string_view& line) { return line.starts_with("TILT"); });
    }

    TILT parse_tilt_line(const std::vector<std::string_view>::const_iterator& tilt_line,
                         const std::vector<std::string_view>::const_iterator& last_line) {
        if (tilt_line == last_line) {
            return TILT::NONE;
        }
        const auto equal_include_index = tilt_line->find("=INCLUDE");

        if (equal_include_index != tilt_line->npos) {
            return TILT::INCLUDE;
        } else {
            return TILT::NONE;
        }
    }

    TILT_Data retrieve_tilt_data(const std::vector<std::string_view>::const_iterator& tilt_line,
                                 const std::vector<std::string_view>::const_iterator& last_line) {
        auto tilt_orientation = static_cast<TILT_Orientation>(parse_element<size_t>(tilt_line + 1));
        auto tilt_angles_number = parse_element<std::size_t>(tilt_line + 2);
        auto tilt_angles = parse_elements<float>(tilt_angles_number, tilt_line + 3, last_line);
        auto mult_factors = parse_elements<float>(tilt_angles_number, tilt_line + 4, last_line);

        return TILT_Data{.orientation = tilt_orientation,
                .angles_number = tilt_angles_number,
                .angles = std::move(tilt_angles),
                .per_angle_candela_multipliers = std::move(mult_factors)};
    }


    template<typename TrimmingFunction, typename StopParsingPredicate>
    requires (std::invocable<TrimmingFunction, std::string_view>
              && std::invocable<StopParsingPredicate, const std::string_view&>)
    void parse_label_entries(const std::vector<std::string_view>::const_iterator& begin_line,
                             const std::vector<std::string_view>::const_iterator& end_line,
                             std::insert_iterator<label_map> inserter,
                             const char delimiter,
                             TrimmingFunction trimmer,
                             StopParsingPredicate stop_condition) {

        auto current_line = begin_line;
        while (current_line != end_line) {
            const auto delimiter_pos = current_line->find(delimiter);
            auto label_key = current_line->substr(0, delimiter_pos);
            auto first_label_value = current_line->substr(delimiter_pos + 1);
            label_key = trimmer(label_key);

            label_entry values{trimmer(first_label_value)};

            current_line = std::next(current_line);
            auto next_label_line = std::find_if(current_line, end_line, stop_condition);
            if (current_line != next_label_line) {
                std::transform(current_line, next_label_line, std::back_inserter(values), trimmer);
                std::advance(current_line, std::distance(current_line, next_label_line));
            }

            inserter = label_pair(label_key, values);
        }
    }

    std::vector<float> parse_photometric_header(const std::vector<std::string_view>::const_iterator& header_line,
                                                const std::vector<std::string_view>::const_iterator& end_line) {
        return parse_elements<float>(10, header_line, end_line);
    }

    std::vector<float> parse_ballast_line(const std::vector<std::string_view>::const_iterator& ballast_line,
                                          const std::vector<std::string_view>::const_iterator& end_line) {
        return parse_elements<float>(3, ballast_line, end_line);
    }

    Photometric_Angles parse_photometric_data(const std::vector<std::string_view>::const_iterator& angles_line,
                                              const std::vector<std::string_view>::const_iterator& end_line,
                                              const std::size_t vertical_angles,
                                              const std::size_t horizontal_angles,
                                              const float multiplying_factor) {
        const std::size_t angles_list_plus_candelas = vertical_angles + horizontal_angles +
                                                      (vertical_angles * horizontal_angles);

        auto parsed_data = parse_elements<float>(angles_list_plus_candelas, angles_line, end_line);

        const auto vertical_angles_list_begin = std::begin(parsed_data);
        const auto horizontal_angles_list_begin = vertical_angles_list_begin + vertical_angles;
        const auto candela_list_begin = horizontal_angles_list_begin + horizontal_angles;
        const auto end_of_data = std::end(parsed_data);

        std::transform(candela_list_begin, end_of_data,
                       candela_list_begin,
                       [&multiplying_factor](auto& candela_value) { return multiplying_factor * candela_value; });

        return Photometric_Angles{
                .vertical_angles = std::vector(vertical_angles_list_begin, horizontal_angles_list_begin),
                .horizontal_angles = std::vector(horizontal_angles_list_begin, candela_list_begin),
                .candelas_per_angle_pair = std::vector(candela_list_begin, end_of_data)};
    }

    //  Explicit instantiation definition for parse_label_entries
    template void parse_label_entries(const std::vector<std::string_view>::const_iterator& begin_line,
                                      const std::vector<std::string_view>::const_iterator& end_line,
                                      std::insert_iterator<label_map> inserter,
                                      const char delimiter,
                                      std::string_view (*trimmer)(std::string_view),
                                      bool (*stop_condition)(const std::string_view&));

}