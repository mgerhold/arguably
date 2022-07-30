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
#include <fmt/ostream.h>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace arguably {
    namespace result {
        struct MissingArgument {
            char abbreviation;
        };

        struct NothingParsedYet { };
        struct UnknownOption {
            std::variant<char, std::string> option;
        };
        struct CannotParseAgain { };
        struct Okay { };
        struct ExcessUnnamedArguments { };
        struct CannotSetValueOfFlag {
            std::string option;
        };
        struct ArgumentTypeMismatch { };
    }// namespace result

    using ParseResult = std::variant<
            result::Okay,
            result::NothingParsedYet,
            result::MissingArgument,
            result::UnknownOption,
            result::CannotParseAgain,
            result::ExcessUnnamedArguments,
            result::CannotSetValueOfFlag,
            result::ArgumentTypeMismatch>;


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
    struct OptionallyNamedParameter {
        using ValueType = Type;

        constexpr static char abbreviation{ Abbreviation };
        constexpr static std::string_view name{ static_cast<std::string_view>(Name) };
        constexpr static std::string_view description{ static_cast<std::string_view>(Description) };
    };

    template<char Abbreviation, String Name, String Description, typename Type>
    struct NamedParameter {
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
    [[nodiscard]] constexpr bool is_named_parameter(NamedParameter<Abbreviation, Name, Description, Type>) {
        return true;
    }

    template<typename T>
    [[nodiscard]] constexpr bool is_named_parameter(T) {
        return false;
    }

    template<char Abbreviation, String Name, String Description, typename Type>
    [[nodiscard]] constexpr bool
    is_optionally_named_parameter(OptionallyNamedParameter<Abbreviation, Name, Description, Type>) {
        return true;
    }

    template<typename T>
    [[nodiscard]] constexpr bool is_optionally_named_parameter(T) {
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
            return static_cast<usize>((is_named_parameter(Arguments{}) + ...));
        }
    }

    template<typename... Arguments>
    consteval usize count_arguments() {
        if constexpr (sizeof...(Arguments) == 0) {
            return 0;
        } else {
            return static_cast<usize>((is_optionally_named_parameter(Arguments{}) + ...));
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

    template<typename First, typename... Rest>
    [[nodiscard]] constexpr std::optional<char> get_abbreviation_of_name_impl(const std::string_view name) {
        if (First::name == name) {
            return First::abbreviation;
        }
        if constexpr (sizeof...(Rest) > 0) {
            return get_abbreviation_of_name_impl<Rest...>(name);
        } else {
            return {};
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
    [[nodiscard]] constexpr bool is_named_parameter_impl([[maybe_unused]] const char abbreviation) {
        if constexpr (sizeof...(Arguments) == 0) {
            return false;
        } else {
            return ((is_named_parameter(Arguments{}) and Arguments::abbreviation == abbreviation) or ...);
        }
    }

    template<typename... Arguments>
    [[nodiscard]] constexpr bool is_optionally_named_parameter_impl([[maybe_unused]] const char abbreviation) {
        if constexpr (sizeof...(Arguments) == 0) {
            return false;
        } else {
            return ((is_optionally_named_parameter(Arguments{}) and Arguments::abbreviation == abbreviation) or ...);
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
        if constexpr (sizeof...(Rest) > 0) {
            print_default_values_impl<Offset + 1, Rest...>(default_values);
        }
    }

    template<usize Offset, typename Argument, typename... Rest>
    auto get_value(const AnyVector& values, const char abbreviation) {
        if (Argument::abbreviation == abbreviation) {
            return std::any_cast<typename Argument::ValueType>(values[Offset]);
        }
        if constexpr (sizeof...(Rest) > 0) {
            return get_value<Offset + 1, Rest...>(values, abbreviation);
        } else {
            throw;
        }
    }

    template<char Abbreviation, usize Offset, typename Argument, typename... Rest>
    auto get_value([[maybe_unused]] const AnyVector& values) {
        if constexpr (Argument::abbreviation == Abbreviation) {
            return std::any_cast<typename Argument::ValueType>(values[Offset]);
        } else if constexpr (sizeof...(Rest) > 0) {
            return get_value<Abbreviation, Offset + 1, Rest...>(values);
        } else {
            throw;
        }
    }

    template<usize Offset, typename Argument, typename... Rest, typename Value>
    [[nodiscard]] bool store_at_impl(AnyVector& values, const usize index, Value&& value) {
        if (index == Offset) {
            std::stringstream stream;
            stream << std::forward<Value>(value);
            auto result = typename Argument::ValueType{};
            stream >> result;
            if (stream.fail()) {
                return false;
            }
            values[index] = result;
            return true;
        } else if constexpr (sizeof...(Rest) > 0) {
            return store_at_impl<Offset + 1, Rest...>(values, index, std::forward<Value>(value));
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
            assert(not eof());
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

    template<usize Offset, typename First, typename... Rest, usize NumArgs>
    [[nodiscard]] std::optional<char> abbreviation_of_first_unseen_optionally_named(
            const std::array<bool, NumArgs>& found_args
    ) {
        if (not found_args[Offset] and is_optionally_named_parameter(First{})) {
            return First::abbreviation;
        }
        if constexpr (sizeof...(Rest) > 0) {
            return abbreviation_of_first_unseen_optionally_named<Offset + 1, Rest...>(found_args);
        } else {
            return {};
        }
    }

    template<String HelpText, typename... Arguments>
    class Parser final {
    private:
        using ArgumentsMap = std::unordered_map<char, std::string>;

    private:
        constexpr explicit Parser(AnyVector&& default_values) : m_values{ std::move(default_values) } { }

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
        constexpr void print_help(auto&& stream, usize max_name_width) const {
            fmt::print(
                    stream, "-{}, --{:{}}  {}\n", First::abbreviation, First::name, max_name_width, First::description
            );
            if constexpr (sizeof...(Rest) > 0) {
                print_help<Rest...>(stream, max_name_width);
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
            return std::holds_alternative<result::Okay>(m_parse_result);
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

        [[nodiscard]] std::optional<char> get_abbreviation_of_name(const std::string_view name) {
            return get_abbreviation_of_name_impl<Arguments...>(name);
        }

        [[nodiscard]] static constexpr bool is_flag(const char abbreviation) {
            return is_flag_impl<Arguments...>(abbreviation);
        }

        [[nodiscard]] static constexpr bool is_named_parameter(const char abbreviation) {
            return is_named_parameter_impl<Arguments...>(abbreviation);
        }

        [[nodiscard]] static constexpr bool is_optionally_named_parameter(const char abbreviation) {
            return is_optionally_named_parameter_impl<Arguments...>(abbreviation);
        }

        template<char Abbreviation>
        [[nodiscard]] consteval std::string_view get_name() {
            static_assert(has_abbreviation<Abbreviation>(), "unknown abbreviation");
            return get_name<Abbreviation, Arguments...>();
        }

        void print_help(auto&& out) const {
            const auto help_text = static_cast<std::string_view>(HelpText);
            if (not help_text.empty()) {
                fmt::print(std::forward<decltype(out)>(out), "{}\n", help_text);
            }
            print_help<Arguments...>(std::forward<decltype(out)>(out), max_name_length<Arguments...>());
        }

        template<char Abbreviation>
        [[nodiscard]] static consteval usize index_of() {
            return index_of_impl<Abbreviation, 0, Arguments...>();
        }

        [[nodiscard]] static constexpr usize index_of(const char abbreviation) {
            return index_of_impl<0, Arguments...>(abbreviation);
        }

        void print_default_values() const {
            print_default_values_impl<0, Arguments...>(m_values);
        }

        template<char Abbreviation>
        [[nodiscard]] bool was_provided() {
            static_assert(has_abbreviation<Abbreviation>(), "unknown abbreviation");
            static constexpr auto index = index_of<Abbreviation>();
            return m_arguments_found[index];
        }

        template<char Abbreviation>
        [[nodiscard]] auto get() const {
            static_assert(has_abbreviation<Abbreviation>(), "unknown abbreviation");
            return get_value<Abbreviation, 0, Arguments...>(m_values);
        }

        [[nodiscard]] static usize get_argc(const char** argv) {
            usize i;
            for (i = 1; argv[i] != nullptr; ++i) { }
            return i;
        }

        enum class ParserState {
            SingleDashArguments,
            DoubleDashArgument,
            AfterFreestandingDoubleDash,
            None,
        };

        template<typename T>
        [[nodiscard]] bool try_store_at(usize index, T&& value) {
            m_arguments_found[index] = true;
            const auto success = store_at_impl<0, Arguments...>(m_values, index, std::forward<T>(value));
            if (not success) {
                m_parse_result = result::ArgumentTypeMismatch{};
            }
            return success;
        }

        void set_flag(usize index) {
            m_arguments_found[index] = true;
            m_values[index] = true;
        }

        void parse(const char** argv) {
            if (not result_is<result::NothingParsedYet>()) {
                m_parse_result = result::CannotParseAgain{};
                return;
            }

            auto data = ArgumentsView{ .argc{ get_argc(argv) }, .argv{ argv } };
            auto state = ParserState::None;

            while (not data.eof()) {
                switch (state) {
                    case ParserState::None:
                        if (data.current() == '-') {
                            const auto next = data.peek();
                            if (next == '-') {
                                state = ParserState::DoubleDashArgument;
                                data.advance();// consume first dash
                                if (isspace(data.peek())) {
                                    state = ParserState::AfterFreestandingDoubleDash;
                                    data.advance();// consume second dash
                                    data.advance();// consume space
                                    break;
                                }
                            } else if (isspace(next)) {
                                const auto abbreviation =
                                        abbreviation_of_first_unseen_optionally_named<0, Arguments...>(m_arguments_found
                                        );
                                if (not abbreviation.has_value()) {
                                    m_parse_result = result::ExcessUnnamedArguments{};
                                    return;
                                }
                                const auto index = index_of(*abbreviation);
                                if (not try_store_at(index, "-")) {
                                    return;
                                }
                            } else {
                                state = ParserState::SingleDashArguments;
                            }
                            data.advance();
                            break;
                        }
                        if (not std::isspace(data.current())) {
                            if (not handle_unnamed_argument(data)) {
                                return;
                            }
                        }
                        break;
                    case ParserState::SingleDashArguments: {
                        if (not handle_single_dash_arguments(data, state)) {
                            return;
                        }
                        break;
                    }
                    case ParserState::DoubleDashArgument:
                        if (not handle_double_dash_argument(data, state)) {
                            return;
                        }
                        break;
                    case ParserState::AfterFreestandingDoubleDash:
                        if (not handle_unnamed_argument(data)) {
                            return;
                        }
                        break;
                }
            }
            m_parse_result = result::Okay{};
        }

    private:
        /// returns false on error
        [[nodiscard]] bool handle_unnamed_argument(ArgumentsView& data) {
            const auto argument = data.arg_tail();
            const auto abbreviation = abbreviation_of_first_unseen_optionally_named<0, Arguments...>(m_arguments_found);
            if (not abbreviation.has_value()) {
                m_parse_result = result::ExcessUnnamedArguments{};
                return false;
            }
            const auto index = index_of(*abbreviation);
            if (not try_store_at(index, argument)) {
                return false;
            }
            data.next_arg();
            return true;
        }

        /// returns false on error
        [[nodiscard]] bool handle_single_dash_arguments(ArgumentsView& data, ParserState& state) {
            if (isspace(data.current())) {
                state = ParserState::None;
                data.advance();
                return true;
            }
            if (is_flag(data.current())) {
                const auto index = index_of(data.current());
                set_flag(index);
                data.advance();
            } else if (is_named_parameter(data.current()) or is_optionally_named_parameter(data.current())) {
                const auto parameter_abbreviation = data.consume();
                const auto tail = data.arg_tail();
                const auto index = index_of(parameter_abbreviation);
                if (tail.empty()) {
                    data.next_arg();
                    if (data.eof()) {
                        m_parse_result = result::MissingArgument{ .abbreviation{ parameter_abbreviation } };
                        return false;
                    }
                    const auto argument = data.arg_tail();
                    if (not try_store_at(index, argument)) {
                        return false;
                    }
                } else {
                    if (not try_store_at(index, tail)) {
                        return false;
                    }
                }
                data.next_arg();
                state = ParserState::None;
            } else {
                m_parse_result = result::UnknownOption{ .option{ data.current() } };
                return false;
            }
            return true;
        }

        /// returns false on error
        [[nodiscard]] bool handle_double_dash_argument(ArgumentsView& data, ParserState& state) {
            const auto arg_tail = data.arg_tail();
            const auto equals_index = arg_tail.find('=');
            const auto equals_found = (equals_index != decltype(arg_tail)::npos);

            if (equals_found) {
                const auto parameter = arg_tail.substr(0, equals_index);
                const auto abbreviation = get_abbreviation_of_name(parameter);
                if (not abbreviation) {
                    m_parse_result = result::UnknownOption{ std::string{ parameter } };
                    return false;
                }
                if (is_flag(*abbreviation)) {
                    m_parse_result = result::CannotSetValueOfFlag{ std::string{ parameter } };
                    return false;
                }

                const auto argument = arg_tail.substr(equals_index + 1);
                if (argument.empty()) {
                    m_parse_result = result::MissingArgument{};
                    return false;
                }

                const auto index = index_of(*abbreviation);
                if (not try_store_at(index, argument)) {
                    return false;
                }
                data.next_arg();
            } else {
                const auto parameter = arg_tail;
                const auto abbreviation = get_abbreviation_of_name(parameter);
                if (not abbreviation) {
                    m_parse_result = result::UnknownOption{ std::string{ parameter } };
                    return false;
                }

                if (is_flag(*abbreviation)) {
                    const auto index = index_of(*abbreviation);
                    set_flag(index);
                } else {
                    data.next_arg();
                    if (data.eof()) {
                        m_parse_result = result::MissingArgument{};
                        return false;
                    }
                    const auto argument = data.arg_tail();
                    const auto index = index_of(*abbreviation);
                    if (not try_store_at(index, argument)) {
                        return false;
                    }
                }
                data.next_arg();
            }
            state = ParserState::None;
            return true;
        }

    private:
        std::array<bool, sizeof...(Arguments)> m_arguments_found{};
        AnyVector m_values;
        ParseResult m_parse_result = result::NothingParsedYet{};

        template<String, typename...>
        friend class ParserBuilder;
    };

    template<String HelpText, typename... Arguments>
    class ParserBuilder final {
    private:
        constexpr ParserBuilder() = default;

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
        [[nodiscard]] auto named(Type&& default_value) {
            check_duplicate<Arguments..., NamedParameter<Abbreviation, Name, Description, Type>>();
            m_default_values.emplace_back(std::forward<Type>(default_value));
            return ParserBuilder<HelpText, Arguments..., NamedParameter<Abbreviation, Name, Description, Type>>{
                std::move(m_default_values)
            };
        }

        template<char Abbreviation, String Name, String Description, typename Type>
        [[nodiscard]] auto optionally_named(Type&& default_value) {
            check_duplicate<Arguments..., OptionallyNamedParameter<Abbreviation, Name, Description, Type>>();
            m_default_values.emplace_back(std::forward<Type>(default_value));
            return ParserBuilder<
                    HelpText, Arguments..., OptionallyNamedParameter<Abbreviation, Name, Description, Type>>{
                std::move(m_default_values)
            };
        }

        template<String NewHelpText>
        [[nodiscard]] ParserBuilder<NewHelpText, Arguments...> help() {
            return ParserBuilder<NewHelpText, Arguments...>{ std::move(m_default_values) };
        }

        [[nodiscard]] Parser<HelpText, Arguments...> create() {
            return Parser<HelpText, Arguments...>{ std::move(m_default_values) };
        }

    private:
        AnyVector m_default_values;

        friend ParserBuilder<""> create_parser();
    };

    [[nodiscard]] ParserBuilder<""> create_parser() {
        return ParserBuilder<"">{};
    }

}// namespace arguably
