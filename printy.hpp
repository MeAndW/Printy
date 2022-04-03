// A helper library for printing various objects.
#include <iostream>
#include <iomanip>
#include <string>

#include <type_traits>
#include <utility>
#include <functional>
#include <concepts>
#include <ranges>

#include <string_view>
/**
 * @brief Credit to 
 * https://stackoverflow.com/questions/81870/is-it-possible-to-print-a-variables-type-in-standard-c/58331141#58331141
 * This needs <string_view> to work.
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
    template<typename CharT, typename Traits, typename T>
    concept insertable = requires(std::basic_ostream<CharT, Traits>& os, const T& t)
    {
        { os << t } -> std::same_as<std::basic_ostream<CharT, Traits>&>;
    };


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
            operator<<(std::basic_ostream<CharT, Traits>& os, const Indentation& indent);

            Indentation& operator++() noexcept { ++level; return *this; }
            Indentation& operator--() noexcept { --level; return *this; }
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

            template<typename T> typed_ostream& operator<<(const T& t)
            {
                os << punctuation.prefix << typeview::type_view<T>() << punctuation.delimiter << t << punctuation.suffix;
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

    namespace general 
    {
        struct Settings
        {
            static constexpr unsigned int indent_size = 4;
            static constexpr char indent_fill = ' ';

            static constexpr bool range_use_indent = true;
            static constexpr char range_prefix[] = "{";
            static constexpr char range_delimiter[] = ", ";
            static constexpr char range_suffix[] = "}";

            static constexpr bool tuple_use_indent = false;
            static constexpr char tuple_prefix[] = "(";
            static constexpr char tuple_delimiter[] = ", ";
            static constexpr char tuple_suffix[] = ")";

            static constexpr bool pair_use_indent = false;
            static constexpr char pair_prefix[] = "";
            static constexpr char pair_delimiter[] = ": ";
            static constexpr char pair_suffix[] = "";

            static unsigned int indent_level;

            template<typename CharT, typename Traits>
            static constexpr void apply_indent(int level_change, std::basic_ostream<CharT, Traits>& os)
            {
                indent_level += level_change;
                os << '\n';
                for (unsigned int i = 0, n = indent_level*indent_size; i < n; i++)
                    os << indent_fill;
            }
        };
        unsigned int Settings::indent_level = 0;

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
        template<typename CharT, typename Traits, std::ranges::range R>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const R& r)
        {
            typedef std::ranges::range_value_t<R> value_type;
            constexpr bool use_indent = std::ranges::range<value_type> && Settings::range_use_indent && !std::derived_from<value_type, std::string> && !std::derived_from<value_type, std::string_view>;

            os << Settings::range_prefix;
            if constexpr (use_indent)
                Settings::apply_indent(+1, os);
            bool first = true;
            for (auto&& e : r)
            {
                if (first)
                    first = false;
                else
                {
                    os << Settings::range_delimiter;
                    if constexpr (use_indent)
                        Settings::apply_indent(0, os);
                }
                os << e;
            }
            if constexpr (use_indent)
                Settings::apply_indent(-1, os);
            os << Settings::range_suffix;
            return os;
        }

        template <typename CharT, typename Traits, typename... Args>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::tuple<Args...>& tuple)
        {
            os << Settings::tuple_prefix;
            if (Settings::tuple_use_indent)
                Settings::apply_indent(+1, os);
            bool first = true;
            std::apply([&os, &first](auto &&...args) {
                auto print = [&] (auto&& val) 
                {
                    if (first)
                        first = false;
                    else
                        os << Settings::tuple_delimiter;
                    os << val;
                };
                (print(args), ...); 
            }, tuple);
            if (Settings::tuple_use_indent)
                Settings::apply_indent(-1, os);
            os << Settings::tuple_suffix;
            return os;
        }

        template <typename CharT, typename Traits, typename K, typename V>
        std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, const std::pair<K, V>& pair)
        {
            os << Settings::pair_prefix;
            if (Settings::pair_use_indent)
                Settings::apply_indent(+1, os);
            os << pair.first << Settings::pair_delimiter << pair.second;
            if (Settings::pair_use_indent)
                Settings::apply_indent(-1, os);
            os << Settings::pair_suffix;
            return os;
        }
        
        template<typename CharT, typename Traits>
        class typed_ostream
        {
            using Punctuation = custom::Punctuation;
        private:
            std::basic_ostream<CharT, Traits>& os;
            const Punctuation punctuation;
        public:
            typed_ostream(std::basic_ostream<CharT, Traits>& os, const Punctuation& punctuation = Punctuation("", "(", ")")) : 
            os(os), punctuation(punctuation) {}

            template<typename T> typed_ostream& operator<<(const T& t)
            {
                os << punctuation.prefix << typeview::type_view<T>() << punctuation.delimiter << t << punctuation.suffix;
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
    } // namespace general

} // namespace printy
