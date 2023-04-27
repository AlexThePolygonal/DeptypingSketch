#pragma once


/// GENERAL WARNINGS :
/// compile with clang, gcc goes insane.
/// std=c++20 REQUIRED
/// tested with clang++-15
/// expect frequent random frontend crashes


#define USEFUL_CEXPR_AUXS
#define CEXPR_IF
#define CEXPR_RECURSION







///
/// Helper: using std may result in excessive crashing
///
namespace replace_std {
    template <class T, class U>
    struct is_same {
        constexpr static bool value = false;
    };
    template <class T>
    struct is_same<T, T> {
        constexpr static bool value = true;
    };
    template <class T, class U> 
    constexpr bool is_same_v = is_same<T, U>::value;

    struct monost {};

    template <class ... Args>
    struct Argpass {
        template <class ... Ts>
        static auto Merge(Argpass<Ts...>) {
            return Argpass<Ts ..., Args ...>{};
        }
    };
    template <>
    struct Argpass<monost> {
        template <class ... Ts> 
        static auto Merge(Argpass<Ts ... >) {
            return Argpass<Ts ...>{};
        }
    };
    
    template <class T, class U>
    const T& cast(const U& ref) { return (const T&)ref; }

    template<typename ...Args>
    inline void pass(Args&&...) {}


    template<typename... Args>
    struct GetFirstArg {
        template<typename T, typename ... Ts>
        struct Chipper {
            using type = T;
        };

        using T = typename Chipper<Args...>::type;
    };

    template <const char v[]>
    struct storage {
        static constexpr auto value = v;
    };
};






///
/// This is an implementation of a compile-time counter.
///
/// Each counter has a **Name**: an user-given type T
/// The value of a counter is an integer of type unsigned
/// Denote its value as $T
/// The counter is zero-initialized
///
/// Usage:
///
/// fetch_add<T>() === ($T)++;
/// load<T>()      === ($T);
///
///
namespace cexpr_counter {
    namespace detail {
        template<class, int>                                         
        struct Flag {
            friend constexpr auto flag(Flag);
        };

        template<class T, int N>
        struct Writer {
            friend constexpr auto flag(Flag<T,N>) { 
                return true; 
            }
            static constexpr int value = N;
        };

