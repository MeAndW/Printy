// A helper library for printing various objects.
#pragma once
#include <iostream>
#include <iomanip>
#include <string>

#include <type_traits>
#include <utility>
#include <functional>
#include <algorithm>
#include <concepts>
#include <ranges>

#include <variant>
#include <optional>

#include <string_view>
/**
 * @brief Credit to 
 * https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/58331141#58331141
 * Require <string_view>.
 */
namespace typeview
{
    using namespace std;
    template <typename T>
    constexpr string_view wrapped_type_view()
    {
#ifdef __clang__
        return __PRETTY_FUNCTION__;
#elif defined(__GNUC__)
        return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)
        return __FUNCSIG__;
#endif
    }

    class probe_type;
    constexpr string_view probe_type_view("typeview::probe_type");
    constexpr string_view probe_type_view_elaborated("class typeview::probe_type");
    constexpr string_view probe_type_view_used(wrapped_type_view<probe_type>().find(probe_type_view_elaborated) != -1 ? probe_type_view_elaborated : probe_type_view);

    constexpr size_t prefix_size()
    {
        return wrapped_type_view<probe_type>().find(probe_type_view_used);
    }

    constexpr size_t suffix_size()
    {
        return wrapped_type_view<probe_type>().length() - prefix_size() - probe_type_view_used.length();
    }

    template <typename T>
    string_view type_view()
    {
        constexpr auto type_view = wrapped_type_view<T>();

        return type_view.substr(prefix_size(), type_view.length() - prefix_size() - suffix_size());
    }
}

