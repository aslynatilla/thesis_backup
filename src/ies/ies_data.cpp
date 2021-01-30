#include "ies_data.h"

namespace ies {
    std::ostream& operator<<(std::ostream& os, IES_Standard standard) {
        switch (standard) {
            case IES_Standard::IES_02:
                os << "IES02";
                break;
            case IES_Standard::IES_95:
                os << "IES95";
                break;
            case IES_Standard::IES_91:
                os << "IES91";
                break;
            case IES_Standard::IES_86:
                os << "IES86";
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, TILT t) {
        os << (t == TILT::INCLUDE ? "TILT=INCLUDE" : "TILT=NONE");
        return os;
    }

    std::ostream& operator<<(std::ostream& os, TILT_Orientation orientation) {
        switch (orientation) {
            case TILT_Orientation::Vertical:
                os << "Vertical luminaire";
                break;
            case TILT_Orientation::Always_Horizontal:
                os << "Always horizontal luminaire";
                break;
            case TILT_Orientation::No_Rotate_Horizontal:
                os << "No rotation horizontal luminaire";
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Length_Unit_Type unit_type) {
        os << (unit_type == Length_Unit_Type::Feet ? "feet" : "meters");
        return os;
    }

    std::ostream& operator<<(std::ostream& os, Photometric_Type photo_type) {
        switch (photo_type) {
            case Photometric_Type::Type_C:
                os << "C";
                break;
            case Photometric_Type::Type_B:
                os << "B";
                break;
            case Photometric_Type::Type_A:
                os << "A";
                break;
        }
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const TILT_Data& data) {
        using namespace printing;
        os << data.orientation;
        os << "\nTILT angles:\n";
        print_container_spaced(data.angles, os, 5);
        os << "\nCandela multipliers per angle:\n";
        print_container_spaced(data.per_angle_candela_multipliers, os, 5);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Luminaire_Dimensions& dim) {
        fmt::print(os,
                   "Dimensions (width, length, height):\n"
                   "{1} {0}, {2} {0}, {3} {0}",
                   dim.measurement_unit_type, dim.width, dim.length, dim.height);
        return os;
    }

    std::string Luminaire_Dimensions::to_string() const {
        return fmt::format("{}", *this);
    }

    std::string Luminaire_Data::to_string() const {
        return fmt::format("{}", *this);
    }

    std::ostream& operator<<(std::ostream& os, const Luminaire_Data& data) {
        fmt::print(os,
                   "Luminaire description\n{0}\n"
                   "Number of lamps:\n{1}\n"
                   "Input watts:\n{2}\n"
                   "Average lumens per lamp:\n{3}",
                   data.dimensions.to_string(), data.lamps_number, data.input_watts, data.average_lumens_per_lamp);
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Photometric_Angles& measures) {
        using namespace printing;
        os << "Vertical angles:\n";
        print_container_spaced(measures.vertical_angles, os, 5);
        os << "\nHorizontal angles:\n";
        print_container_spaced(measures.horizontal_angles, os, 5);
        os << "\nCandela per angle pair:\n";
        print_container_spaced_multiline(measures.candelas_per_angle_pair, os,
                                         static_cast<int>(measures.vertical_angles.size()), 7);
        return os;
    }

    std::string Photometric_Data::to_string() const {
        return fmt::format("{}", *this);
    }

    std::ostream& operator<<(std::ostream& os, const Photometric_Data& data) {
        fmt::print(os,
                   "Photometric Type:\n{0}\n"
                   "Measured angles and corresponding candela values:\n{1}\n"
                   "Candela multiplier:\n{2}",
                   data.data_type, data.measured_data, data.candela_multiplier);
        return os;
    }

    std::string Ballast_Data::to_string() const {
        return fmt::format("{}", *this);
    }

    std::ostream& operator<<(std::ostream& os, const Ballast_Data& data) {
        fmt::print(os,
                   "Ballast factor:\n{0}\n"
                   "Ballast lamp factor:\n{1}",
                   data.ballast_factor, data.ballast_lamp_factor);
        return os;
    }
}
