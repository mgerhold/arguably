#pragma once

#include <algorithm>
#include <any>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fmt/core.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace Arguably {
    namespace Result {
        struct MissingArgument {
            char abbreviation;
        };

        struct NothingParsedYet { };
        struct UnknownOption {
            std::variant<char, std::string> option;
        };
        struct CannotParseAgain { };
        struct Okay { };
    }// namespace Result

    using ParseResult = std::variant<
            Result::Okay,
            Result::NothingParsedYet,
            Result::MissingArgument,
            Result::UnknownOption,
            Result::CannotParseAgain>;


    using usize = std::size_t;
    using u8 = std::uint8_t;

    template<usize Length>
    struct String {
        constexpr String(const char (&value)[Length]) {
            std::copy(std::begin(value), std::end(value), std::begin(this->value));
        }

        explicit constexpr operator std::string_view() const {
            return std::string_view{ std::begin(value), std::end(value) - 1 };
        }

        std::array<char, Length> value;
    };

    template<char Abbreviation, String Name, String Description>
    struct Flag {
        using ValueType = bool;

        constexpr static char abbreviation{ Abbreviation };
        constexpr static std::string_view name{ static_cast<std::string_view>(Name) };
        constexpr static std::string_view description{ static_cast<std::string_view>(Description) };
    };

    template<char Abbreviation, String Name, String Description, typename Type>
    struct Argument {
        using ValueType = Type;

        constexpr static char abbreviation{ Abbreviation };
        constexpr static std::string_view name{ static_cast<std::string_view>(Name) };
        constexpr static std::string_view description{ static_cast<std::string_view>(Description) };
    };

    template<char Abbreviation, String Name, String Description, typename Type>
    struct Parameter {
        using ValueType = Type;

        constexpr static char abbreviation{ Abbreviation };
        constexpr static std::string_view name{ static_cast<std::string_view>(Name) };
        constexpr static std::string_view description{ static_cast<std::string_view>(Description) };
    };

    template<typename Argument, typename... Rest>
    constexpr void check_duplicate() {
        using namespace std::string_view_literals;

        static_assert(Argument::abbreviation != 'h', "h is a reserved argument abbreviation");
        static_assert(Argument::name != "help"sv, R"("help" is a reserved argument name)");

        if constexpr (sizeof...(Rest) != 0) {
            static_assert(
                    ((Argument::abbreviation != Rest::abbreviation) and ...),
                    "duplicate argument abbreviations are not allowed"
            );
            static_assert(((Argument::name != Rest::name) and ...), "duplicate argument names are not allowed");
            check_duplicate<Rest...>();
        }
    }

    template<char Abbreviation, String Name, String Description>
    [[nodiscard]] constexpr bool is_flag(Flag<Abbreviation, Name, Description>) {
        return true;
    }

    template<typename T>
    [[nodiscard]] constexpr bool is_flag(T) {
        return false;
    }

    template<char Abbreviation, String Name, String Description, typename Type>
    [[nodiscard]] constexpr bool is_parameter(Parameter<Abbreviation, Name, Description, Type>) {
        return true;
    }

    template<typename T>
    [[nodiscard]] constexpr bool is_parameter(T) {
        return false;
    }

    template<char Abbreviation, String Name, String Description, typename Type>
    [[nodiscard]] constexpr bool is_argument(Argument<Abbreviation, Name, Description, Type>) {
        return true;
    }

    template<typename T>
    [[nodiscard]] constexpr bool is_argument(T) {
        return false;
    }

    template<typename... Arguments>
    consteval usize count_flags() {
        if constexpr (sizeof...(Arguments) == 0) {
            return 0;
        } else {
            return static_cast<usize>((is_flag(Arguments{}) + ...));
        }
    }

    template<typename... Arguments>
    consteval usize count_parameters() {
        if constexpr (sizeof...(Arguments) == 0) {
            return 0;
        } else {
            return static_cast<usize>((is_parameter(Arguments{}) + ...));
        }
    }

    template<typename... Arguments>
    consteval usize count_arguments() {
        if constexpr (sizeof...(Arguments) == 0) {
            return 0;
        } else {
            return static_cast<usize>((is_argument(Arguments{}) + ...));
        }
    }

    template<char Abbreviation, typename First, typename... Rest>
    [[nodiscard]] consteval bool has_abbreviation_impl() {
        if constexpr (First::abbreviation == Abbreviation) {
            return true;
        } else if constexpr (sizeof...(Rest) > 0) {
            return has_abbreviation_impl<Abbreviation, Rest...>();
        } else {
            return false;
        }
    }

    template<typename... Arguments>
    [[nodiscard]] constexpr bool is_flag_impl([[maybe_unused]] const char abbreviation) {
        if constexpr (sizeof...(Arguments) == 0) {
            return false;
        } else {
            return ((is_flag(Arguments{}) and Arguments::abbreviation == abbreviation) or ...);
        }
    }

    template<typename... Arguments>
    [[nodiscard]] constexpr bool is_parameter_impl([[maybe_unused]] const char abbreviation) {
        if constexpr (sizeof...(Arguments) == 0) {
            return false;
        } else {
            return ((is_parameter(Arguments{}) and Arguments::abbreviation == abbreviation) or ...);
        }
    }

    template<typename... Arguments>
    [[nodiscard]] constexpr bool is_argument_impl([[maybe_unused]] const char abbreviation) {
        if constexpr (sizeof...(Arguments) == 0) {
            return false;
        } else {
            return ((is_argument(Arguments{}) and Arguments::abbreviation == abbreviation) or ...);
        }
    }

    template<char Abbreviation, usize Offset, typename Argument, typename... Rest>
    [[nodiscard]] consteval usize index_of_impl() {
        static_assert(has_abbreviation_impl<Abbreviation, Argument, Rest...>(), "unknown abbreviation");
        if constexpr (Argument::abbreviation == Abbreviation) {
            return Offset;
        } else if constexpr (sizeof...(Rest) > 0) {
            return index_of_impl<Abbreviation, Offset + 1, Rest...>();
        } else {
            throw;
        }
    }

    template<usize Offset, typename Argument, typename... Rest>
    [[nodiscard]] constexpr usize index_of_impl(const char abbreviation) {
        if (Argument::abbreviation == abbreviation) {
            return Offset;
        } else if constexpr (sizeof...(Rest) > 0) {
            return index_of_impl<Offset + 1, Rest...>(abbreviation);
        } else {
            throw;
        }
    }

    struct AnyVector : public std::vector<std::any> { };

    template<usize Offset, typename Argument, typename... Rest>
    void print_default_values_impl(const AnyVector& default_values) {
        fmt::print(
                "default value of {} is: {}\n", Argument::name,
                std::any_cast<typename Argument::ValueType>(default_values[Offset])
        );
        if constexpr (sizeof...(Rest) > 0) {
            print_default_values_impl<Offset + 1, Rest...>(default_values);
        }
    }

    template<usize Offset, typename Argument, typename... Rest>
    auto get_default_value(const AnyVector& default_values, const char abbreviation) {
        if (Argument::abbreviation == abbreviation) {
            return std::any_cast<typename Argument::ValueType>(default_values[Offset]);
        }
        if constexpr (sizeof...(Rest) > 0) {
            return get_default_value<Offset + 1, Rest...>(default_values, abbreviation);
        } else {
            throw;
        }
    }

    template<char Abbreviation, usize Offset, typename Argument, typename... Rest>
    auto get_default_value([[maybe_unused]] const AnyVector& default_values) {
        if constexpr (Argument::abbreviation == Abbreviation) {
            return std::any_cast<typename Argument::ValueType>(default_values[Offset]);
        } else if constexpr (sizeof...(Rest) > 0) {
            return get_default_value<Abbreviation, Offset + 1, Rest...>(default_values);
        } else {
            throw;
        }
    }

    template<usize Offset, typename Argument, typename... Rest, typename Value>
    void store_at_impl(AnyVector& values, const usize index, Value&& value) {
        if (index == Offset) {
            std::stringstream stream;
            stream << std::forward<Value>(value);
            auto result = typename Argument::ValueType{};
            stream >> result;
            values[index] = result;
        } else if constexpr (sizeof...(Rest) > 0) {
            store_at_impl<Offset + 1, Rest...>(values, index, std::forward<Value>(value));
        } else {
            throw;
        }
    }

    struct ArgumentsView {
        usize argument_index{ 1 };
        usize char_offset{ 0 };
        usize argc;
        const char** argv;

        [[nodiscard]] char current() const {
            const auto at_current = argv[argument_index][char_offset];
            return at_current == '\0' ? ' ' : at_current;
        }

        [[nodiscard]] char consume() {
            const auto result = current();
            advance();
            return result;
        }

        [[nodiscard]] char peek() const {
            if (eof()) {
                return '\0';
            }
            const auto [new_argument_index, new_char_offset] = *next_position();
            const auto next_char = argv[new_argument_index][new_char_offset];
            return next_char == '\0' ? ' ' : next_char;
        }

        void next_arg() {
            ++argument_index;
            char_offset = 0;
        }

        [[nodiscard]] std::optional<std::pair<usize, usize>> next_position() const {
            if (argument_index >= argc) {
                return {};
            }
            if (argv[argument_index][char_offset] == '\0') {
                if (argument_index >= argc - 1) {
                    return {};
                }
                return {
                    std::pair{argument_index + 1, 0}
                };
            }
            return {
                std::pair{argument_index, char_offset + 1}
            };
        }

        void advance() {
            if (eof()) {
                return;
            }

            const auto [new_argument_index, new_char_offset] = *next_position();
            argument_index = new_argument_index;
            char_offset = new_char_offset;
        }

        [[nodiscard]] std::string_view arg_tail() const {
            return std::string_view{ argv[argument_index] }.substr(char_offset);
        }

        [[nodiscard]] std::string_view consume_arg() {
            const auto result = arg_tail();
            ++argument_index;
            char_offset = 0;
            return result;
        }

        [[nodiscard]] bool eof() const {
            return not next_position().has_value();
        }
    };

    template<String HelpText, typename... Arguments>
    class Parser {
    private:
        using ArgumentsMap = std::unordered_map<char, std::string>;

    private:
        explicit Parser(AnyVector&& default_values) : m_values{ std::move(default_values) } { }

        template<typename First, typename... Rest>
        [[nodiscard]] constexpr static usize max_name_length() {
            constexpr usize first_length = First::name.length();
            if constexpr (sizeof...(Rest) == 0) {
                return first_length;
            } else {
                constexpr usize max_rest_length = max_name_length<Rest...>();
                if constexpr (first_length > max_rest_length) {
                    return first_length;
                } else {
                    return max_rest_length;
                }
            }
        }

        template<typename First, typename... Rest>
        constexpr void print_help(std::FILE* file, usize max_name_width) const {
            fmt::print(
                    file, "-{}, --{:{}}  {}\n", First::abbreviation, First::name, max_name_width, First::description
            );
            if constexpr (sizeof...(Rest) > 0) {
                print_help<Rest...>(file, max_name_width);
            }
        }

        template<typename First, typename... Rest>
        [[nodiscard]] static constexpr std::string_view get_name(const char abbreviation) {
            if (First::abbreviation == abbreviation) {
                return First::name;
            }
            if constexpr (sizeof...(Rest) > 0) {
                return get_name<Rest...>(abbreviation);
            }
            return "";
        }

        template<char Abbreviation, typename First, typename... Rest>
        [[nodiscard]] consteval std::string_view get_name() {
            if constexpr (First::abbreviation == Abbreviation) {
                return First::name;
            } else if constexpr (sizeof...(Rest) > 0) {
                return get_name<Abbreviation, Rest...>();
            } else {
                throw;
            }
        }

    public:
        operator bool() const {
            return std::holds_alternative<Result::Okay>(m_parse_result);
        }

        [[nodiscard]] ParseResult result() const {
            return m_parse_result;
        }

        template<typename T>
        [[nodiscard]] bool result_is() const {
            return std::holds_alternative<T>(m_parse_result);
        }

        template<char Abbreviation>
        [[nodiscard]] static consteval bool has_abbreviation() {
            return has_abbreviation_impl<Abbreviation, Arguments...>();
        }

        [[nodiscard]] static constexpr bool is_flag(const char abbreviation) {
            return is_flag_impl<Arguments...>(abbreviation);
        }

        [[nodiscard]] static constexpr bool is_parameter(const char abbreviation) {
            return is_parameter_impl<Arguments...>(abbreviation);
        }

        [[nodiscard]] static constexpr bool is_argument(const char abbreviation) {
            return is_argument_impl<Arguments...>(abbreviation);
        }

        template<char Abbreviation>
        [[nodiscard]] consteval std::string_view get_name() {
            static_assert(has_abbreviation<Abbreviation>(), "unknown abbreviation");
            return get_name<Abbreviation, Arguments...>();
        }

        void print_help(std::FILE* file = stdout) const {
            print_help<Arguments...>(file, max_name_length<Arguments...>());
        }

        template<char Abbreviation>
        [[nodiscard]] consteval usize index_of() const {
            return index_of_impl<Abbreviation, 0, Arguments...>();
        }

        [[nodiscard]] constexpr usize index_of(const char abbreviation) const {
            return index_of_impl<0, Arguments...>(abbreviation);
        }

        void print_default_values() const {
            print_default_values_impl<0, Arguments...>(m_values);
        }

        template<char Abbreviation>
        [[nodiscard]] auto get() const {
            static_assert(has_abbreviation<Abbreviation>(), "unknown abbreviation");
            return get_default_value<Abbreviation, 0, Arguments...>(m_values);
        }

        [[nodiscard]] static usize get_argc(const char** argv) {
            usize i;
            for (i = 1; argv[i] != nullptr; ++i) { }
            return i;
        }

        enum class ParserState {
            SingleDashArguments,
            DoubleDashArgument,
            UnnamedArgument,
            None,
        };

        template<typename T>
        void store_at(usize index, T&& value) {
            store_at_impl<0, Arguments...>(m_values, index, std::forward<T>(value));
        }

        void parse(const char** argv) {
            if (not std::holds_alternative<Result::NothingParsedYet>(m_parse_result)) {
                m_parse_result = Result::CannotParseAgain{};
                return;
            }

            auto data = ArgumentsView{ .argc{ get_argc(argv) }, .argv{ argv } };
            auto state = ParserState::None;

            while (not data.eof()) {
                switch (state) {
                    case ParserState::None:
                        if (data.current() == '-') {
                            if (data.peek() == '-') {
                                state = ParserState::DoubleDashArgument;
                                data.advance();
                                if (isspace(data.peek())) {
                                    return;
                                }
                            } else {
                                state = ParserState::SingleDashArguments;
                            }
                            data.advance();
                        }
                        break;
                    case ParserState::SingleDashArguments: {
                        if (isspace(data.current())) {
                            state = ParserState::None;
                            data.advance();
                            break;
                        }
                        if (is_flag(data.current())) {
                            fmt::print("{} is a flag!\n", data.current());
                            const auto index = index_of(data.current());
                            m_values[index] = true;
                        } else if (is_parameter(data.current())) {
                            fmt::print("{} is a parameter!\n", data.current());
                            const auto parameter_abbreviation = data.consume();
                            const auto tail = data.arg_tail();
                            const auto index = index_of(parameter_abbreviation);
                            if (tail.empty()) {
                                data.next_arg();
                                if (data.eof()) {
                                    m_parse_result = Result::MissingArgument{ .abbreviation{ parameter_abbreviation } };
                                    return;
                                }
                                const auto argument = data.arg_tail();
                                store_at(index, argument);
                            } else {
                                store_at(index, tail);
                            }
                            data.next_arg();
                        } else {
                            m_parse_result = Result::UnknownOption{ .option{ data.current() } };
                            return;
                        }
                        data.advance();
                        break;
                    }
                }
            }
            m_parse_result = Result::Okay{};
        }

    public:
        static constexpr usize num_flags = count_flags<Arguments...>();
        static constexpr usize num_parameters = count_parameters<Arguments...>();
        static constexpr usize num_arguments = count_arguments<Arguments...>();

    private:
        std::array<std::string_view, sizeof...(Arguments)> m_raw_values;
        // std::array<bool, sizeof...(Arguments)> m_argument_found;
        AnyVector m_values;
        ParseResult m_parse_result = Result::NothingParsedYet{};

        template<String, typename...>
        friend class ParserBuilder;
    };

    template<String HelpText, typename... Arguments>
    class ParserBuilder {
    private:
        ParserBuilder() = default;

        explicit ParserBuilder(AnyVector&& default_values) : m_default_values{ std::move(default_values) } { }

        template<String, typename...>
        friend class ParserBuilder;

    public:
        template<char Abbreviation, String Name, String Description>
        [[nodiscard]] auto flag() {
            check_duplicate<Arguments..., Flag<Abbreviation, Name, Description>>();
            m_default_values.emplace_back(false);
            return ParserBuilder<HelpText, Arguments..., Flag<Abbreviation, Name, Description>>{
                std::move(m_default_values)
            };
        }

        template<char Abbreviation, String Name, String Description, typename Type>
        [[nodiscard]] auto parameter(Type&& default_value) {
            check_duplicate<Arguments..., Parameter<Abbreviation, Name, Description, Type>>();
            m_default_values.emplace_back(std::forward<Type>(default_value));
            return ParserBuilder<HelpText, Arguments..., Parameter<Abbreviation, Name, Description, Type>>{
                std::move(m_default_values)
            };
        }

        template<char Abbreviation, String Name, String Description, typename Type>
        [[nodiscard]] auto argument(Type&& default_value) {
            check_duplicate<Arguments..., Argument<Abbreviation, Name, Description, Type>>();
            m_default_values.emplace_back(std::forward<Type>(default_value));
            return ParserBuilder<HelpText, Arguments..., Argument<Abbreviation, Name, Description, Type>>{
                std::move(m_default_values)
            };
        }

        template<String NewHelpText>
        [[nodiscard]] ParserBuilder<NewHelpText, Arguments...> help() {
            return ParserBuilder<NewHelpText, Arguments...>{ std::move(m_default_values) };
        }

        [[nodiscard]] Parser<HelpText, Arguments...> create() {
            fmt::print("creating parser with {} default values...\n", m_default_values.size());
            return Parser<HelpText, Arguments...>{ std::move(m_default_values) };
        }

    private:
        AnyVector m_default_values;

        friend ParserBuilder<""> create_parser();
    };

    [[nodiscard]] ParserBuilder<""> create_parser() {
        return ParserBuilder<"">{};
    }

}// namespace Arguably
