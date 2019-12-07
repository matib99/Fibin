#ifndef FIBIN_FIBIN_H
#define FIBIN_FIBIN_H

#include <iostream>

// representation of value True
struct True {
    constexpr static bool val = true;
};

// representation of value False
struct False {
    constexpr static bool val = false;
};


// Fib - general case
template<uint64_t v>
struct Fib {
    constexpr static uint64_t val = Fib<v - 1>::val + Fib<v - 2>::val;
};

// Fib - base case
template<>
struct Fib<0> {
    constexpr static uint64_t val = 0;
};

// Fib - base case
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

namespace details {

// count length of element of type "const char*"
    constexpr uint32_t length(const char *nam) {

        uint32_t len = 0;
        for (size_t i = 0; nam[i] != '\0'; i++) {
            len++;
        }
        return len;
    }


// add next sign to hash
    constexpr uint32_t hash(char c, uint32_t value) {

        //there are 36 signs: 0,...,9, a,...,z
        //theirs ids are      0,...,9,10,...,35
        //there are max 6 positions, each with base 36^(pos_id + 1)

        if (c >= 'a' && c <= 'z') {
            value *= 36;
            value = value + c - 'a' + 10;
        } else if (c >= 'A' && c <= 'Z') { //lowercase
            value *= 36;
            value = value + c - 'A' + 10;
        } else if (c >= '0' && c <= '9') {
            value *= 36;
            value = value + c - '0';
        }
        return value;
    }


// validates name
    constexpr void is_correct_name(size_t len, const char *name) {

        if (len < 1 || len > 6)
            throw "Wrong length of variable name!";

        for (size_t i = 0; i < len; i++) {
            char c = name[i];
            if ((c < '0' || c > '9') &&
                (c < 'a' || c > 'z') &&
                (c < 'A' || c > 'Z'))
                throw "Forbidden char sign in variable name!";
        }
    }
}

// validates name and count (and return) it's hash value
constexpr uint32_t Var(const char *name) {

    size_t len = details::length(name);
    details::is_correct_name(len, name);

    uint32_t value = 1;
    for (size_t j = 0; j < len; j++) {
        char c = name[len - 1 - j];
        value = details::hash(c, value);
    }
    return value;
}

// declarations

template<typename T>
struct Lit {
};

template<uint32_t Var>
struct Ref {
};

template<uint32_t Var, typename Value, typename Expr>
struct Let {
};

template<uint32_t Var, typename Body>
struct Lambda {
};

template<typename Fun, typename Arg>
struct Invoke {
};

template<typename Cond, typename Then, typename Else>
struct If {
};

template<typename T1, typename T2>
struct Eq {
};

template<typename Arg>
struct Inc1 {
};

template<typename Arg>
struct Inc10 {
};

template<typename Arg1, typename Arg2, typename... Args>
struct Sum {
};

//////////////////////////////////////////////////////////////////////


// class Fibin
template<typename ValueType>
class Fibin {


private:

//empty environment
    struct EmptyEnv;

// type of environment, which means Env extended with a binding for Name => Value.
    template<uint32_t Name, typename Value, typename Env>
    struct Binding {
    };

// find variable named Var in Env
    template<uint32_t Name, typename Env>
    struct FindVar {
    };

// variable not found in empty environmet
    template<uint32_t Name>
    struct FindVar<Name, EmptyEnv> {
    };

// found variable value and it's environment
    template<uint32_t Name, typename Value, typename Env>
    struct FindVar<Name, Binding<Name, Value, Env> > {
        Value typedef result;
        Binding<Name, Value, Env> typedef env;
    };

// haven't found the variable - go recursively to the previous layer
// and check has it been binded to Environment there
    template<uint32_t Name, uint32_t Name2, typename Value2, typename Env>
    struct FindVar<Name, Binding<Name2, Value2, Env> > {
        typename FindVar<Name, Env>::result typedef result;
        typename FindVar<Name, Env>::env typedef env;
    };

//
//  EVAL
//