        template<class T, int N = 0>
        constexpr int fadd_reader(float) {
            return Writer<T, N>::value;
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int fadd_reader(int) {
            return fadd_reader<T, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr int load_reader(float) {
            return N;
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int load_reader(int) {
            return load_reader<T, N + 1>(int{});
        }

        template <class T>
        struct name_guard {};
    };

    template<class T, int R = detail::fadd_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int fetch_add() {
        return R;
    }

    template <class T, int R = detail::load_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int load() {
        return R;
    }
};



///
/// This is an implementation of a compile-time list of variables.
///
///
/// Each list has a **Name**: an user-given type T
/// Denote its values as $T
/// The counter is zero-initialized
///
/// Usage:
///
/// Store<T, val>  === $T.append(val)
/// value<T>       === $T[-1]
/// value_at<T, M> === $T[M]
/// len<T>         === len($T)
/// pack_of<T>     === return $T as parameter pack of Argpass< Vals ... >;
///

namespace type_var {
    namespace detail {
        template <typename T>
        struct ConjureInstanceOf {T& operator*();};

        template <typename T>
        struct Wrap {
            using Res = T;
        };

        template <class T, class U>
        struct MonadicWrap {
            using Res = U;
        };

        template <class T> 
        struct MonadicWrap<T, replace_std::monost>{
            using Res = T;
        };
    

        template<class, int>                                         
        struct Flag {
            friend constexpr auto flag(Flag);
        };

        template<class T, int N, class Val>
        struct Writer {
            friend constexpr auto flag(Flag<T,N>) { 
                return *ConjureInstanceOf<Val>{}; 
            }
            static constexpr int value = N;
        };

        template<class T, class Val, int N = 0>
        constexpr int fadd_reader(float) {
            return Writer<T, N, Val>::value;
        }

        template<class T, class Val, int N = 0,
                class = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr int fadd_reader(int) {
            return fadd_reader<T, Val, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr auto load_reader(float) {
            return Wrap<replace_std::monost>{};
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto load_reader(int) {
            return MonadicWrap<U, typename decltype(load_reader<T, N + 1>(int{}))::Res>{};
        }

        template <class, int N = 0>
        constexpr auto at_reader(float) {
            return Wrap<replace_std::monost>{};
        }

        template<class T, int M, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto at_reader(int) {
            if constexpr (N == M) {
                return Wrap<U>{};
            } else {
                return at_reader<T, M, N+1>(int{});
            }
        }


        template <class, int N = 0>
        constexpr auto len_reader(float) {
            return N;
        }

        template<class T, int N = 0,
            class U = decltype(flag(Flag<T, N>{})),
            auto = [](){}>
        constexpr int len_reader(int) {
            return len_reader<T, N + 1>(int{});
        }

        template <class, int N = 0>
        constexpr auto pack_reader(float) {
            return replace_std::Argpass<>{};
        }

        template<class T, int N = 0,
                class U = decltype(flag(Flag<T, N>{})),
                auto = [](){}>
        constexpr auto pack_reader(int) {
            return decltype(pack_reader<T, N + 1>(int{}))::Merge(replace_std::Argpass<U>{});
        }

        template <class T>
        struct name_guard {};

    };

    template<class T, class Val, int R = detail::fadd_reader<detail::name_guard<T>, Val, 0>(int{})>
    struct Store { template <class U> struct ComputeIdx { static constexpr int idx = R - 1;}; };

    template <class T, class R = typename decltype(detail::load_reader<detail::name_guard<T>, 0>(int{}))::Res>
    using value = R;

    template <class T, int N = detail::len_reader<detail::name_guard<T>, 0>(int{})>
    constexpr int len = N;

    template <class T, unsigned M, class U = typename decltype(detail::at_reader<detail::name_guard<T>, M, 0>(int{}))::Res>
    using value_at = U;

    template <class T, class U = decltype(detail::pack_reader<detail::name_guard<T>, 0>(int{}))>
    using pack_of = U;
};

/// Useful types and macros :

#ifdef USEFUL_CEXPR_AUXS

namespace caux {


// #define STORE template struct type_var::Store
// #define CEXPR_DO template struct

    template <unsigned val_> struct Int { constexpr static unsigned val = val_; };
    template <class T> struct next {};

    struct Bool {};
    struct True : Bool {};
    struct False : Bool {};
};
#endif

///
/// Control Patterns
///

#ifdef CEXPR_IF

namespace cexpr_control {

/// Essentially, a constexpr if
///
/// if `cond`, 
/// forms func_wrap::call<F>, instantiating its side-effects
/// else       
/// forms func_wrap::call<G>, instantiating its side-effects
///
/// To use, implement your side-effects in func_wrap::call, leaving func_wrap a wrapper
///
template <bool cond, class func_wrap, class F, class G, auto v = [](){}> 
struct cond_subst;

template <bool, class func_wrap, class F, class G, auto v> 
struct cond_subst : 
    func_wrap:: template call<G> 
{};

template <class func_wrap, class F, class G, auto v>
struct cond_subst<true, func_wrap, F, G, v> : 
    func_wrap:: template call<F>
{};

///
/// Curried Store<name, T>
/// Use in cond_subst
///
template <class name>
struct setter {
    template <class T, auto = [](){}>
    struct call : type_var::Store<name, T> {};
};

#ifdef CEXPR_RECURSION


///
/// Primitive recursion :
/// for (unsigned N; N--; N > 0) {
///     func_wrap::call
/// }
///
template <class func_wrap, unsigned N, auto v = [](){}>

struct Recurse :
    func_wrap:: template call<[](){}>,  
    Recurse<func_wrap, N-1, v> 
{};

template <class func_wrap, auto v> 

struct Recurse<func_wrap, 0, v> 
{};

///
/// True recursion :
/// While 
///     ($stopcond == True) 
/// do 
///     func_wrap::call
///
template <class func_wrap, class stopcond, unsigned N = 0, bool sfinae_cond = true, auto v = [](){}>
struct While;

template <class func_wrap, class stopcond, unsigned N, bool sfinae_cond, auto v>
struct While :
    func_wrap:: template call<[](){}>,  
    While<
        func_wrap, 
        stopcond, 
        N+1, 
        replace_std::is_same_v<type_var::value<stopcond>, caux::True>, 
        v
    > 
{};

template <class func_wrap, class stopcond, unsigned N, auto v>
struct While<func_wrap, stopcond, N, false, v> {};

#endif
};
#endif

#include <iostream>
template <typename T>
T& type_var::detail::ConjureInstanceOf<T>::operator*() {
    std::cout << "error with " << typeid(T).name() << '\n';
    throw 1;
}