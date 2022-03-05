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
// #include "p.hpp"

#include "printy.hpp"

using namespace std;

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    array<int, 5> arr2 = {1, 2, 3, 4, 5};

    using namespace printy::general;


    cout << arr2 << endl;
    printy::general::typed_ostream os(cout);

    os << arr;

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
}