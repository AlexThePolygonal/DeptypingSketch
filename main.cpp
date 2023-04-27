#include <iostream>
#include "cexpr_lib.hpp"

namespace deptypes {
    namespace ops {

        template <auto v>
        struct Name {};

        template <auto v = [](){}>
        struct Source {};

        struct CexprSource {};

        template <int u>
        struct CexprInt {
            constexpr static int value = u;
        };
        
    };

    template <class Name = ops::Name<[](){}>>
    struct dependent_int {

        template <class Src, auto v = [](){}>
        struct RenewSrcs : type_var::Store<Name, replace_std::Argpass<Src>>{};
        
        template <class OtherName, auto v = [](){}>
        struct CopySrcs : type_var::Store<Name, type_var::value<OtherName>>{};


        using this_name = Name;
        int __v;

        template <class Int, class = RenewSrcs<ops::Source<[](){}>>>
        dependent_int(Int i) : __v(i) {}
 
        template <class T, class = CopySrcs<T>>
        dependent_int(const dependent_int<T>& other) : __v(other.__v) {}
    };
};

template <
    class T, 
    class = type_var::Store<T, replace_std::Argpass<deptypes::ops::Source<[](){}>>>, 
    auto=[](){}
>
std::istream& operator>>(std::istream& in, deptypes::dependent_int<T> i) {
    in >> i.__v;
    return in;
}


template <class T = deptypes::ops::Name<[](){}>>
using new_int = deptypes::dependent_int<T>;








//
// I WANT THIS
//
// BADLY
//

int main() {
    std::cout << std::boolalpha;
    new_int<> n(0);
    // std::cin >> n;
    // new_int<> i = n;
    // new_int<> j = n;
    // ++i;
    // ++j;
    // constexpr bool v = (i == j);
    // std::cout << v << '\n';
}