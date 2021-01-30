#ifndef IES_DATA_H
#define IES_DATA_H

#include <iomanip>

#include "ies_label_data.h"
#include "ies_printing.h"

namespace ies {

    enum struct IES_Standard : size_t {
        IES_86 = 3,
        IES_91 = 2,
        IES_95 = 1,
        IES_02 = 0
    };

    std::ostream& operator<<(std::ostream& os, IES_Standard standard);

    enum struct TILT : bool {
        NONE = false,
        INCLUDE = true
    };

    std::ostream& operator<<(std::ostream& os, TILT t);

    enum struct TILT_Orientation : size_t {
        Vertical = 1,
        Always_Horizontal = 2,
        No_Rotate_Horizontal = 3
    };

    std::ostream& operator<<(std::ostream& os, TILT_Orientation orientation);

    enum struct Length_Unit_Type : size_t {
        Feet = 1,
        Meters = 2
    };

    std::ostream& operator<<(std::ostream& os, Length_Unit_Type unit_type);

    enum struct Photometric_Type : size_t {
        Type_C = 1,
        Type_B = 2,
        Type_A = 3
    };

    std::ostream& operator<<(std::ostream& os, Photometric_Type photo_type);

    struct TILT_Data {
        TILT_Orientation orientation;
        std::size_t angles_number;
        std::vector<float> angles;
        std::vector<float> per_angle_candela_multipliers;

        friend std::ostream& operator<<(std::ostream& os, const TILT_Data& data);
    };

    struct Luminaire_Dimensions {
        Length_Unit_Type measurement_unit_type;
        float width;
        float length;
        float height;

        std::string to_string() const;

        friend std::ostream& operator<<(std::ostream& os, const Luminaire_Dimensions& dim);
    };

    struct Luminaire_Data {
        Luminaire_Dimensions dimensions;
        std::size_t lamps_number;
        float average_lumens_per_lamp;
        float input_watts;

        std::string to_string() const;

        friend std::ostream& operator<<(std::ostream& os, const Luminaire_Data& data);
    };

    struct Photometric_Angles {
        std::vector<float> vertical_angles;
        std::vector<float> horizontal_angles;
        std::vector<float> candelas_per_angle_pair;

        friend std::ostream& operator<<(std::ostream& os, const Photometric_Angles& measures);
    };

    struct Photometric_Data {
        Photometric_Type data_type;
        Photometric_Angles measured_data;
        float candela_multiplier;

        std::string to_string() const;

        friend std::ostream& operator<<(std::ostream& os, const Photometric_Data& data);
    };

    struct Ballast_Data {
        float ballast_factor;
        float ballast_lamp_factor;

        std::string to_string() const;

        friend std::ostream& operator<<(std::ostream& os, const Ballast_Data& data);
    };
}

#endif //IES_DATA_H
