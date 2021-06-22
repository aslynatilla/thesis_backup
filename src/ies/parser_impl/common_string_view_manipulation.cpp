#include "common_string_view_manipulation.h"

namespace ies::string_view_manipulation {

    template<typename Predicate>
    std::string_view remove_suffix_while(Predicate condition, std::string_view view, size_t suffix_size) {
        while (view.size() > 0 && condition(view)) {
            view.remove_suffix(suffix_size);
        }
        return view;
    }

    template<typename Predicate>
    std::string_view remove_prefix_while(Predicate condition, std::string_view view, size_t prefix_size) {
        while (view.size() > 0 && condition(view)) {
            view.remove_prefix(prefix_size);
        }
        return view;
    }

    bool has_leading_space(const std::string_view& view) {
        return static_cast<bool>(isspace(view[0]));
    }

    bool has_trailing_space(const std::string_view& view) {
        return static_cast<bool>(isspace(view[view.size() - 1]));
    }

    bool has_leading_square_brackets(const std::string_view& view) {
        return view.starts_with('[') || view.starts_with(']');
    }

    bool has_trailing_square_brackets(const std::string_view& view) {
        return view.ends_with('[') || view.ends_with(']');
    }

    std::string_view remove_leading_and_trailing_spaces(std::string_view view) {
        view = remove_prefix_while(has_leading_space, view, 1);
        return remove_suffix_while(has_trailing_space, view, 1);
    }

    std::string_view remove_leading_and_trailing_square_brackets(std::string_view view) {
        view = remove_prefix_while(has_leading_square_brackets, view, 1);
        return remove_suffix_while(has_trailing_square_brackets, view, 1);
    }

    std::string_view remove_tl_spaces_and_brackets(std::string_view view) {
        return remove_leading_and_trailing_square_brackets(
                remove_leading_and_trailing_spaces(
                        view
                ));
    }

    std::string_view remove_prefix_if_present(std::string_view view, std::string_view prefix) {
        const std::size_t prefix_length = prefix.length();
        //  If prefix is found at the start of the string_view
        if (view.find(prefix) == 0) {
            view.remove_prefix(prefix_length);
        }
        return view;
    }

    std::string_view remove_spaces_brackets_and_keyword(std::string_view view) {
        return remove_tl_spaces_and_brackets(remove_prefix_if_present(view, "[MORE]"));
    }
}

