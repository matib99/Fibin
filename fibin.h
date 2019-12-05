#ifndef FIBIN_FIBIN_H
#define FIBIN_FIBIN_H

#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <cstring>



struct True {
    constexpr static bool val = true;
};
struct False {
    constexpr static bool val = false;
};

template <uint64_t v>
struct Fib {
    constexpr static int val = Fib<v-1>::val + Fib<v-2>::val;
};

template<>
struct Fib<0> {
    constexpr static uint64_t val = 0;
};

template<>
struct Fib<1> {
    constexpr static uint64_t val = 1;
};


/* The syntax contains eleven expression types:
 * Lit for literals, two types: Fib and True/False,
 * Var for variable names
 * Ref for variable references,
 * Let for joining Var with its value (which could be Lit but also Lambda),
 * Lambda terms for anonymous functions,
 * Invoke for function applications,
 * If for conditionals,
 * Eq for equal,
 * Inc1 for increasing an argument by Fib<1> = 1,
 * Inc1 for increasing an argument by Fib<10> = 55,
 * and Sum for addition at least 2 arguments.
 *  */

template <typename T>
struct Lit {} ;

constexpr uint32_t length(const char* nam) {

    uint32_t len = 0;
    for (size_t i = 0; nam[i] != '\0'; i++) {
        len++;
    }
    return len;
}

constexpr uint32_t Var(const char* nam) {

    size_t len = length(nam);
    if (len < 1 || len > 6)
        throw "Wrong length of variable name!";

    for (size_t i = 0; i < len; i++) {
        char c = nam[i];
        if ((c < '0' || c > '9') &&
            (c < 'a' || c > 'z') &&
            (c < 'A' || c > 'Z'))
            throw "Forbidden char sign in variable name!";
    }

    uint32_t value = 0;
    for (size_t j = 0; j < len; j++) {
        size_t i = len - 1 - j;
        char c = nam[i];
        //hash
        //there are 36 signs: 0,...,9, a,...,z
        //theirs ids are      0,...,9,10,...,35
        //there are max 6 positions, each with base 36^pos_id
        if (c >= 'a' && c <= 'z') {
            value *= 36;
            value = value + c - 'a' + 10;
        }
        else if (c >= 'A' && c <= 'Z') {//lowercase
            value *= 36;
            value = value + c - 'A' + 10;
        }
        else if (c >= '0' && c <= '9') {
            value *= 36;
            value = value + c - '0';
        }
    }
    return value;
}

template <uint32_t Var>
struct Ref {};

template <uint32_t Var, typename Value, typename Expr>
struct Let {};

template<uint32_t Var, typename Body>
struct Lambda {};

template<typename Fun, typename Arg>
struct Invoke {};

template<typename Cond, typename Then, typename Else>
struct If {};

template <typename T1, typename T2>
struct Eq {};

template<typename Arg>
struct Inc1 {};

template<typename Arg>
struct Inc10 {};

template <typename Arg1, typename Arg2, typename... Args>
struct Sum {};

template <uint64_t v>
struct Calc {
    constexpr static uint64_t val = v;
};



/* Environments are structures that map (variable) names to values.
 * Environments are paired with expressions to give meanings to free
 * variables. */

// EmptyEnv is the empty environment.
struct EmptyEnv;

// Bindings<Name,Value,Env> is a type than encodes the environment Env
// extended with a binding for Name => Value.
template<uint32_t Name, typename Value, typename Env>
struct Binding {};

// EnvLookup<Name,Env> :: result looks up the value of Name in Env.
template<uint32_t Name, typename Env>
struct FindVar {};

template<uint32_t Name>
struct FindVar<Name, EmptyEnv> {}; // Name not found.

// Found variable - it's value is Value and the "new" type is result
template<uint32_t Name, typename Value, typename Env>
struct FindVar<Name, Binding<Name, Value, Env> > {
    Value typedef result;
    Binding<Name, Value, Env> typedef env;
};

// Haven't found the variable - go recursively to the previous layer
// and check has it been binded to Environment there
template<uint32_t Name, uint32_t Name2, typename Value2, typename Env>
struct FindVar<Name, Binding<Name2, Value2, Env> > {
    typename FindVar<Name, Env>::result typedef result;
    typename FindVar<Name, Env>::env typedef env;
};

//
//  EVAL
//


// Eval<Exp,Env> :: result is the value of expression Exp in
// environment Env.
template<typename Exp, typename Env>
struct Eval {};

// Inv<Proc,Value> :: result is the value of applying Proc to Value.
template<typename Fun, typename Value, typename Env>
struct Inv {};

template<uint32_t Var, typename Body, typename EnvL>
struct LambdaEnv {};

template <uint64_t v, typename Env>
struct Eval<Calc<v>, Env> {
    Calc<v> typedef result;
};

// Literals evaluate to themselves:
template<typename T, typename Env>
struct Eval<Lit<T>, Env> {
    T typedef result;
};

//Lit<T>
template <uint64_t n, typename Env>
struct Eval<Lit<Fib<n>>, Env> {
    Fib<n> typedef result;
};

// Variable references are looked up in the current environment:
template<uint32_t name, typename Env>
struct Eval<Ref<name>, Env> {
    typename Eval<typename FindVar<name, Env>::result, Env>::result typedef result;
};


