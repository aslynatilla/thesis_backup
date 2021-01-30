#include "ies_document.h"

namespace ies {

    IES_Document::IES_Document(std::string name, std::string&& data,
                               IES_Standard document_standard, IES_Label_Data&& parsed_labels,
                               TILT parsed_tilt, TILT_Data&& parsed_tilt_info, Luminaire_Data&& luminaire_info,
                               Ballast_Data&& ballast_info, Photometric_Data&& photometric_info) :
            filename(std::move(name)),
            raw_data(std::move(data)),
            standard(document_standard),
            label_entries(parsed_labels),
            tilt_value(parsed_tilt),
            tilt_description(parsed_tilt_info),
            luminaire_description(luminaire_info),
            ballast_description(ballast_info),
            photometric_description(photometric_info) {}

    std::ostream& operator<<(std::ostream& os, const IES_Document& document) {
        os << "File name: " << document.filename << "\n";
        os << "IES Standard: " << document.standard << "\n";
        os << "Labels:\n" << document.label_entries << "\n";
        if (document.tilt_value == TILT::INCLUDE) {
            os << "TILT description\n" << document.tilt_description << "\n";
        }
        os << document.luminaire_description << "\n";
        os << document.ballast_description << "\n";
        os << document.photometric_description << "\n";
        return os;
    }
}

