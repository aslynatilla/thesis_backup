#include "ies_label_data.h"

namespace ies {
    IES_Label_Data::IES_Label_Data(label_map&& data) : label_entries(data) {}

    std::vector<std::string_view> IES_Label_Data::keys() const {
        auto key_extraction = [](const label_pair& pair) -> label {
            return pair.first;
        };

        std::vector<label> keys;
        keys.reserve(label_entries.size());
        std::transform(std::begin(label_entries), std::end(label_entries), std::back_inserter(keys), key_extraction);
        return keys;
    }

    bool IES_Label_Data::contains(const std::string& searched_key) const noexcept {
        return label_entries.contains(searched_key);
    }

    bool IES_Label_Data::contains(const char *searched_key) const noexcept {
        return label_entries.contains(searched_key);
    }

    label_entry IES_Label_Data::lines_at(const std::string_view key) const {
        return label_entries.at(key);
    }

    std::string IES_Label_Data::at(const std::string_view key) const {
        const auto entries = label_entries.at(key);
        std::stringstream string_builder;
        string_builder << std::string(entries.front());
        if (entries.size() > 1) {
            std::for_each(entries.cbegin() + 1, entries.cend(), [&](const std::string_view sv) {
                string_builder << " " << std::string(sv);
            });
        }
        return string_builder.str();
    }

    std::ostream& operator<<(std::ostream& os, const IES_Label_Data& labels) {
        const auto keys = labels.keys();
        std::for_each(keys.cbegin(), keys.cend() - 1, [&](const auto& key) {
            fmt::print(os, "{0} -- {1}\n", key, labels.at(key));
        });
        const auto last_key = keys.back();
        fmt::print(os, "{0} -- {1}", last_key, labels.at(last_key));
        return os;
    }

    std::string IES_Label_Data::to_string() const {
        return fmt::format("{}", *this);
    }
}