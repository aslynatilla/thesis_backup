#include "ies86_parser_impl.h"

namespace ies::parser_impl {
    bool IES86_ParserImpl::can_handle(const std::string_view& ies_data) {
        // this should be equal to: return true;
        return !ies_data.starts_with("IESNA");
    }

    std::optional<IES_Document> IES86_ParserImpl::handle(const std::string& filename, std::string&& ies_data) {
        using namespace ies::string_view_manipulation;

        const auto lines = data_as_lines(ies_data);
        IES_Standard parsed_standard = IES_Standard::IES_86;

        const auto last_line = lines.cend();
        const auto first_label_line = lines.cbegin();
        const auto tilt_line = find_tilt_line(lines);

        const auto parsed_tilt_value = parse_tilt_line(tilt_line, last_line);
        auto parsed_tilt_data = (parsed_tilt_value == TILT::INCLUDE ? retrieve_tilt_data(tilt_line, last_line)
                                                                    : TILT_Data());

        label_map label_entries;

        //  Special case for double label on first line
        const auto delimiter_pos = first_label_line->find(label_delimiter);
        auto first_label_key = first_label_line->substr(0, delimiter_pos);
        auto first_label_value = first_label_line->substr(delimiter_pos + 1);
        first_label_key = remove_leading_and_trailing_spaces(first_label_key);
        first_label_value = remove_leading_and_trailing_spaces(first_label_value);

        auto DATE_pos = first_label_value.find("DATE:");
        auto[DATE_key, DATE_value] = std::make_pair(first_label_value.substr(DATE_pos, 4),
                                                    first_label_value.substr(DATE_pos + 5));
        first_label_value.remove_suffix(first_label_value.length() - DATE_pos);
        first_label_value = remove_leading_and_trailing_spaces(first_label_value);
        DATE_value = remove_leading_and_trailing_spaces(DATE_value);

        label_entries.emplace(first_label_key, std::vector{first_label_value});
        label_entries.emplace(DATE_key, std::vector{DATE_value});
        //  End special case

        parse_label_entries(first_label_line + 1, tilt_line,
                            std::inserter(label_entries, std::end(label_entries)), label_delimiter,
                            remove_leading_and_trailing_spaces, contains_colon);

        const auto header_line = (parsed_tilt_value == TILT::INCLUDE ? tilt_line + 5 : tilt_line + 1);
        auto header_data = parse_photometric_header(header_line, last_line);
        auto ballast_and_watts = parse_ballast_line(header_line + 1, last_line);

        auto p_type = static_cast<Photometric_Type>(header_data[5]);
        Luminaire_Dimensions l_dim{.measurement_unit_type = static_cast<Length_Unit_Type>(header_data[6]),
                .width = header_data[7],
                .length = header_data[8],
                .height = header_data[9]};

        Luminaire_Data l_data{.dimensions = l_dim,
                .lamps_number = static_cast<std::size_t>(header_data[0]),
                .average_lumens_per_lamp = header_data[1],
                .input_watts = ballast_and_watts[2]};

        Ballast_Data b_data{.ballast_factor = ballast_and_watts[0],
                .ballast_lamp_factor = ballast_and_watts[1]};

        float candela_multiplier = header_data[2];

        const auto vertical_angles_num = static_cast<std::size_t>(header_data[3]);
        const auto horizontal_angles_num = static_cast<std::size_t>(header_data[4]);
        const auto multiplying_factor = candela_multiplier * b_data.ballast_factor * b_data.ballast_lamp_factor;
        auto photometric_angles = parse_photometric_data(header_line + 2, last_line,
                                                         vertical_angles_num, horizontal_angles_num,
                                                         multiplying_factor);

        Photometric_Data p_data{.data_type = p_type,
                .measured_data = photometric_angles,
                .candela_multiplier = candela_multiplier};


        IES_Document result(filename, std::move(ies_data), parsed_standard,
                            IES_Label_Data(std::move(label_entries)), parsed_tilt_value, std::move(parsed_tilt_data),
                            std::move(l_data), std::move(b_data), std::move(p_data));
        return result;
    }

    bool IES86_ParserImpl::contains_colon(const std::string_view& line) {
        return line.find(':') != std::string_view::npos;
    }
}
