#ifndef REWORKED_PARSER_IES_PRINTING_H
#define REWORKED_PARSER_IES_PRINTING_H

#include <algorithm>
#include <sstream>
#include <string>

#include "fmt/ostream.h"

namespace ies::printing {
    template<typename ContiguousContainer, typename PrintingFunc, typename SeparationFunc>
    requires std::contiguous_iterator<typename ContiguousContainer::iterator> &&
             requires(const ContiguousContainer& container){
                 std::empty(container);
                 container.front();
             }
    void print_container(const ContiguousContainer& container,
                         const PrintingFunc& printer,
                         const SeparationFunc& separator) {
        if (std::empty(container)) {
            return;
        }

        printer(container.front());
        std::for_each(std::cbegin(container) + 1, std::cend(container),
                      [&separator, &printer](const auto& value) {
                          separator();
                          printer(value);
                      });
    }

    template<typename ContiguousContainer>
    requires std::contiguous_iterator<typename ContiguousContainer::iterator>
    void print_container_spaced(const ContiguousContainer& container, std::ostream& output, const int space_width) {
        print_container(container,
                        [&](const auto& value) {
                            fmt::print(output, "{:>{}}", value, space_width);
                        },
                        [&output]() { fmt::print(output, " "); });
    }

    template<typename ContiguousContainer>
    requires std::contiguous_iterator<typename ContiguousContainer::iterator> &&
             requires(const ContiguousContainer& container){
                 std::empty(container);
                 ContiguousContainer(std::begin(container), std::end(container));
             }
    std::vector<ContiguousContainer> chunk_container(const ContiguousContainer& container, const int values_per_chunk) {
        std::vector<ContiguousContainer> chunks;
        if (std::empty(container)) {
            return chunks;
        }
        const auto end = std::cend(container);
        auto chunk_begin = std::cbegin(container);
        while (chunk_begin != end) {
            if (std::distance(chunk_begin, end) < values_per_chunk) {
                chunks.emplace_back(ContiguousContainer(chunk_begin, end));
                chunk_begin = end;
            } else {
                chunks.emplace_back(ContiguousContainer(chunk_begin, chunk_begin + values_per_chunk));
                std::advance(chunk_begin, values_per_chunk);
            }
        }
        return chunks;
    }

    template<typename ContiguousContainer>
    requires std::contiguous_iterator<typename ContiguousContainer::iterator>
    void print_container_spaced_multiline(const ContiguousContainer& container, std::ostream& output,
                                          const int values_per_line, const int space_width) {
        const auto lines = chunk_container(container, values_per_line);
        std::for_each(lines.begin(), lines.end() - 1, [&](const auto& line) {
            print_container_spaced(line, output, space_width);
            output << "\n";
        });

        print_container_spaced(lines.back(), output, space_width);
    }
}
#endif //REWORKED_PARSER_IES_PRINTING_H
