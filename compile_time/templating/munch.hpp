// munch.hpp — A type-level, compile-time token muncher for C++
// Author: Ryan Mangeno (2025)

/*
------------------------------------------------------------------------------
 How is this useful?

 This compile-time "muncher" system lets you define and match patterns in token
 streams entirely at compile time, similar to Rust's tt-muncher macro idiom
 
 In a game engine, for example, you might want to define a domain-specific
 scripting language for AI or cutscenes. For example:

     using Script = TokenList<If, EnemyNear, Then, Attack>;

 You can then build matching a matching rule like:

     using Condition = IfThen<If, EnemyNear, Then, Attack>;

 on face value, it may seem repetive, but ... 
 a condition could also simply satisfy a leading part of a rule
 this is useful for making a rule that can match multiple predicates
     
         using Predicate1 = IfThenRule<If, Water, Then, Drink>;
         using Predicate2 = IfThenRule<If, Water, Then, Pour>;
         using Rule1 = IfRule<Water>
         Sample Func -> { std::cout << "IsWater"; }
         then you can make a script the runs for both predicates determined at compile time

 Benefits:
 - Compile-time safety: invalid scripts fail to compile
 - No runtime parsing cost
 - Flexible enough to implement embedded DSLs for AI, shaders, configs, etc


------------------------------------------------------------------------------
*/

#ifndef MUNCH_HPP
#define MUNCH_HPP

#include <type_traits>
#include <tuple>
#include <iostream>



//------------------------------//
//      Token Definitions       //
//------------------------------//

// Base token type
struct TokenBase {};

// Keyword tokens
struct If   : TokenBase {};
struct Then : TokenBase {};
struct Else : TokenBase {};
struct End  : TokenBase {};

// Identifier tokens (simple types)
struct User : TokenBase {};
struct Name : TokenBase {};

// Token list: a pack of token types
template <typename... Ts>
struct TokenList {};

//------------------------------//
//       Token Utilities        //
//------------------------------//

// Head<List>::type == first token
template <typename List>
struct Head;

template <typename First, typename... Rest>
struct Head<TokenList<First, Rest...>> {
    using type = First;
};

// Tail<List>::type == everything but first
template <typename List>
struct Tail;

template <typename First, typename... Rest>
struct Tail<TokenList<First, Rest...>> {
    using type = TokenList<Rest...>;
};

// IsEmpty<List>::value == true if no tokens
template <typename List>
struct IsEmpty : std::false_type {};

template <>
struct IsEmpty<TokenList<>> : std::true_type {};

//------------------------------//
//         Rule System          //
//------------------------------//

// MatchOne: checks if first token == Expected
// Provides nested template `match<Input>` with `value` and `rest`
template <typename Expected>
struct MatchOne {
    template <typename Input>
    struct match {
        static constexpr bool value = std::is_same_v<Expected, typename Head<Input>::type>;
        using rest = typename Tail<Input>::type;
    };
};

// Done: always matches, leaves input unchanged
struct Done {
    template <typename Input>
    struct match {
        static constexpr bool value = true;
        using rest = Input;
    };
};

// AndThen<R1, R2>: sequence: match R1 then R2 on the remainder
template <typename R1, typename R2>
struct AndThen {
    template <typename Input>
    struct match {
        static constexpr bool first_ok = R1::template match<Input>::value;
        using mid = typename R1::template match<Input>::rest;
        static constexpr bool value = first_ok && R2::template match<mid>::value;
        using rest = typename R2::template match<mid>::rest;
    };
};


//------------------------------//
//       Muncher Engine         //
//------------------------------//

// muncher<Rule, Input>: asserts match and exposes `result`
template <typename Rule, typename Input>
struct muncher {
    using M = typename Rule::template match<Input>;
    static_assert(M::value, "Pattern does not match input tokens");
    using result = typename M::rest;
};


//------------------------------//
//        Utility Functions     //
//------------------------------//

template <typename Rule, typename Input>
constexpr bool match_only = Rule::template match<Input>::value;

template <typename Rule, typename Input, typename... Actions>
bool match_and_execute() {
    if constexpr (Rule::template match<Input>::value) {
        (Actions::execute(), ...);
        return true;
    } 
    else {
        std::cout << "Pattern did not match.\n";
        return false;
    }
}


template <typename Remainder, typename MyTokens>
struct PrintRemainder {
    static void execute() {
        using RemainderType = typename muncher<Remainder, MyTokens>::result;
        if constexpr (std::is_same_v<RemainderType, TokenList<Then, Name>>) {
            std::cout << "Remainder is [Then, Name].\n";
        }
        else {
            std::cout << "Remainder is not [Then, Name].\n";
        }
    }
};



//---------------------------//
//     FLEXIBLE RULES        //
//---------------------------//

// if <Cond> then <Act>
template <typename Cond, typename Act>
using IfThenRule = AndThen<
    MatchOne<If>,
    AndThen<
        MatchOne<Cond>,
        AndThen<
            MatchOne<Then>,
            MatchOne<Act>
        >
    >
>;

// for partial matching rules -> if <Cond> 
template <typename Cond>
using IfRule = AndThen<
    MatchOne<If>,
    MatchOne<Cond>
>;




#endif // MUNCH_HPP
