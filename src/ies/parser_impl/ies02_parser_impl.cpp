#include "ies02_parser_impl.h"

namespace ies::parser_impl {
    bool IES02_ParserImpl::can_handle(const std::string_view& ies_data) {
        return ies_data.starts_with("IESNA:LM-63-2002");
    }

    std::optional<IES_Document>
    IES02_ParserImpl::handle(const std::string& filename, std::string&& ies_data) {
        using namespace ies::string_view_manipulation;

        const auto lines = data_as_lines(ies_data);
        IES_Standard parsed_standard = IES_Standard::IES_02;

        const auto last_line = lines.cend();
        const auto first_label_line = lines.cbegin() + 1;
        const auto tilt_line = find_tilt_line(lines);

        label_map label_entries;
        parse_label_entries(first_label_line, tilt_line,
                            std::inserter(label_entries, std::end(label_entries)), label_delimiter,
                            remove_spaces_brackets_and_keyword, no_more_entry_lines);

        label_entries.erase("BLOCK");
        label_entries.erase("ENDBLOCK");

        const auto parsed_tilt_value = parse_tilt_line(tilt_line, last_line);
        auto parsed_tilt_data = (parsed_tilt_value == TILT::INCLUDE ? retrieve_tilt_data(tilt_line, last_line)
                                                                    : TILT_Data());

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
                            std::move(l_data), std::move(b_data), std
                            ::move(p_data));
        return result;
    }

    bool IES02_ParserImpl::no_more_entry_lines(const std::string_view& line) {
        //  If you cannot find [MORE] in this line AND there is a delimiter,
        //  this line contains a new label
        return (line.find("[MORE]") == std::string_view::npos) &&
               (line.find("]") != std::string_view::npos);
    }
}