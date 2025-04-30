// =======================================
// Example: Compile-Time AI Behavior Script Parser
// Purpose: Match and execute NPC scripts like:
//          "If EnemyNear Then Attack"
// Using:   The muncher system (see munch.hpp)
// =======================================

#include "munch.hpp"


/*
    Note -> For the muncher, the user has to create tokens and define the rules 
            for matching them. The muncher will then check if the tokens match the
            rules and execute the corresponding actions
*/



//---------------------------//
// GAME EXAMPLE TOKEN SETUP  //
//---------------------------//

// conditions and actions in AI scripts
struct EnemyNear : TokenBase {};
struct HealthLow : TokenBase {};
struct AllyDown : TokenBase {};
struct Attack : TokenBase {};
struct Retreat : TokenBase {};
struct Revive : TokenBase {};

//---------------------------//
//     GAME AI ACTIONS      //
//---------------------------//

struct DoAttack {
    static void execute() {
        std::cout << "[AI] Attacking enemy!\n";
    }
};

struct DoRetreat {
    static void execute() {
        std::cout << "[AI] Retreating to safety!\n";
    }
};

struct DoRevive {
    static void execute() {
        std::cout << "[AI] Reviving ally!\n";
    }
};



//---------------------------//
//   GAME OBJECT & STATE    //
//---------------------------//

// fyi -> this is not actually how I make game objects :))

struct GameObject {
    bool enemyNear = false;
    bool healthLow = false;
    bool allyDown = false;

    void updateState() {
        // for simplicity, im toggling conditions manually in this example, this GameObject is not necesary to
        // the example, but to make the example more realistic, why not
        enemyNear = (rand() % 2 == 0);
        healthLow = (rand() % 2 == 0);
        allyDown = (rand() % 2 == 0);
    }

    void render() {
        // its there ... in spirit
    }
};





// this run_script function an definitely be improved, but for now, this is a simple way to evaluate the script

/*

Script is a type like TokenList<If, EnemyNear, Then, Attack>

Each if constexpr checks at compile time whether Script matches a rule like IfThenRule<EnemyNear, Attack>

Only the matching branch is compiled into the final binary — all other branches are discarded by the compiler

This means:

No runtime string parsing

No dynamic branching or runtime matching

Any syntax or token errors are compile-time errors

*/

template <typename Script>
void run_script() {
    if constexpr (match_only<IfThenRule<EnemyNear, Attack>, Script>) {
        DoAttack::execute();
    }
    if constexpr (match_only<IfThenRule<HealthLow, Retreat>, Script>) {
        DoRetreat::execute();
    }
    if constexpr (match_only<IfThenRule<AllyDown, Revive>, Script>) {
        DoRevive::execute();
    }
    if constexpr (match_only<IfRule<EnemyNear>, Script>) {
        std::cout << "[AI] ENEMY NEAR!\n";
    }
    if constexpr (match_only<IfRule<HealthLow>, Script>) {
        std::cout << "[AI] HEALTH LOW!\n";
    }
}

#include <chrono>
#include <thread>


int main() {
    const std::chrono::seconds timeLimit(5);
    
    auto startTime = std::chrono::steady_clock::now();


    GameObject ai;


    // in practice, this could be defined somewhere like npc_behavior.hpp
    using Script_A = TokenList<If, EnemyNear, Then, Attack>;
    using Script_B = TokenList<If, HealthLow, Then, Retreat>;
    using Script_C = TokenList<If, AllyDown, Then, Revive>;
    using Script_D = TokenList<If, EnemyNear, Then, Retreat>;
    using Script_E = TokenList<If, HealthLow, Then, Revive>;

    while (true) {
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime);

        if (elapsedTime >= timeLimit) {
            std::cout << "Time limit reached, stopping game loop.\n";
            break;  
        }

        // update AI state 
        ai.updateState();

        
        run_script<Script_B>();
        run_script<Script_C>();
        run_script<Script_D>();
        run_script<Script_E>();
        run_script<Script_A>();


        ai.render();



        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "Game update completed. Time elapsed: " << elapsedTime.count() << " seconds.\n";
    }

    return 0;
}