template<uint32_t name, typename Body, typename Env>
struct Eval<Lambda<name, Body>, Env> {
    typename Eval<Body, Binding<name, typename FindVar<0, Env>::result, Env>>::result typedef result;
};


template<typename Fun, typename Arg, typename Env>
struct Eval<Invoke<Fun, Arg>, Env> {
    typename Eval<Fun, Binding<0, Arg, Env>>::result typedef result;
};

template<uint32_t name, typename Arg, typename Env>
struct Eval<Invoke<Ref<name>, Arg>, Env> {
    typename Eval<typename FindVar<name, Env>::result, Binding<0, Arg, typename FindVar<name, Env>::env>>::result typedef result;
};


/*

template<uint32_t name, typename Body, typename Env>
struct Eval<Lambda<name, Body>, Env> {
    Lambda<name, Body> typedef result;
};

template<typename Fun, typename Arg, typename Env>
struct Eval<Invoke<Fun, Arg>, Env> {
    typename Inv<Fun, Env, Lit<typename Eval<Arg, Env>::result>>::result typedef result;
};

template<uint32_t name, typename Arg, typename Env>
struct Eval<Invoke<Ref<name>, Arg>, Env> {
    typename Inv<typename FindVar<name, Env>::result, typename FindVar<name, Env>::env, Lit<typename Eval<Arg, typename FindVar<name, Env>::env>::result>>::result typedef result;
};
 */

template<uint32_t name, typename Body, typename Env, typename Value>
struct Inv<Lambda<name, Body>, Env, Value> {
    typename Eval<Body, Binding<name, Value, Env>>::result typedef result;
};

// Branch true:
template<typename Then, typename Else, typename Env>
struct Eval<If<True, Then, Else>, Env> {
    typename Eval<Then, Env>::result typedef result;
};

// Branch false:
template<typename Then, typename Else, typename Env>
struct Eval<If<False, Then, Else>, Env> {
    typename Eval<Else, Env>::result typedef result;
};

// Evaluate the condition:
template<typename Cond, typename Then, typename Else, typename Env>
struct Eval<If<Cond, Then, Else>, Env> {
    typename Eval<If<typename Eval<Cond, Env>::result, Then, Else>, Env>::result
    typedef result;
};

template<typename T1, typename Env>
struct Eval<Eq<T1, T1>, Env> {
    True typedef result;
};

template<uint64_t v1, uint64_t v2, typename Env>
struct Eval<Eq< Lit<Fib<v1>>, Lit<Fib<v2>>>, Env> {
    False typedef result;
};

template<typename Env>
struct Eval<Eq< Lit<Fib<1>>, Lit<Fib<2>>>, Env> {
    True typedef result;
};

template<typename Env>
struct Eval<Eq< Lit<Fib<2>>, Lit<Fib<1>>>, Env> {
    True typedef result;
};


template<uint64_t v1, typename Env>
struct Eval<Eq< Lit<Fib<v1>>, Lit<Fib<v1>>>, Env> {
    True typedef result;
};

template<typename T1, typename T2, typename Env>
struct Eval<Eq<T1, T2>, Env> {
    typename Eval<Eq< Lit<typename Eval<T1, Env>::result>,
            Lit<typename Eval<T2, Env>::result>>,
            Env>:: result typedef result;
};

//Sum<Arg> |Args| = 2
template <typename Arg1, typename Arg2, typename Env>
struct Eval<Sum<Arg1, Arg2>, Env> {
    Calc<Eval<Arg1, Env>::result::val +
         Eval<Arg2, Env>::result::val> typedef result;
};

//Sum<Args> |Args| > 2
template <typename Arg1, typename Arg2, typename... Args, typename Env>
struct Eval<Sum<Arg1, Arg2, Args...>, Env> {
    typename Eval<Sum<Arg1, typename
    Eval<Sum<Arg2, Args...>, Env>::result>, Env>::result typedef result;
};

//Inc1<Arg>
template <typename Arg, typename Env>
struct Eval<Inc1<Arg>, Env> {
    typename Eval<Sum<Arg, Lit<Fib<1>>>, Env>::result typedef result;
};

//Inc1<Arg>
template <typename Arg, typename Env>
struct Eval<Inc10<Arg>, Env> {
    typename Eval<Sum<Arg, Lit<Fib<10>>>, Env>::result typedef result;
};

template <uint32_t name, typename Value, typename Expr, typename Env>
struct Eval<Let<name, Value, Expr>, Env> {
    typename Eval<Expr, Binding<name, Value, Env>>::result typedef result;
};

template <typename ValueType>
class Fibin {
public:

    // For all types except integral types:
    template<typename Expr, typename fake = ValueType,
            typename = typename std::enable_if_t<!std::is_integral<fake>::value>>
    constexpr static void eval() {
        std::cout << "Fibin doesn't support: " << typeid(ValueType).name() << "\n";
    }

    // For integral types only:
    template<typename Expr, typename fake = ValueType,
            typename = typename std::enable_if_t<std::is_integral<fake>::value>>
    constexpr static ValueType eval() {
        return (ValueType)Eval<Expr, EmptyEnv>::result::val;
    }
};

#endif //FIBIN_FIBIN_H