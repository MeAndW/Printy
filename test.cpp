// #include <iostream>

// struct Bar :  private std::streambuf , public std::ostream
// {
// private:
//     std::ostream& os;
// public:
//     // Bar() : std::ostream(this) {}
//     Bar(std::ostream& os) : std::ostream(this), os(os) {}

// private:
//     std::streamsize xsputn(const char* s, std::streamsize n) override
//     {
//         std::cout << "xsputn: " << n << " bytes" << std::endl;
//         os.write(s, n);
//         return n;
//     }
//     int overflow(int c) override
//     {
//         foo(c);
//         return 0;
//     }


//     void foo(char c)
//     {
//         os.put(c);
//     }
// };

// int main()
// {
//     Bar b(std::cout);
//     b << '\n';
//     b << "Look a number: " << 29 << std::endl;
//     b << "Look a number: " << 29 << std::endl;

//     return 0;
// }
#include <iostream>
#include <tuple>
#include <iomanip>
#include <ios>
#include <vector>
#include <type_traits>
#include <concepts>
#include <map>
// #include "p.hpp"

#include "printy.hpp"

using namespace std;

static_assert(ranges::range<vector<int>>);
static_assert(not ranges::viewable_range<vector<int>>);
static_assert(printy::insertable_range<vector<int>, char, char_traits<char>>);
static_assert(not ranges::view<vector<int>>);
static_assert(printy::container_sup_nest<vector<int>>);
static_assert(printy::container_sup_nest<tuple<int, int>>);
static_assert(not printy::container_sup_nest<char>);
static_assert(printy::string_or_string_view<string>);
static_assert(printy::string_or_string_view<string_view>);

using namespace printy::general;

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    array<int, 5> arr2 = {1, 2, 3, 4, 5};
    auto nested = array<array<int, 4>, 4>{
        {
            {1, 2, 3, 4},
            {5, 6, 7, 8},
            {9, 10, 11, 12},
            {13, 14, 15, 16}
        }
    };
    variant<int, string> v = "hello";
    variant<monostate, double, char> v2 = monostate{};

    cout << arr << endl;
    cout << arr2 << endl;
    cout << nested << endl;
    cout << v << endl;
    cout << v2 << endl;
    printy::custom::typed_ostream os(cout);

    os << arr << endl;
    os << arr2 << endl;
    os << nested << endl;
    os << v << endl;
    os << v2 << endl;
    os << "hello" << endl;
    os << string{"hello"} << endl;
    os << string_view{"\\"} << endl;
    os << tuple<int, int, int>{1, 2, 3} << endl;
    os << make_tuple(1, 2, 
        vector<double>{3.0, 4.0, 5.0},
        array<int, 3>{6, 7, 8},
        array<string, 3>{"a", "b", "c"},
        array<array<int, 3>, 3>{{
            {1, 2, 3}, 
            {4, 5, 6}, 
            {7, 8, 9},
        }},
        vector<tuple<const char*, int>>{
            make_tuple("hello", 1),
            make_tuple("world", 2),
        }
    ) << endl;
    os << map<int, string>{
        {1, "one"},
        {2, "two"},
        {3, "three"},
    } << endl;
    os << map<int, tuple<int, int, int>>{
        {1, {1, 2, 3}},
        {2, {4, 5, 6}},
        {3, {7, 8, 9}},
    } << endl;
    os << map<int, tuple<int, vector<int>>>{
        {1, {1, {1, 2, 3}}},
        {2, {2, {4, 5, 6}}},
        {3, {3, {7, 8, 9}}},
    } << endl;
    // uninsertable types
    os << os << endl;
    os << function<void(int)>{} << endl;
    os << cout << endl;
    os << cin << endl;
}