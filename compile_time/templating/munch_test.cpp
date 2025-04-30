#include "munch.hpp"



struct dog : TokenBase {};
struct cat : TokenBase {};
struct bark : TokenBase {};
struct meow : TokenBase {};

struct function {
    static void execute() {
        std::cout << "Function executed!\n";
    }
};


int main() {


    using MyTokens = TokenList<If, Then, Then, Name>;

    // printing remaining tokens is a bit tricky, so there is a specific function 
    // to check for remaining tokens in munch.hpp, used here
    // point is to show that we can execute based off of partial tokens and we dont need both input rule
    // and token list to be the same length, we can check for an aritrary number of tokens
    match_and_execute<
        AndThen<MatchOne<If>, MatchOne<Then>>,
        MyTokens,
        PrintRemainder<AndThen<MatchOne<If>, MatchOne<Then>>, MyTokens>
    >();

    using MyOtherTokens = TokenList<If, dog, Then, bark>;


    // all tokens match, so we can execute the action
    match_and_execute<
        IfThenRule<dog, bark>,
        MyOtherTokens,
        function
    >();


    return 0;
}