    template<ValueType v>
    struct Calc {
        constexpr static ValueType val = v;
    };
    // Eval<Exp,Env>::result is the value of expression Exp in environment Env
    template<typename Exp, typename Env>
    struct Eval {
    };

//return its own value
    template<ValueType v, typename Env>
    struct Eval<Calc<v>, Env> {
        Calc<v> typedef result;
    };

// Lit - general, result is a type of Lit
    template<typename T, typename Env>
    struct Eval<Lit<T>, Env> {
        T typedef result;
    };

// Lit - specification for Fib
    template<ValueType n, typename Env>
    struct Eval<Lit<Fib<n>>, Env> {
        Fib<n> typedef result;
    };

// Ref - result is a value binded to "name"
    template<uint32_t name, typename Env>
    struct Eval<Ref<name>, Env> {
        typename Eval<typename FindVar<name, Env>::result, Env>::result typedef result;
    };

// Lambda - add to environment pair: name with value that was before in Env,
// but with name "0" (what is impossible in time of hash operation)
// and eval expression from Body in this new environment
    template<uint32_t name, typename Body, typename Env>
    struct Eval<Lambda<name, Body>, Env> {
        typename Eval<Body, Binding<name, typename FindVar<0, Env>::result,
                Env>>::result typedef result;
    };

// Invoke - bind function argument to name "0" (to save the info that this
// parameter has already a name) and eval Function
    template<typename Fun, typename Arg, typename Env>
    struct Eval<Invoke<Fun, Arg>, Env> {
        typename Eval<Fun, Binding<0, Arg, Env>>::result typedef result;
    };

// Invoke - specialisation when Fun isn't Lambda, but reference to Lambda
    template<uint32_t name, typename Arg, typename Env>
    struct Eval<Invoke<Ref<name>, Arg>, Env> {
        typename Eval<typename FindVar<name, Env>::result,
                Binding<0, Arg, typename FindVar<name, Env>::env>>::result typedef result;
    };

// If - check is condition True or False and evaluate appropriate branch
    template<typename Cond, typename Then, typename Else, typename Env>
    struct Eval<If<Cond, Then, Else>, Env> {
        typename Eval<If<typename Eval<Cond, Env>::result, Then, Else>, Env>::result
        typedef result;
    };

// If - branch True
    template<typename Then, typename Else, typename Env>
    struct Eval<If<True, Then, Else>, Env> {
        typename Eval<Then, Env>::result typedef result;
    };

// If - branch False
    template<typename Then, typename Else, typename Env>
    struct Eval<If<False, Then, Else>, Env> {
        typename Eval<Else, Env>::result typedef result;
    };

    template <ValueType n1, ValueType n2>
    struct NumEq
    {
        False typedef result;
    };

    template <ValueType n1>
    struct NumEq<n1, n1>
    {
        True typedef result;
    };


// Eq - check whether two parameters are equal
    template<typename T1, typename T2, typename Env>
    struct Eval<Eq<T1, T2>, Env> {
        typename Eval<Eq<Lit<typename Eval<T1, Env>::result>,
                Lit<typename Eval<T2, Env>::result>>,
                Env>::result typedef result;
    };

// Eq - specialisation when expressions are just the same
    template<typename T1, typename Env>
    struct Eval<Eq<T1, T1>, Env> {
        True typedef result;
    };

// Eq - specialisation when both expressions are Literals
    template<typename T1, typename T2, typename Env>
    struct Eval<Eq<Lit<T1>, Lit<T2>>, Env> {
        typename NumEq<(ValueType) T1::val, (ValueType) T2::val>::result typedef result;
    };


// Sum - when it's exactly two elements to add up
    template<typename Arg1, typename Arg2, typename Env>
    struct Eval<Sum<Arg1, Arg2>, Env> {
        Calc<Eval<Arg1, Env>::result::val +
             Eval<Arg2, Env>::result::val> typedef result;
    };

// Sum - when it's more than two elements to add up
    template<typename Arg1, typename Arg2, typename... Args, typename Env>
    struct Eval<Sum<Arg1, Arg2, Args...>, Env> {
        typename Eval<Sum<Arg1, typename
        Eval<Sum<Arg2, Args...>, Env>::result>, Env>::result typedef result;
    };

// Inc1 - sum Arg with Fib(1) = 1
    template<typename Arg, typename Env>
    struct Eval<Inc1<Arg>, Env> {
        typename Eval<Sum<Arg, Lit<Fib<1>>>, Env>::result typedef result;
    };

// Inc10 - sum Arg with Fib(10) = 55
    template<typename Arg, typename Env>
    struct Eval<Inc10<Arg>, Env> {
        typename Eval<Sum<Arg, Lit<Fib<10>>>, Env>::result typedef result;
    };

// Let - bind name with Value
// and evaluate the Expression (in new Env that contains this binding)
    template<uint32_t name, typename Value, typename Expr, typename Env>
    struct Eval<Let<name, Value, Expr>, Env> {
        typename Eval<Expr, Binding<name, Value, Env>>::result typedef result;
    };



public:

    // For all types except integral types:
    template<typename Expr, typename fake = ValueType,
            typename = typename std::enable_if_t<!std::is_integral<fake>::value>>
    constexpr static void eval() {
        std::cout << "Fibin doesn't support: " << typeid(ValueType).name()
                  << "\n";
    }

    // For integral types only:
    template<typename Expr, typename fake = ValueType,
            typename = typename std::enable_if_t<std::is_integral<fake>::value>>
    constexpr static ValueType eval() {
        return (ValueType) Eval<Expr, EmptyEnv>::result::val;
    }
};

#endif //FIBIN_FIBIN_H