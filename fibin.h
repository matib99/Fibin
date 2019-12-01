#ifndef FIBIN_FIBIN_H
#define FIBIN_FIBIN_H

#include <iostream>
#include <array>
#include <cstring>
#include <cassert>



template <typename ValueType>
class Fibin {

public:

    struct True
    {
        const static bool val = true;
    };
    struct False
    {
        const static bool val = false;
    };

    template<ValueType v, typename = void>
    struct Fib {
        const static ValueType val = Fib<v-1>::val + Fib<v-2>::val;
    };

    template<typename F>
    struct Fib<0, F> {
        const static ValueType val = 0;
    };

    template<typename F>
    struct Fib<1, F> {
        const static ValueType val = 1;
    };

    template <typename T>
    struct Lit {};

    template <ValueType v>
    struct Lit<Fib<v>> {};

    template<typename Arg>
    struct Inc1 {};

    template<typename Arg>
    struct Inc10 {};

    template <typename T1, typename T2>
    struct Eq {};


private:

    //puste środowisko, bez żadnej zmiennej
    struct EmptyEnv {};

    //nadpisanie [Env] dodając do niej zmienną [Name] o wartości [Value]
    template <const char* Name, typename Value, typename Env>
    struct Binding {};

    //metafunkcja szukająca zmiennej w środowisku
    template <const char* Name, typename Env>
    struct FindVar {};

    // szukanie w pustym środowisku - błąd nie znaleziono zmiennej (jakiś assert czy coś)
    template <const char* Name>
    struct FindVar <Name, EmptyEnv> {};

    //znaleźliśmy zmienną o
    template <const char* Name, typename Value, typename Env>
    struct FindVar <Name, Binding<Name, Value, Env> >
    {
        Value typedef result;
    };

    //nie znaleźliśmy zmiennej, przechodzimy rekurencyjnie warstwę wyżej
    template <const char* Name, const char* Name2, typename Value2, typename Env>
    struct FindVar <Name, Binding<Name2, Value2, Env> >
    {
        typename FindVar<Name, Env>::result typedef result;
    };

    //
    //  EVAL
    //

    //liczy wyrażenie Exp w środowisku Env
    template <typename Exp, typename Env>
    struct Eval {};

    //Lit<T>
    template <typename T, typename Env>
    struct Eval<Lit<T>, Env>
    {
        const static auto val = T::val;
    };

    //Eq<T1,T2>
    template <typename T1, typename T2, typename Env>
    struct Eval<Eq<T1, T2>, Env>
    {
        const static bool val = Eval<T1, Env>::val == Eval<T2, Env>::val;
    };

    //Inc1<Arg>
    template <typename Arg, typename Env>
    struct Eval<Inc1<Arg>, Env>
    {
        const static ValueType val = Eval<Arg, Env>::val + Eval<Lit<Fib<1>>, Env>::val;
    };

    //Inc1<Arg>
    template <typename Arg, typename Env>
    struct Eval<Inc10<Arg>, Env>
    {
        const static ValueType val = Eval<Arg, Env>::val + Eval<Lit<Fib<10>>, Env>::val;
    };


    template<class C>
    static void print() {
        std::cout << C::val << "\n";
    }
};

#endif //FIBIN_FIBIN_H
