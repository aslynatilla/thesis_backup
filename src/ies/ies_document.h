#ifndef IES_DOCUMENT_H
#define IES_DOCUMENT_H

#include <array>
#include <string>
#include <string_view>

#include "ies_data.h"

namespace ies {
    class IES_Document {
    public:
        std::string filename;
        std::string raw_data;
        IES_Standard standard;
        IES_Label_Data label_entries;
        TILT tilt_value;
        TILT_Data tilt_description;
        Luminaire_Data luminaire_description;
        Ballast_Data ballast_description;
        Photometric_Data photometric_description;

        IES_Document() = default;

        explicit IES_Document(std::string name, std::string&& data, IES_Standard document_standard,
                              IES_Label_Data&& parsed_labels, TILT parsed_tilt, TILT_Data&& parsed_tilt_info,
                              Luminaire_Data&& luminaire_info, Ballast_Data&& ballast_info,
                              Photometric_Data&& photometric_info);

        IES_Document(IES_Document&& other) = default;

        IES_Document& operator=(IES_Document&& other) = default;

        friend std::ostream& operator<<(std::ostream& os, const IES_Document& document);
    };
}

#endif //IES_DOCUMENT_H
