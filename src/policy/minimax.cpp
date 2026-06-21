#include <utility>

#include "state.hpp"

#include "minimax.hpp"





/*============================================================

 * MiniMax — eval_ctx

 *

 * Negamax without pruning. Caller manages memory.

 *============================================================*/

int MiniMax::eval_ctx(

    State *state,

    int depth,

    GameHistory& history,

    int ply,

    SearchContext& ctx,

    const MMParams& p

){

    ctx.nodes++;

    if(ply > ctx.seldepth){

        ctx.seldepth = ply;

    }

    if(ctx.stop){

        return 0;

    }



    /* === Lazy move generation (sets game_state) === */

    if(state->legal_actions.empty() && state->game_state == UNKNOWN){

        state->get_legal_actions();

    }



    /* === Terminal / leaf checks === */



    // [ Hackathon TODO 3-1 ]

    // return the score for a winning terminal state

    // Hint: prefer faster wins by using ply.

    if (state->game_state == WIN) {

        // Current player wins. Subtract 'ply' to prioritize faster checkmates/captures.

        return P_MAX - ply;

    }



    if(state->game_state == DRAW){

        return 0;

    }



    /* === Repetition check (game-specific) === */

    int rep_score;

    if(state->check_repetition(history, rep_score)){

        return rep_score;

    }

    history.push(state->hash());



    if(depth <= 0){

        int score = state->evaluate(

            p.use_kp_eval, p.use_eval_mobility, &history

        );

        history.pop(state->hash());

        return score;

    }



    /* === Negamax loop === */

    int best_score = M_MAX;



    for(auto& action : state->legal_actions){

        // [ Hackathon TODO 3-2 ]

        // create the child state after applying action

        State* next = state->next_state(action);



        bool same = next->same_player_as_parent();



        // [Hackathon TODO 3-3]

        // search the child one level deeper

        int raw_score = eval_ctx(next, depth - 1, history, ply + 1, ctx, p);



        // [Hackathon TODO 3-4]

        // convert raw to the current player's perspective.

        int score = same ? raw_score : -raw_score;



        delete next;



        // [ Hackathon TODO 3-5 ]

        // update best_score if this child is better.

        if (score > best_score) {

            best_score = score;

        }



    }



    history.pop(state->hash());

    return best_score;

}





/*============================================================

 * MiniMax — search

 *

 * Iterate legal moves, call eval_ctx, return SearchResult.

 *============================================================*/

SearchResult MiniMax::search(

    State *state,

    int depth,

    GameHistory& history,

    SearchContext& ctx

){

    ctx.reset();

    MMParams p = MMParams::from_map(ctx.params);

    SearchResult result;

    result.depth = depth;



    if(!state->legal_actions.size()){

        state->get_legal_actions();

    }





    int best_score = M_MAX - 10;

    int move_index = 0;

    int total_moves = (int)state->legal_actions.size();



    for(auto& action : state->legal_actions){

        /* [ Hackathon TODO 4-1 ]

         * search this move like TODO 3, but starting from the root */

        // Generate the successor state for this root action

        State* next = state->next_state(action);

        bool same = next->same_player_as_parent();



        // Call the recursive search function with depth decremented and ply initialized to 1

        int raw_score = eval_ctx(next, depth - 1, history, 1, ctx, p);

       

        // Convert the raw score to the root player's perspective using Negamax rules

        int score = same ? raw_score : -raw_score;

       

        // Free the dynamically allocated state to prevent memory leaks

        delete next;



        if(score > best_score){

            // [ Hackathon TODO 4-2 ]

            // keep this move if it is the best so far

            best_score = score;

            result.best_move = action;

            result.score = best_score;



            if(p.report_partial && ctx.on_root_update){

                ctx.on_root_update({result.best_move, best_score, depth, move_index + 1, total_moves});

            }

        }

        move_index++;

    }



    // Pop the root state hash from history after completing the entire search loop

    history.pop(state->hash());



    // [ Hackathon TODO 4-3 ]

    // update result and return

    result.nodes = ctx.nodes;

    result.seldepth = ctx.seldepth;



    return result;

}





/*============================================================

 * MiniMax — default_params / param_defs

 *============================================================*/

ParamMap MiniMax::default_params(){

    return {

        {"UseKPEval", "true"},

        {"UseEvalMobility", "true"},

        {"ReportPartial", "true"},

    };

}



std::vector<ParamDef> MiniMax::param_defs(){

    return {

        {"UseKPEval", ParamDef::CHECK, "true"},

        {"UseEvalMobility", ParamDef::CHECK, "true"},

        {"ReportPartial", ParamDef::CHECK, "true"},

    };

}