#define pdebug(x) std::cout << std::string(sizeof(#x), '-') << '\n' \
                           << #x << '\n'                      \
                           << std::string(sizeof(#x), '.') << '\n' \
                           << (x) << '\n'                     \
                           << std::string(sizeof(#x), '-') << '\n'
namespace printy
{
    template<typename T, typename CharT, typename Traits>
    concept insertable = requires(std::basic_ostream<CharT, Traits>& os, const T& t)
    {
        { os << t } -> std::same_as<std::basic_ostream<CharT, Traits>&>;
    };
    template<typename T, typename CharT, typename Traits>
    concept uninsertable = !insertable<T, CharT, Traits>;

    template<typename R, typename CharT, typename Traits>
    concept insertable_range = 
        std::ranges::input_range<R> &&
        insertable<std::ranges::range_value_t<R>, CharT, Traits>;

    template<typename R>
    concept nested_input_range = 
        std::ranges::input_range<R> &&
        std::ranges::input_range<std::ranges::range_value_t<R>>;

    template<typename>
    struct is_tuple : std::false_type {};
    template<typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};
    template<typename T>
    constexpr bool is_tuple_v = is_tuple<T>::value;

    template<typename>
    struct is_pair : std::false_type {};
    template<typename T, typename U>
    struct is_pair<std::pair<T, U>> : std::true_type {};
    template<typename T>
    constexpr bool is_pair_v = is_pair<T>::value;

    template<typename>
    struct is_string : std::false_type {};
    template<typename CharT, typename Traits, typename Alloc>
    struct is_string<std::basic_string<CharT, Traits, Alloc>> : std::true_type {};
    template<typename T>
    constexpr bool is_string_v = is_string<T>::value;

    template<typename>
    struct is_string_view : std::false_type {};
    template<typename CharT, typename Traits>
    struct is_string_view<std::basic_string_view<CharT, Traits>> : std::true_type {};
    template<typename T>
    constexpr bool is_string_view_v = is_string_view<T>::value;


    template<typename T>
    constexpr bool is_string_or_string_view_v = is_string_v<T> || is_string_view_v<T>;

    template<typename T>
    constexpr bool is_string_or_string_view_or_cs_v = is_string_or_string_view_v<T> || std::is_same_v<T, const char*>;

    template<typename T>
    concept string_or_string_view = is_string_or_string_view_v<T>;

    template<typename T>
    concept string_type = is_string_or_string_view_or_cs_v<T>;

    template<typename T>
    concept container_sup_nest = 
        !string_type<T> &&
        (std::ranges::input_range<T> || is_tuple_v<T>);

    // Use this namespace in the global scope to let its overloads be used everywhere.
    namespace general 
    {
        static unsigned int indent_level;

        template<typename CharT, typename Traits>
        static constexpr void apply_indent(int level_change, std::basic_ostream<CharT, Traits>& os)
        {
            indent_level += level_change;
            os << '\n';
            for (unsigned int i = 0, n = indent_level * 4; i < n; i++)
                os << ' ';
        }
        
        /**
         * @brief The overload responsible for inserting a range. 
         * Normally, if the inserted range is a range of ranges, a special style is applied. 
         * `std::string` and `std::string_view` in particular are excluded from this special style.
         * 
         * @tparam CharT 
         * @tparam Traits 
         * @tparam R 
         * @param os 
         * @param r 
         * @return std::basic_ostream<CharT, Traits>& 
         */
        template<typename CharT, typename Traits, insertable_range<CharT, Traits> R>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const R& r)
        {
            using value_type = std::ranges::range_value_t<R>;
            constexpr bool use_indent = container_sup_nest<value_type>;
            os << '{';
            if constexpr (use_indent)
                apply_indent(+1, os);
            bool first = true;
            for (auto&& e : r)
            {
                if (first)
                    first = false;
                else
                {
                    os << ", ";
                    if constexpr (use_indent)
                        apply_indent(0, os);
                }
                os << e;
            }
            if constexpr (use_indent)
                apply_indent(-1, os);
            os << '}';
            return os;
        }

        template <typename CharT, typename Traits, insertable<CharT, Traits>... Args>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::tuple<Args...>& tuple)
        {
            constexpr bool use_indent = (container_sup_nest<Args> || ...);
            os << '(';
            if constexpr (use_indent)
                apply_indent(+1, os);
            std::apply([&](const auto&... args) {
                bool first = true;
                ([&](const auto& arg) {
                    if (first)
                        first = false;
                    else
                    {
                        os << ", ";
                        if constexpr (use_indent)
                            apply_indent(0, os);
                    }
                    os << arg;
                }(args), ...);
            }, tuple);
            if constexpr (use_indent)
                apply_indent(-1, os);
            os << ')';
            return os;
        }

        template <typename CharT, typename Traits, insertable<CharT, Traits> T, insertable<CharT, Traits> U>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::pair<T, U>& pair)
        {
            return os << '(' << pair.first << ": " << pair.second << ')';
        }

        template<typename CharT, typename Traits>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::monostate&)
        {
            os << "std::monostate";
            return os;
        }

        template<typename CharT, typename Traits, insertable<CharT, Traits> Arg, insertable<CharT, Traits>... Args>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::variant<Arg, Args...>& variant)
        {
            if (variant.valueless_by_exception())
                return os << "valueless_by_exception";
            std::visit([&](const auto& arg) {
                os << arg;
            }, variant);
            return os;
        }

        template<typename CharT, typename Traits, insertable<CharT, Traits> Arg>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::optional<Arg>& optional)
        {
            if (optional.has_value())
                os << optional.value();
            else
                os << "std::nullopt";
            return os;
        }

    } // namespace general

    /**
     * @brief printy::custom 
     * A namespace that contains constructs that control how to print more complex types.
     * 
     */
    namespace custom 
    {
        struct Indentation 
        {
            unsigned int level = 0;
            const unsigned int size = 4;
            const char fill = ' ';
            constexpr Indentation() noexcept = default;
            constexpr Indentation(unsigned int level, unsigned int size, char fill) noexcept : 
            level(level), size(size), fill(fill) {}

            template<typename CharT, typename Traits>
            friend std::basic_ostream<CharT, Traits>& 
            operator<<(std::basic_ostream<CharT, Traits>& os, const Indentation& indent)
            {
                os << '\n';
                for (unsigned int i = 0, n = indent.level * indent.size; i < n; i++)
                    os << indent.fill;
                return os;
            }

            Indentation operator++() noexcept { ++level; return *this; }
            Indentation operator--() noexcept { --level; return *this; }
            Indentation operator++(int) noexcept { return Indentation(level++, size, fill); }
            Indentation operator--(int) noexcept { return Indentation(level--, size, fill); }
        };

        struct Punctuation 
        {
            const bool use_indentation = true;
            const char* prefix = "[";
            const char* delimiter = ", ";
            const char* suffix = "]";
            
            constexpr Punctuation() noexcept = default;
            constexpr Punctuation(const char* prefix, const char* delimiter, const char* suffix) noexcept :
            prefix(prefix), delimiter(delimiter), suffix(suffix) {}
        };

        template<typename CharT, typename Traits>
        class typed_ostream
        {
        private:
            std::basic_ostream<CharT, Traits>& os;
            const Punctuation punctuation;
        public:
            typed_ostream(std::basic_ostream<CharT, Traits>& os, const Punctuation& punctuation = Punctuation("", "(", ")")) : 
            os(os), punctuation(punctuation) {}
            
            template<insertable<CharT, Traits> T> typed_ostream& operator<<(const T& t)
            {
                os << punctuation.prefix << typeview::type_view<T>() << punctuation.delimiter << t << punctuation.suffix;
                return *this;
            }
            template<uninsertable<CharT, Traits> T> typed_ostream& operator<<(const T& t)
            {
                os << typeview::type_view<T>();
                return *this;
            }
            


            typed_ostream& operator<<(typed_ostream& (*pf)(typed_ostream&))
            { return pf(*this); }

            typed_ostream& operator<<(std::ostream& (*pf)(std::ostream&))
            { 
                pf(os);
                return *this;
            }

            typed_ostream& operator<<(std::ios_base& (*pf)(std::ios_base&))
            { 
                pf(os);
                return *this;
            }
        };
    } // namespace custom

} // namespace printy
