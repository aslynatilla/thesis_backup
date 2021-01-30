#ifndef LABEL_DATA_H
#define LABEL_DATA_H

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "fmt/ostream.h"

namespace ies {
    using label = std::string_view;
    using label_entry = std::vector<std::string_view>;
    using label_map = std::unordered_map<label, label_entry>;
    using label_pair = std::pair<label, label_entry>;

    class IES_Label_Data {
    public:
        IES_Label_Data() = default;

        explicit IES_Label_Data(label_map&& data);

        [[nodiscard]]
        std::vector<label> keys() const;

        [[nodiscard]]
        bool contains(const std::string& searched_key) const noexcept;

        [[nodiscard]]
        bool contains(const char *searched_key) const noexcept;

        [[nodiscard]]
        label_entry lines_at(const std::string_view key) const;

        [[nodiscard]]
        std::string at(const std::string_view key) const;

        friend std::ostream& operator<<(std::ostream& os, const IES_Label_Data& labels);

        std::string to_string() const;

    private:
        label_map label_entries;
    };
}


#endif //IES_LABEL_DATA_H
