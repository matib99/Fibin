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

    template <typename Arg1, typename Arg2, typename... Args>
    struct Sum {};

    template <const char* VariableName>
    struct Var
    {
        const char* name = VariableName;
    };

    template <typename T> // tu jakiś błąd trzeba jeśli damy coś innego od Var, albo to jakoś lepiej rozwiązać
    struct Ref{};

    template <const char* Name>
    struct Ref<Var<Name>>{};


    template <typename Variable, typename Value, typename Expression> // jak wyżej
    struct Let {};

    template <const char* Name, typename Value, typename Expression>
    struct Let<Var<Name>, Value, Expression> {};

    template<class C>
    static void print() {
        std::cout << C::val << "\n";
    }



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

    //Sum<Arg> |Args| = 2
    template <typename Arg1, typename Arg2, typename Env>
    struct Eval<Sum<Arg1, Arg2>, Env>
    {
        const static ValueType val = Eval<Arg1, Env>::val + Eval<Arg2, Env>::val;
    };

    //Sum<Args> |Args| > 2
    template <typename Arg1, typename Arg2, , typename... Args, typename Env>
    struct Eval<Sum<Arg1, Arg2, Args...>, Env>
    {
        const static ValueType val = Eval<Arg1, Env>::val + Eval<Sum<Arg2, Args...>, Env>::val;
    };

    //prawdopodobnie trzeba będzie zrobić jakiś error kiedy argumentów w sum będzie mniej niż 2

    //Eq<T1,T2>
    template <typename T1, typename T2, typename Env>
    struct Eval<Eq<T1, T2>, Env>
    {
        const static bool val = Eval<T1, Env>::val == Eval<T2, Env>::val;
    };

    template <const char* Name, typename Env>
    struct Eval<Ref<Var<Name>>, Env>
    {
        // tutaj pewien problem, bo to może zwracać też typ. więc nie wiem. Trzeba bedzie typedefować val,
        // ale z drugiej strony co wtedy robić kiedy wartościąjest po prostu

        // możliwe, że trzeba będzie to wszystko przebudować :c

        // albo jawnie zrobić 2 rodzaje jakby Variabli gdzie jedna jest funkcjąa druga wyrażeniem, które oblicza siędo wartości.
        // i wtedy, w invoke np dać 2 opcje, gdy pierwszy argument to Ref<Var> lub gdy pierwszy argument to Lambda
        // (ale to by było brzydsze)
    };

    template<const char* Name, typename Value, typename Expression, typename Env>
    struct Eval<Let<Var<Name>, Value, Expression>, Env>
    {
        // wykonujemy Expression ale w środowisku które zawiera nową zmienną
        const static auto val = Eval<Expression, Binding<Name, Value, Env>>::val;
    };


    /*
     * TODO:
     * - IF
     * - Lambda
     * - Invoke
     * - pomyśleć co z tymi Refami jak to rozwiązać
     * - napisać eval(), co nie powinno byćproblemem, skoro Eval robi wszystko
     *      (Eval jest po to, żeby ukryć Env których nie chcą w specyfikacji)
     * - Testy...
    */


};

#endif //FIBIN_FIBIN_H
