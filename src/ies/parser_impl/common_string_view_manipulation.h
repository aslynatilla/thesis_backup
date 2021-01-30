#ifndef COMMON_STRING_VIEW_MANIPULATION_H
#define COMMON_STRING_VIEW_MANIPULATION_H

#include <string_view>
#include <locale>


namespace ies::string_view_manipulation {

    template<typename Predicate>
    std::string_view remove_suffix_while(Predicate condition, std::string_view view, size_t suffix_size);

    template<typename Predicate>
    std::string_view remove_prefix_while(Predicate condition, std::string_view view, size_t prefix_size);

    bool has_leading_space(const std::string_view& view);

    bool has_trailing_space(const std::string_view& view);

    bool has_leading_square_brackets(const std::string_view& view);

    bool has_trailing_square_brackets(const std::string_view& view);

    std::string_view remove_leading_and_trailing_spaces(std::string_view view);

    std::string_view remove_leading_and_trailing_square_brackets(std::string_view view);

    std::string_view remove_tl_spaces_and_brackets(std::string_view view);

    std::string_view remove_prefix_if_present(std::string_view view, std::string_view prefix);

    std::string_view remove_spaces_brackets_and_keyword(std::string_view view);
}

#endif //COMMON_STRING_VIEW_MANIPULATION_H
