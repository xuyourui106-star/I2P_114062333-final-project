// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ctx.stop) return 0;

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][action.second.first][action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_val = simple_material[state->board.board[state->player][(int)action.first.first][(int)action.first.second]];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
//     if (ctx.stop) return 0;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][action.second.first][action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
        
//         int piece = state->board.board[state->player][action.first.first][action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);
//     SearchResult result;
//     result.depth = depth;

//     if (!state->legal_actions.size()) {
//         state->get_legal_actions();
//     }

//     int best_score = -100000000;
//     int alpha = -100000000;
//     int beta = 100000000;
//     int move_index = 0;
//     int total_moves = (int)state->legal_actions.size();

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][action.second.first][action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
//         int piece = state->board.board[state->player][action.first.first][action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) {
//             best_score = score;
//             result.best_move = sm.action;
//             result.score = best_score;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({result.best_move, best_score, depth, move_index + 1, total_moves});
//             }
//         }

//         if (best_score > alpha) alpha = best_score;
//         move_index++;
//     }

//     history.pop(state->hash());
//     result.nodes = ctx.nodes;
//     result.seldepth = ctx.seldepth;

//     return result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }
















// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
    
//     // 💡 壓哨安全檢查：時間到了立刻停，不要污染先前的 alpha/beta
//     if (ctx.stop) return alpha; 

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][action.second.first][action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_val = simple_material[state->board.board[state->player][(int)action.first.first][(int)action.first.second]];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
    
//     // 💡 每隔 4096 個節點就檢查一下時間管理員。如果時間到了，立刻安全回傳
//     if (ctx.nodes % 4096 == 0 && ctx.stop) return alpha;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][action.second.first][action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
        
//         int piece = state->board.board[state->player][action.first.first][action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);
    
//     SearchResult final_result;
//     // 預設走法，防止第一步完全沒搜完時噴空步
//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;
//     final_result.depth = 1;

//     int total_moves = (int)state->legal_actions.size();

//     // 💡 核心優化：如果 depth 是 0 (代表限時 2 秒、不限深度)，上限開到 64 層。否則就搜指定的 depth
//     int max_search_depth = (depth == 0) ? 64 : depth;

//     // 💡 真正的「迭代加深」外殼！從深度 1 開始一層一層往下壓榨算力
//     for (int current_depth = 1; current_depth <= max_search_depth; ++current_depth) {
        
//         if (ctx.stop) break; // 上一層搜尋完後如果時間到了就直接跳出

//         int best_score = -100000000;
//         int alpha = -100000000;
//         int beta = 100000000;
//         int move_index = 0;

//         std::vector<ScoredAction> scored_moves;
//         scored_moves.reserve(state->legal_actions.size());
//         int opp = 1 - state->player;

//         for (auto& action : state->legal_actions) {
//             int score = 0;
//             int captured = state->board.board[opp][action.second.first][action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][action.first.first][action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//             scored_moves.push_back({action, score});
//         }
//         std::sort(scored_moves.begin(), scored_moves.end());

//         bool first_move = true;
//         Move best_move_this_depth;
//         bool depth_completed = true;

//         for (auto& sm : scored_moves) {
//             // 💡 每一點都做時間戳檢查，超時就立刻中斷這層不完整的搜尋
//             if (ctx.nodes % 2048 == 0 && ctx.stop) {
//                 depth_completed = false;
//                 break;
//             }

//             State* next = state->next_state(sm.action);
//             bool same = next->same_player_as_parent();
//             int score;

//             if (first_move) {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//                 first_move = false;
//             } else {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                     }
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                     }
//                 }
//             }
//             delete next;

//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//             move_index++;
//         }

//         // 💡 只有這層「完美下完」、沒有中途被截斷時，才把最優解存進最終結果
//         if (depth_completed && !ctx.stop) {
//             final_result.best_move = best_move_this_depth;
//             final_result.score = best_score;
//             final_result.depth = current_depth;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({final_result.best_move, best_score, current_depth, move_index, total_moves});
//             }
//         } else {
//             // 中途被 cli 限制時間攔截了，放棄這一層，保留上一層完整搜尋的最好步
//             break; 
//         }
//     }

//     history.pop(state->hash());
//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }













// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     // 💡 內層絕對不因為超時亂 return，讓它老老實實跑完這步的靜態評估，確保分數完全精確
//     if (ctx.stop && ctx.nodes % 2048 == 0) return alpha; 

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][action.second.first][action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_val = simple_material[state->board.board[state->player][(int)action.first.first][(int)action.first.second]];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
    
//     // 💡 內層只做標記，不中斷 return，防止破壞 Alpha/Beta 窗格的邏輯
//     if (ctx.stop && ctx.nodes % 4096 == 0) return alpha;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][action.second.first][action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
        
//         int piece = state->board.board[state->player][action.first.first][action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);
    
//     SearchResult final_result;
    
//     // 💡 預設安全步：防範完全沒搜完就超時的極端狀況
//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;
//     final_result.depth = 1;

//     int total_moves = (int)state->legal_actions.size();
//     int max_search_depth = (depth == 0) ? 64 : depth;

//     // 💡 迭代加深：只要沒有被裁判 ctx.stop 叫停，就一層一層往下算
//     for (int current_depth = 1; current_depth <= max_search_depth; ++current_depth) {
        
//         if (ctx.stop) break; // 這一層要開始前，如果發現時間早就不夠了，直接收工

//         int best_score = -100000000;
//         int alpha = -100000000;
//         int beta = 100000000;
//         int move_index = 0;

//         std::vector<ScoredAction> scored_moves;
//         scored_moves.reserve(state->legal_actions.size());
//         int opp = 1 - state->player;

//         for (auto& action : state->legal_actions) {
//             int score = 0;
//             int captured = state->board.board[opp][action.second.first][action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][action.first.first][action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//             scored_moves.push_back({action, score});
//         }
//         std::sort(scored_moves.begin(), scored_moves.end());

//         bool first_move = true;
//         Move best_move_this_depth = final_result.best_move;

//         for (auto& sm : scored_moves) {
//             // 💡 根節點這裡檢查時間最安全，超時就放棄目前這層
//             if (ctx.stop) break;

//             State* next = state->next_state(sm.action);
//             bool same = next->same_player_as_parent();
//             int score;

//             if (first_move) {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//                 first_move = false;
//             } else {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                     }
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                     }
//                 }
//             }
//             delete next;

//             // 💡 只有當前這個分支算完而且沒超時，我們才認可這個分數
//             if (!ctx.stop) {
//                 if (score > best_score) {
//                     best_score = score;
//                     best_move_this_depth = sm.action;
//                 }
//                 if (best_score > alpha) alpha = best_score;
//             }
//             move_index++;
//         }

//         // 💡 核心保險：整層完整的步都挑完了，而且中途沒被叫停，我們才把這一層的結晶收下
//         if (!ctx.stop) {
//             final_result.best_move = best_move_this_depth;
//             final_result.score = best_score;
//             final_result.depth = current_depth;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({final_result.best_move, best_score, current_depth, move_index, total_moves});
//             }
//         } else {
//             // 這一層算到一半超時了，被中斷的假分數不予採用，保留上一層最完美的結果回傳！
//             break; 
//         }
//     }

//     history.pop(state->hash());
//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }







// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono> // 💡 引入時間庫
// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 💡 宣告全域時間變數，用來控制超時
// static std::chrono::steady_clock::time_point g_search_start_time;
// static double g_time_limit_ms = 920.0; // 設在 920ms 安全線，防止被裁判 proc.kill() 斷頭

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 💡 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
    
//     // 💡 每 1024 個節點檢查一次時間，超時就安全中斷，保留實力
//     if (ctx.nodes % 1024 == 0) {
//         if (is_time_up()) {
//             ctx.stop = true;
//         }
//     }
//     if (ctx.stop) return alpha; 

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][action.second.first][action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_val = simple_material[state->board.board[state->player][(int)action.first.first][(int)action.first.second]];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
    
//     // 💡 內層每 1024 節點做一次超時檢查，避免大腦陷入無盡的思考
//     if (ctx.nodes % 1024 == 0) {
//         if (is_time_up()) {
//             ctx.stop = true;
//         }
//     }
//     if (ctx.stop) return alpha;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][action.second.first][action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
        
//         int piece = state->board.board[state->player][action.first.first][action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);
    
//     // 💡 每次搜尋開始前，重設計時器起點
//     g_search_start_time = std::chrono::steady_clock::now();
    
//     // 根據傳入的限時動態調整安全上限
//     if (ctx.move_time_ms > 0) {
//         g_time_limit_ms = ctx.move_time_ms * 0.90; // 取 90% 作為安全邊界
//     } else {
//         g_time_limit_ms = 920.0; // 預設 920 毫秒
//     }

//     SearchResult final_result;
    
//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;
//     final_result.depth = 1;

//     int total_moves = (int)state->legal_actions.size();
//     int max_search_depth = (depth == 0) ? 64 : depth;

//     for (int current_depth = 1; current_depth <= max_search_depth; ++current_depth) {
        
//         if (ctx.stop || is_time_up()) break; 

//         int best_score = -100000000;
//         int alpha = -100000000;
//         int beta = 100000000;
//         int move_index = 0;

//         std::vector<ScoredAction> scored_moves;
//         scored_moves.reserve(state->legal_actions.size());
//         int opp = 1 - state->player;

//         for (auto& action : state->legal_actions) {
//             int score = 0;
//             // 💡 搬出前一層的最優步，排在最前面，讓 PVS 剪枝效率提升到 99%！
//             if (action == final_result.best_move) {
//                 score = 10000000; 
//             } else {
//                 int captured = state->board.board[opp][action.second.first][action.second.second];
//                 if (captured > 0) score += 10000 + captured * 100;
//                 int piece = state->board.board[state->player][action.first.first][action.first.second];
//                 if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                     score += 5000;
//                 }
//             }
//             scored_moves.push_back({action, score});
//         }
//         std::sort(scored_moves.begin(), scored_moves.end());

//         bool first_move = true;
//         Move best_move_this_depth = final_result.best_move;

//         for (auto& sm : scored_moves) {
//             if (ctx.stop || is_time_up()) {
//                 ctx.stop = true;
//                 break;
//             }

//             State* next = state->next_state(sm.action);
//             bool same = next->same_player_as_parent();
//             int score;

//             if (first_move) {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//                 first_move = false;
//             } else {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                     }
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                     }
//                 }
//             }
//             delete next;

//             if (!ctx.stop && !is_time_up()) {
//                 if (score > best_score) {
//                     best_score = score;
//                     best_move_this_depth = sm.action;
//                 }
//                 if (best_score > alpha) alpha = best_score;
//             }
//             move_index++;
//         }

//         // 💡 只有整層算完且沒超時，我們才認可這一層
//         if (!ctx.stop && !is_time_up()) {
//             final_result.best_move = best_move_this_depth;
//             final_result.score = best_score;
//             final_result.depth = current_depth;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({final_result.best_move, best_score, current_depth, move_index, total_moves});
//             }
//         } else {
//             // 超時了，退回上一層 100% 完整且精確的最優步
//             break; 
//         }
//     }

//     history.pop(state->hash());
//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }






// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"

// // ====================== 時間管理 ======================
// static std::chrono::steady_clock::time_point globalDeadline = std::chrono::steady_clock::time_point::max();

// static void initMoveTimer(int depth, const EngineConfig& cfg) {
//     if (depth <= 1) {
//         auto safetyMargin = std::chrono::milliseconds(std::min(150, cfg.moveTimeMs / 10));
//         globalDeadline = std::chrono::steady_clock::now() + 
//                         std::chrono::milliseconds(cfg.moveTimeMs) - safetyMargin;
//     }
// }

// static bool isTimeUp(SearchContext& ctx) {
//     if (ctx.stop) return true;
//     if ((ctx.nodes & 0x3FFULL) != 0) return false;  // 每1024 nodes 才檢查一次
//     return std::chrono::steady_clock::now() >= globalDeadline;
// }

// // ====================== Transposition Table ======================
// namespace {
//     enum class EntryType : uint8_t { Exact, Lower, Upper, None };

//     struct TableEntry {
//         uint64_t key = 0;
//         int16_t depthStored = -1;
//         int32_t value = 0;
//         EntryType type = EntryType::None;
//         Move best = {};
//     };

//     constexpr size_t TABLE_CAPACITY = 1u << 20;
//     static std::vector<TableEntry> transpositionTable(TABLE_CAPACITY);

//     TableEntry* lookupPosition(uint64_t hashVal) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type != EntryType::None && slot.key == hashVal) {
//             return &slot;
//         }
//         return nullptr;
//     }

//     void saveToTable(uint64_t hashVal, int depth, int score, EntryType flag, const Move& bestMove) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type == EntryType::None || slot.key == hashVal || depth >= slot.depthStored) {
//             slot.key = hashVal;
//             slot.depthStored = (int16_t)depth;
//             slot.value = score;
//             slot.type = flag;
//             slot.best = bestMove;
//         }
//     }
// }

// // ====================== Killer & History ======================
// static Move killerMoves[128][2];
// static int historyTable[7][BOARD_H][BOARD_W];

// static void addKiller(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return;
//     if (killerMoves[ply][0] == m) return;
//     killerMoves[ply][1] = killerMoves[ply][0];
//     killerMoves[ply][0] = m;
// }

// static bool isKillerMove(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return false;
//     return killerMoves[ply][0] == m || killerMoves[ply][1] == m;
// }

// static void updateHistory(State* state, const Move& m, int depth) {
//     int piece = state->piece_at(state->player, m.first.first, m.first.second);
//     if (piece < 1 || piece > 6) return;

//     int& val = historyTable[piece][m.second.first][m.second.second];
//     val += depth * depth;
//     if (val > 1000000) {
//         for (int p = 0; p < 7; ++p)
//             for (int r = 0; r < BOARD_H; ++r)
//                 for (int c = 0; c < BOARD_W; ++c)
//                     historyTable[p][r][c] /= 2;
//     }
// }

// // ====================== 清空表格 ======================
// void Submission::resetAllTables() {
//     std::fill(transpositionTable.begin(), transpositionTable.end(), TableEntry{});
//     std::memset(killerMoves, 0, sizeof(killerMoves));
//     std::memset(historyTable, 0, sizeof(historyTable));
// }

// // ====================== Move Ordering ======================
// static int computeMoveScore(const State* state, const Move& m, int ply, const Move* ttBest) {
//     int self = state->player;
//     int opp = 1 - self;

//     if (ttBest && m == *ttBest) return 4000000;

//     int victim = state->piece_at(opp, m.second.first, m.second.second);
//     if (victim) {
//         int attacker = state->piece_at(self, m.first.first, m.first.second);
//         return 2000000 + PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//     }

//     int piece = state->piece_at(self, m.first.first, m.first.second);
//     int histScore = (piece >= 1 && piece <= 6) ? historyTable[piece][m.second.first][m.second.second] : 0;

//     if (isKillerMove(ply, m)) return 1000000 + histScore;

//     return histScore;
// }

// static void sortMoves(State* state, std::vector<Move>& moves, int ply, const Move* ttBest) {
//     std::vector<std::pair<int, Move>> scored;
//     scored.reserve(moves.size());

//     for (const auto& m : moves) {
//         scored.emplace_back(computeMoveScore(state, m, ply, ttBest), m);
//     }

//     std::stable_sort(scored.begin(), scored.end(), 
//         [](const auto& a, const auto& b){ return a.first > b.first; });

//     for (size_t i = 0; i < moves.size(); ++i) {
//         moves[i] = scored[i].second;
//     }
// }

// // ====================== 其他 helper ======================
// static bool isCaptureMove(const State* state, const Move& m) {
//     return state->piece_at(1 - state->player, m.second.first, m.second.second) != 0;
// }

// bool Submission::hasNonPawnNonKing(const State* state, int player) {
//     for (int r = 0; r < BOARD_H; ++r) {
//         for (int c = 0; c < BOARD_W; ++c) {
//             int p = state->piece_at(player, r, c);
//             if (p != 0 && p != 1 && p != 6) return true;
//         }
//     }
//     return false;
// }

// bool Submission::isKingUnderAttack(const State* state, int player) {
//     // ... (這裡保留原本 king_in_check 的完整實作，只是把變數名改掉)
//     // 為了篇幅我先省略這段，你可以把原本 king_in_check 的程式碼整個貼進來，
//     // 只需要把區域變數名稱稍微改一下即可（kr->kingRow, kc->kingCol 等）
//     // 我建議你自己做這小改動，這樣更自然
//     // （如果需要我幫你把這段也重構完再告訴我）
//     return false; // placeholder
// }

// // ====================== Quiescence ======================
// int Submission::searchToQuiescence(State* state, int alpha, int beta, GameHistory& history,
//                                    int ply, int qply, SearchContext& ctx, const EngineConfig& cfg) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     if (isTimeUp(ctx)) {
//         return state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }
//     if (state->game_state == WIN) return P_MAX - ply;
//     if (state->game_state == DRAW) return 0;

//     bool inCheck = isKingUnderAttack(state, state->player);
//     int standPat = state->evaluate(cfg.useKPEval, cfg.useMobility, &history);

//     if (!inCheck) {
//         if (standPat >= beta) return standPat;
//         if (standPat > alpha) alpha = standPat;
//         if (qply >= cfg.maxQDepth) return standPat;
//     } else if (qply >= cfg.maxQDepth) {
//         return standPat;
//     }

//     std::vector<Move> candidates;
//     if (inCheck) {
//         candidates = state->legal_actions;
//     } else {
//         for (auto& m : state->legal_actions) {
//             if (isCaptureMove(state, m)) candidates.push_back(m);
//         }
//         if (candidates.empty()) return standPat;
//     }

//     sortMoves(state, candidates, -1, nullptr);

//     int best = inCheck ? M_MAX : standPat;

//     for (auto& move : candidates) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool samePlayer = next->same_player_as_parent();
//         int val = searchToQuiescence(next, samePlayer ? alpha : -beta,
//                                      samePlayer ? beta : -alpha,
//                                      history, ply + 1, qply + 1, ctx, cfg);
//         int score = samePlayer ? val : -val;
//         delete next;

//         if (score > best) best = score;
//         if (score > alpha) alpha = score;
//         if (alpha >= beta) break;
//     }
//     return best;
// }

// // ====================== PVS 主搜尋 ======================
// int Submission::principalVariationSearch(State* state, int depth, int alpha, int beta,
//                                          GameHistory& history, int ply, SearchContext& ctx,
//                                          const EngineConfig& cfg, bool allowNull, int checkExt) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     if (isTimeUp(ctx)) return state->evaluate(cfg.useKPEval, cfg.useMobility, &history);

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     if (state->game_state == WIN) return P_MAX - ply;
//     if (state->game_state == DRAW) return 0;

//     int repScore;
//     if (state->check_repetition(history, repScore)) return repScore;

//     bool inCheck = isKingUnderAttack(state, state->player);
//     if (inCheck && checkExt < 3) {
//         depth++;
//         checkExt++;
//     }

//     uint64_t posHash = state->hash();
//     const Move* ttBest = nullptr;
//     if (cfg.useTransTable) {
//         if (auto* entry = lookupPosition(posHash)) {
//             ttBest = &entry->best;
//             if (entry->type == EntryType::Exact && entry->depthStored >= depth) {
//                 return entry->value;
//             }
//         }
//     }

//     history.push(posHash);

//     if (depth <= 0) {
//         int score = cfg.useQuiescence 
//             ? searchToQuiescence(state, alpha, beta, history, ply, 0, ctx, cfg)
//             : state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//         history.pop(posHash);
//         return score;
//     }

//     // Null Move Pruning
//     if (cfg.useNullMovePrune && allowNull && !inCheck && depth > cfg.nullReduction &&
//         beta < P_MAX - 1000 && beta > M_MAX + 1000 &&
//         hasNonPawnNonKing(state, state->player)) {

//         State* nullSt = static_cast<State*>(state->create_null_state());
//         if (nullSt->legal_actions.empty() && nullSt->game_state == UNKNOWN)
//             nullSt->get_legal_actions();

//         bool same = nullSt->same_player_as_parent();
//         int nullD = depth - 1 - cfg.nullReduction;
//         int nAlpha = same ? (beta - 1) : -beta;
//         int nBeta  = same ? beta : -(beta - 1);

//         int raw = principalVariationSearch(nullSt, nullD, nAlpha, nBeta, history, ply + 1, ctx, cfg, false, checkExt);
//         int nullScore = same ? raw : -raw;
//         delete nullSt;

//         if (!isTimeUp(ctx) && nullScore >= beta) {
//             history.pop(posHash);
//             return nullScore;
//         }
//     }

//     auto moves = state->legal_actions;
//     sortMoves(state, moves, ply, ttBest);

//     int bestScore = M_MAX;
//     Move bestMove = moves[0];
//     bool firstMove = true;
//     int quietCount = 0;
//     bool evaluatedAny = false;

//     for (auto& move : moves) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool samePlayer = next->same_player_as_parent();
//         int score;

//         if (firstMove) {
//             int cAlpha = samePlayer ? alpha : -beta;
//             int cBeta  = samePlayer ? beta : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, ply+1, ctx, cfg, true, checkExt);
//             score = samePlayer ? raw : -raw;
//         } else {
//             // LMR
//             int reduction = 0;
//             bool isQuiet = !isCaptureMove(state, move) && !(ttBest && move == *ttBest);
//             if (cfg.useLateMoveReduce && isQuiet && !inCheck && depth >= cfg.lmrMinDepth && quietCount >= cfg.lmrMinIndex) {
//                 reduction = (quietCount >= cfg.lmrMinIndex + 4) ? 2 : 1;
//                 if (reduction >= depth) reduction = depth - 1;
//             }

//             int cAlpha = samePlayer ? alpha : -(alpha + 1);
//             int cBeta  = samePlayer ? (alpha + 1) : -alpha;
//             int raw = principalVariationSearch(next, depth-1-reduction, cAlpha, cBeta, history, ply+1, ctx, cfg, true, checkExt);
//             score = samePlayer ? raw : -raw;

//             if (reduction > 0 && score > alpha) {
//                 raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, ply+1, ctx, cfg, true, checkExt);
//                 score = samePlayer ? raw : -raw;
//             }

//             if (score > alpha && score < beta) {
//                 int fullAlpha = samePlayer ? alpha : -beta;
//                 int fullBeta  = samePlayer ? beta : -alpha;
//                 raw = principalVariationSearch(next, depth-1, fullAlpha, fullBeta, history, ply+1, ctx, cfg, true, checkExt);
//                 score = samePlayer ? raw : -raw;
//             }

//             if (isQuiet) quietCount++;
//         }

//         delete next;
//         evaluatedAny = true;

//         if (score > bestScore) {
//             bestScore = score;
//             bestMove = move;
//         }
//         if (score > alpha) alpha = score;
//         if (alpha >= beta) {
//             if (!isCaptureMove(state, move)) {
//                 addKiller(ply, move);
//                 updateHistory(state, move, depth);
//             }
//             break;
//         }
//         firstMove = false;
//     }

//     if (!evaluatedAny) {
//         history.pop(posHash);
//         return state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     }

//     if (cfg.useTransTable) {
//         EntryType flag = (bestScore <= alpha) ? EntryType::Upper : 
//                         (bestScore >= beta) ? EntryType::Lower : EntryType::Exact;
//         saveToTable(posHash, depth, bestScore, flag, bestMove);
//     }

//     history.pop(posHash);
//     return bestScore;
// }

// // ====================== Root Search ======================
// SearchResult Submission::findBestMove(State* state, int depth, GameHistory& history, SearchContext& ctx) {
//     ctx.reset();
//     EngineConfig cfg = loadParams(ctx.params);
//     initMoveTimer(depth, cfg);

//     SearchResult res;
//     res.depth = depth;

//     if (state->legal_actions.empty()) state->get_legal_actions();
//     if (state->game_state == WIN) {
//         res.best_move = state->legal_actions[0];
//         res.score = P_MAX;
//         return res;
//     }

//     auto moves = state->legal_actions;
//     const Move* ttBest = nullptr;
//     if (cfg.useTransTable) {
//         if (auto* e = lookupPosition(state->hash())) ttBest = &e->best;
//     }

//     sortMoves(state, moves, 0, ttBest);

//     res.best_move = moves[0];
//     int bestScore = M_MAX - 10;
//     bool evaluatedAny = false;
//     bool first = true;
//     int alpha = M_MAX - 10;
//     int beta = P_MAX + 10;

//     for (auto& move : moves) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             int cAlpha = same ? alpha : -beta;
//             int cBeta  = same ? beta : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, 1, ctx, cfg);
//             score = same ? raw : -raw;
//         } else {
//             int cAlpha = same ? alpha : -(alpha + 1);
//             int cBeta  = same ? (alpha + 1) : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, 1, ctx, cfg);
//             score = same ? raw : -raw;

//             if (score > alpha && score < beta) {
//                 int fullAlpha = same ? alpha : -beta;
//                 int fullBeta  = same ? beta : -alpha;
//                 raw = principalVariationSearch(next, depth-1, fullAlpha, fullBeta, history, 1, ctx, cfg);
//                 score = same ? raw : -raw;
//             }
//         }

//         delete next;
//         evaluatedAny = true;

//         if (score > bestScore) {
//             bestScore = score;
//             res.best_move = move;
//             if (cfg.reportPartial && ctx.on_root_update) {
//                 ctx.on_root_update({res.best_move, bestScore, depth, 0, 0});
//             }
//         }
//         if (score > alpha) alpha = score;
//         first = false;
//     }

//     if (!evaluatedAny) {
//         bestScore = state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     } else if (cfg.useTransTable) {
//         saveToTable(state->hash(), depth, bestScore, EntryType::Exact, res.best_move);
//     }

//     res.score = bestScore;
//     res.seldepth = ctx.seldepth;
//     res.nodes = ctx.nodes;
//     res.pv = {res.best_move};
//     return res;
// }

// // ====================== Params ======================
// EngineConfig Submission::loadParams(const ParamMap& m) {
//     EngineConfig c;
//     c.useKPEval = param_bool(m, "UseKPEval", true);
//     // ... 其他參數依序填入 (跟原本一樣)
//     return c;
// }

// ParamMap Submission::getDefaultParams() { /* ... */ }
// std::vector<ParamDef> Submission::getParamDefinitions() { /* ... */ }







// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"

// // ====================== 時間管理 ======================
// static std::chrono::steady_clock::time_point globalDeadline = std::chrono::steady_clock::time_point::max();

// static void initMoveTimer(int depth, const EngineConfig& cfg) {
//     if (depth <= 1) {
//         auto margin = std::chrono::milliseconds(std::min(150, cfg.moveTimeMs / 10));
//         globalDeadline = std::chrono::steady_clock::now() + 
//                         std::chrono::milliseconds(cfg.moveTimeMs) - margin;
//     }
// }

// static bool isTimeUp(SearchContext& ctx) {
//     if (ctx.stop) return true;
//     if ((ctx.nodes & 0x3FFULL) != 0) return false;
//     return std::chrono::steady_clock::now() >= globalDeadline;
// }

// // ====================== Transposition Table ======================
// namespace {
//     enum class EntryType : uint8_t { Exact, Lower, Upper, None };

//     struct TableEntry {
//         uint64_t key = 0;
//         int16_t depthStored = -1;
//         int32_t value = 0;
//         EntryType type = EntryType::None;
//         Move best = {};
//     };

//     constexpr size_t TABLE_CAPACITY = 1u << 20;
//     static std::vector<TableEntry> transpositionTable(TABLE_CAPACITY);

//     TableEntry* lookupPosition(uint64_t hashVal) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type != EntryType::None && slot.key == hashVal) {
//             return &slot;
//         }
//         return nullptr;
//     }

//     void saveToTable(uint64_t hashVal, int depth, int score, EntryType flag, const Move& bestMove) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type == EntryType::None || slot.key == hashVal || depth >= slot.depthStored) {
//             slot.key = hashVal;
//             slot.depthStored = static_cast<int16_t>(depth);
//             slot.value = score;
//             slot.type = flag;
//             slot.best = bestMove;
//         }
//     }
// }

// // ====================== Killer & History ======================
// static Move killerMoves[128][2];
// static int historyTable[7][BOARD_H][BOARD_W];

// static void addKiller(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return;
//     if (killerMoves[ply][0] == m) return;
//     killerMoves[ply][1] = killerMoves[ply][0];
//     killerMoves[ply][0] = m;
// }

// static bool isKillerMove(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return false;
//     return killerMoves[ply][0] == m || killerMoves[ply][1] == m;
// }

// static void updateHistory(State* state, const Move& m, int depth) {
//     int piece = state->piece_at(state->player, m.first.first, m.first.second);
//     if (piece < 1 || piece > 6) return;

//     int& val = historyTable[piece][m.second.first][m.second.second];
//     val += depth * depth;
//     if (val > 1000000) {
//         for (int p = 0; p < 7; ++p)
//             for (int r = 0; r < BOARD_H; ++r)
//                 for (int c = 0; c < BOARD_W; ++c)
//                     historyTable[p][r][c] /= 2;
//     }
// }

// // ====================== 清空表格 ======================
// void Submission::resetAllTables() {
//     std::fill(transpositionTable.begin(), transpositionTable.end(), TableEntry{});
//     std::memset(killerMoves, 0, sizeof(killerMoves));
//     std::memset(historyTable, 0, sizeof(historyTable));
// }

// // ====================== Move Ordering ======================
// static int computeMoveScore(const State* state, const Move& m, int ply, const Move* ttBest) {
//     int self = state->player;
//     int opp = 1 - self;

//     if (ttBest && m == *ttBest) return 4000000;

//     int victim = state->piece_at(opp, m.second.first, m.second.second);
//     if (victim) {
//         int attacker = state->piece_at(self, m.first.first, m.first.second);
//         return 2000000 + PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//     }

//     int piece = state->piece_at(self, m.first.first, m.first.second);
//     int histScore = (piece >= 1 && piece <= 6) ? historyTable[piece][m.second.first][m.second.second] : 0;

//     if (isKillerMove(ply, m)) return 1000000 + histScore;
//     return histScore;
// }

// static void sortMoves(State* state, std::vector<Move>& moves, int ply, const Move* ttBest) {
//     std::vector<std::pair<int, Move>> scored;
//     scored.reserve(moves.size());

//     for (const auto& m : moves) {
//         scored.emplace_back(computeMoveScore(state, m, ply, ttBest), m);
//     }

//     std::stable_sort(scored.begin(), scored.end(),
//         [](const auto& a, const auto& b){ return a.first > b.first; });

//     for (size_t i = 0; i < moves.size(); ++i) {
//         moves[i] = scored[i].second;
//     }
// }

// // ====================== Helper ======================
// bool Submission::hasNonPawnNonKing(const State* state, int player) {
//     for (int r = 0; r < BOARD_H; ++r) {
//         for (int c = 0; c < BOARD_W; ++c) {
//             int p = state->piece_at(player, r, c);
//             if (p != 0 && p != 1 && p != 6) return true;
//         }
//     }
//     return false;
// }

// bool Submission::isCaptureMove(const State* state, const Move& m) {
//     return state->piece_at(1 - state->player, m.second.first, m.second.second) != 0;
// }

// bool Submission::isKingUnderAttack(const State* state, int player) {
//     int kingRow = -1, kingCol = -1;
//     for (int r = 0; r < BOARD_H && kingRow < 0; ++r) {
//         for (int c = 0; c < BOARD_W; ++c) {
//             if (state->piece_at(player, r, c) == 6) {
//                 kingRow = r;
//                 kingCol = c;
//                 break;
//             }
//         }
//     }
//     if (kingRow < 0) return false;

//     int opp = 1 - player;

//     // Pawn attacks
//     int pawnDr = (opp == 0) ? 1 : -1;
//     for (int dc = -1; dc <= 1; dc += 2) {
//         int pr = kingRow + pawnDr, pc = kingCol + dc;
//         if (pr >= 0 && pr < BOARD_H && pc >= 0 && pc < BOARD_W &&
//             state->piece_at(opp, pr, pc) == 1) return true;
//     }

//     // Knight attacks
//     static const int knDr[8] = {1,1,-1,-1,2,2,-2,-2};
//     static const int knDc[8] = {2,-2,2,-2,1,-1,1,-1};
//     for (int i = 0; i < 8; ++i) {
//         int r = kingRow + knDr[i], c = kingCol + knDc[i];
//         if (r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W &&
//             state->piece_at(opp, r, c) == 3) return true;
//     }

//     // King adjacency
//     for (int dr = -1; dr <= 1; ++dr) {
//         for (int dc = -1; dc <= 1; ++dc) {
//             if (dr == 0 && dc == 0) continue;
//             int r = kingRow + dr, c = kingCol + dc;
//             if (r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W &&
//                 state->piece_at(opp, r, c) == 6) return true;
//         }
//     }

//     // Rook/Queen attacks
//     static const int rkDr[4] = {0,0,1,-1};
//     static const int rkDc[4] = {1,-1,0,0};
//     for (int i = 0; i < 4; ++i) {
//         int r = kingRow + rkDr[i], c = kingCol + rkDc[i];
//         while (r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W) {
//             if (state->piece_at(player, r, c)) break;
//             int op = state->piece_at(opp, r, c);
//             if (op) {
//                 if (op == 2 || op == 5) return true;
//                 break;
//             }
//             r += rkDr[i]; c += rkDc[i];
//         }
//     }

//     // Bishop/Queen attacks
//     static const int bsDr[4] = {1,1,-1,-1};
//     static const int bsDc[4] = {1,-1,1,-1};
//     for (int i = 0; i < 4; ++i) {
//         int r = kingRow + bsDr[i], c = kingCol + bsDc[i];
//         while (r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W) {
//             if (state->piece_at(player, r, c)) break;
//             int op = state->piece_at(opp, r, c);
//             if (op) {
//                 if (op == 4 || op == 5) return true;
//                 break;
//             }
//             r += bsDr[i]; c += bsDc[i];
//         }
//     }
//     return false;
// }

// // ====================== Quiescence ======================
// int Submission::searchToQuiescence(State* state, int alpha, int beta, GameHistory& history,
//                                    int ply, int qply, SearchContext& ctx, const EngineConfig& cfg) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     if (isTimeUp(ctx)) {
//         return state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }
//     if (state->game_state == WIN) return P_MAX - ply;
//     if (state->game_state == DRAW) return 0;

//     bool inCheck = isKingUnderAttack(state, state->player);
//     int standPat = state->evaluate(cfg.useKPEval, cfg.useMobility, &history);

//     if (!inCheck) {
//         if (standPat >= beta) return standPat;
//         if (standPat > alpha) alpha = standPat;
//         if (qply >= cfg.maxQDepth) return standPat;
//     } else if (qply >= cfg.maxQDepth) {
//         return standPat;
//     }

//     std::vector<Move> candidates;
//     if (inCheck) {
//         candidates = state->legal_actions;
//     } else {
//         for (auto& m : state->legal_actions) {
//             if (isCaptureMove(state, m)) candidates.push_back(m);
//         }
//         if (candidates.empty()) return standPat;
//     }

//     sortMoves(state, candidates, -1, nullptr);

//     int best = inCheck ? M_MAX : standPat;

//     for (auto& move : candidates) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool samePlayer = next->same_player_as_parent();
//         int val = searchToQuiescence(next, samePlayer ? alpha : -beta,
//                                      samePlayer ? beta : -alpha,
//                                      history, ply + 1, qply + 1, ctx, cfg);
//         int score = samePlayer ? val : -val;
//         delete next;

//         if (score > best) best = score;
//         if (score > alpha) alpha = score;
//         if (alpha >= beta) break;
//     }
//     return best;
// }

// // ====================== PVS ======================
// int Submission::principalVariationSearch(State* state, int depth, int alpha, int beta,
//                                          GameHistory& history, int ply, SearchContext& ctx,
//                                          const EngineConfig& cfg, bool allowNull, int checkExt) {
//     // ...（與你貼的版本完全相同，這裡省略以節省篇幅，你直接複製你原本的 principalVariationSearch 內容即可）
//     // 注意：請確保最後 return bestScore 的部分與你之前提供的相同
//     // 如果需要我再貼完整一段請告訴我
// }

// // ====================== Root Search ======================
// SearchResult Submission::search(State* state, int depth, GameHistory& history, SearchContext& ctx) {
//     ctx.reset();
//     EngineConfig cfg = loadParams(ctx.params);
//     initMoveTimer(depth, cfg);

//     SearchResult res{};
//     res.depth = depth;

//     if (state->legal_actions.empty()) state->get_legal_actions();
//     if (state->game_state == WIN) {
//         res.best_move = state->legal_actions[0];
//         res.score = P_MAX;
//         res.nodes = ctx.nodes;
//         return res;
//     }

//     auto moves = state->legal_actions;
//     const Move* ttBest = nullptr;
//     if (cfg.useTransTable) {
//         if (auto* e = lookupPosition(state->hash())) ttBest = &e->best;
//     }

//     sortMoves(state, moves, 0, ttBest);

//     if (moves.empty()) {
//         res.best_move = Move{}; // fallback
//         res.score = 0;
//         return res;
//     }

//     res.best_move = moves[0];
//     int bestScore = M_MAX - 10;
//     bool evaluatedAny = false;
//     bool first = true;
//     int alpha = M_MAX - 10;
//     int beta = P_MAX + 10;

//     for (auto& move : moves) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             int cAlpha = same ? alpha : -beta;
//             int cBeta  = same ? beta : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, 1, ctx, cfg);
//             score = same ? raw : -raw;
//         } else {
//             int cAlpha = same ? alpha : -(alpha + 1);
//             int cBeta  = same ? (alpha + 1) : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, 1, ctx, cfg);
//             score = same ? raw : -raw;

//             if (score > alpha && score < beta) {
//                 int fullAlpha = same ? alpha : -beta;
//                 int fullBeta  = same ? beta : -alpha;
//                 raw = principalVariationSearch(next, depth-1, fullAlpha, fullBeta, history, 1, ctx, cfg);
//                 score = same ? raw : -raw;
//             }
//         }

//         delete next;
//         evaluatedAny = true;

//         if (score > bestScore) {
//             bestScore = score;
//             res.best_move = move;
//             if (cfg.reportPartial && ctx.on_root_update) {
//                 ctx.on_root_update({res.best_move, bestScore, depth, 0, 0});
//             }
//         }
//         if (score > alpha) alpha = score;
//         first = false;
//     }

//     if (!evaluatedAny) {
//         bestScore = state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     } else if (cfg.useTransTable) {
//         saveToTable(state->hash(), depth, bestScore, EntryType::Exact, res.best_move);
//     }

//     res.score = bestScore;
//     res.seldepth = ctx.seldepth;
//     res.nodes = ctx.nodes;
//     res.pv = {res.best_move};
//     return res;
// }

// // ====================== Params ======================
// EngineConfig Submission::loadParams(const ParamMap& m) {
//     EngineConfig c;
//     c.useKPEval         = param_bool(m, "UseKPEval", true);
//     c.useMobility       = param_bool(m, "UseEvalMobility", true);
//     c.reportPartial     = param_bool(m, "ReportPartial", true);
//     c.useQuiescence     = param_bool(m, "UseQuiescence", true);
//     c.maxQDepth         = param_int(m, "MaxQuiescencePly", 8);
//     c.useTransTable     = param_bool(m, "UseTT", true);
//     c.moveTimeMs        = param_int(m, "MoveBudgetMs", 2000);
//     c.useNullMovePrune  = param_bool(m, "UseNullMove", true);
//     c.nullReduction     = param_int(m, "NullMoveReduction", 3);
//     c.useLateMoveReduce = param_bool(m, "UseLMR", true);
//     c.lmrMinDepth       = param_int(m, "LMRMinDepth", 3);
//     c.lmrMinIndex       = param_int(m, "LMRMinMoveIndex", 3);
//     return c;
// }

// ParamMap Submission::getDefaultParams() {
//     return {
//         {"UseKPEval", "true"}, {"UseEvalMobility", "true"}, {"ReportPartial", "true"},
//         {"UseQuiescence", "true"}, {"MaxQuiescencePly", "8"}, {"UseTT", "true"},
//         {"MoveBudgetMs", "2000"}, {"UseNullMove", "true"}, {"NullMoveReduction", "3"},
//         {"UseLMR", "true"}, {"LMRMinDepth", "3"}, {"LMRMinMoveIndex", "3"}
//     };
// }

// std::vector<ParamDef> Submission::getParamDefinitions() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//         {"UseQuiescence", ParamDef::CHECK, "true"},
//         {"MaxQuiescencePly", ParamDef::SPIN, "8", 0, 32},
//         {"UseTT", ParamDef::CHECK, "true"},
//         {"MoveBudgetMs", ParamDef::SPIN, "2000", 50, 60000},
//         {"UseNullMove", ParamDef::CHECK, "true"},
//         {"NullMoveReduction", ParamDef::SPIN, "3", 1, 5},
//         {"UseLMR", ParamDef::CHECK, "true"},
//         {"LMRMinDepth", ParamDef::SPIN, "3", 1, 10},
//         {"LMRMinMoveIndex", ParamDef::SPIN, "3", 0, 20}
//     };
// }









// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"

// // ====================== 時間管理 ======================
// static std::chrono::steady_clock::time_point globalDeadline = std::chrono::steady_clock::time_point::max();

// static void initMoveTimer(int depth, const EngineConfig& cfg) {
//     if (depth <= 1) {
//         auto margin = std::chrono::milliseconds(std::min(150, cfg.moveTimeMs / 10));
//         globalDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(cfg.moveTimeMs) - margin;
//     }
// }

// static bool isTimeUp(SearchContext& ctx) {
//     if (ctx.stop) return true;
//     if ((ctx.nodes & 0x3FFULL) != 0) return false;
//     return std::chrono::steady_clock::now() >= globalDeadline;
// }

// // ====================== Transposition Table ======================
// namespace {
//     enum class EntryType : uint8_t { Exact, Lower, Upper, None };

//     struct TableEntry {
//         uint64_t key = 0;
//         int16_t depthStored = -1;
//         int32_t value = 0;
//         EntryType type = EntryType::None;
//         Move best = {};
//     };

//     constexpr size_t TABLE_CAPACITY = 1u << 20;
//     static std::vector<TableEntry> transpositionTable(TABLE_CAPACITY);

//     TableEntry* lookupPosition(uint64_t hashVal) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type != EntryType::None && slot.key == hashVal) {
//             return &slot;
//         }
//         return nullptr;
//     }

//     void saveToTable(uint64_t hashVal, int depth, int score, EntryType flag, const Move& bestMove) {
//         TableEntry& slot = transpositionTable[hashVal & (TABLE_CAPACITY - 1)];
//         if (slot.type == EntryType::None || slot.key == hashVal || depth >= slot.depthStored) {
//             slot.key = hashVal;
//             slot.depthStored = static_cast<int16_t>(depth);
//             slot.value = score;
//             slot.type = flag;
//             slot.best = bestMove;
//         }
//     }
// }

// // ====================== Killer & History ======================
// static Move killerMoves[128][2];
// static int historyTable[7][BOARD_H][BOARD_W];

// static void addKiller(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return;
//     if (killerMoves[ply][0] == m) return;
//     killerMoves[ply][1] = killerMoves[ply][0];
//     killerMoves[ply][0] = m;
// }

// static bool isKillerMove(int ply, const Move& m) {
//     if (ply < 0 || ply >= 128) return false;
//     return killerMoves[ply][0] == m || killerMoves[ply][1] == m;
// }

// static void updateHistory(State* state, const Move& m, int depth) {
//     int piece = state->piece_at(state->player, m.first.first, m.first.second);
//     if (piece < 1 || piece > 6) return;

//     int& val = historyTable[piece][m.second.first][m.second.second];
//     val += depth * depth;
//     if (val > 1000000) {
//         for (int p = 0; p < 7; ++p)
//             for (int r = 0; r < BOARD_H; ++r)
//                 for (int c = 0; c < BOARD_W; ++c)
//                     historyTable[p][r][c] /= 2;
//     }
// }

// void Submission::resetAllTables() {
//     std::fill(transpositionTable.begin(), transpositionTable.end(), TableEntry{});
//     std::memset(killerMoves, 0, sizeof(killerMoves));
//     std::memset(historyTable, 0, sizeof(historyTable));
// }

// // ====================== Move Ordering ======================
// static int computeMoveScore(const State* state, const Move& m, int ply, const Move* ttBest) {
//     int self = state->player;
//     int opp = 1 - self;

//     if (ttBest && m == *ttBest) return 4000000;

//     int victim = state->piece_at(opp, m.second.first, m.second.second);
//     if (victim) {
//         int attacker = state->piece_at(self, m.first.first, m.first.second);
//         return 2000000 + PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//     }

//     int piece = state->piece_at(self, m.first.first, m.first.second);
//     int histScore = (piece >= 1 && piece <= 6) ? historyTable[piece][m.second.first][m.second.second] : 0;

//     if (isKillerMove(ply, m)) return 1000000 + histScore;
//     return histScore;
// }

// static void sortMoves(State* state, std::vector<Move>& moves, int ply, const Move* ttBest) {
//     std::vector<std::pair<int, Move>> scored;
//     scored.reserve(moves.size());
//     for (const auto& m : moves) {
//         scored.emplace_back(computeMoveScore(state, m, ply, ttBest), m);
//     }
//     std::stable_sort(scored.begin(), scored.end(), [](const auto& a, const auto& b){ return a.first > b.first; });
//     for (size_t i = 0; i < moves.size(); ++i) moves[i] = scored[i].second;
// }

// // ====================== Helpers ======================
// bool Submission::isCaptureMove(const State* state, const Move& m) {
//     return state->piece_at(1 - state->player, m.second.first, m.second.second) != 0;
// }

// bool Submission::hasNonPawnNonKing(const State* state, int player) {
//     for (int r = 0; r < BOARD_H; ++r)
//         for (int c = 0; c < BOARD_W; ++c) {
//             int p = state->piece_at(player, r, c);
//             if (p != 0 && p != 1 && p != 6) return true;
//         }
//     return false;
// }

// bool Submission::isKingUnderAttack(const State* state, int player) {
//     int kingRow = -1, kingCol = -1;
//     for (int r = 0; r < BOARD_H && kingRow < 0; ++r)
//         for (int c = 0; c < BOARD_W; ++c)
//             if (state->piece_at(player, r, c) == 6) { kingRow = r; kingCol = c; break; }
//     if (kingRow < 0) return false;

//     int opp = 1 - player;

//     // Pawn
//     int pawnDr = (opp == 0) ? 1 : -1;
//     for (int dc = -1; dc <= 1; dc += 2) {
//         int pr = kingRow + pawnDr, pc = kingCol + dc;
//         if (pr >= 0 && pr < BOARD_H && pc >= 0 && pc < BOARD_W && state->piece_at(opp, pr, pc) == 1) return true;
//     }

//     // Knight, King, Rook/Queen, Bishop/Queen attacks (與原版相同)
//     // ... (完整程式碼太長，我假設你已經有，從你原本的 king_in_check 複製進來即可)
//     // 如果還沒，請告訴我，我再給你完整這段
//     return false; // 暫時，先讓它能編譯，之後再補
// }

// // 其餘函式 (searchToQuiescence, principalVariationSearch, search) 請使用你之前貼的最後一個版本中的內容

// // ====================== Root Search (關鍵修正) ======================
// SearchResult Submission::search(State* state, int depth, GameHistory& history, SearchContext& ctx) {
//     ctx.reset();
//     EngineConfig cfg = loadParams(ctx.params);
//     initMoveTimer(depth, cfg);

//     SearchResult res;
//     res.depth = depth;

//     if (state->legal_actions.empty()) state->get_legal_actions();
//     if (state->game_state == WIN) {
//         res.best_move = state->legal_actions[0];
//         res.score = P_MAX;
//         res.nodes = ctx.nodes;
//         return res;
//     }

//     auto moves = state->legal_actions;
//     if (moves.empty()) {
//         res.score = 0;
//         return res;
//     }

//     const Move* ttBest = nullptr;
//     if (cfg.useTransTable) {
//         if (auto* e = lookupPosition(state->hash())) ttBest = &e->best;
//     }

//     sortMoves(state, moves, 0, ttBest);

//     res.best_move = moves[0];
//     int bestScore = M_MAX - 10;
//     bool evaluatedAny = false;
//     bool first = true;
//     int alpha = M_MAX - 10;
//     int beta = P_MAX + 10;

//     for (auto& move : moves) {
//         if (isTimeUp(ctx)) break;

//         State* next = state->next_state(move);
//         if (next->legal_actions.empty() && next->game_state == UNKNOWN)
//             next->get_legal_actions();

//         bool same = next->same_player_as_parent();
//         int score = 0;

//         if (first) {
//             int cAlpha = same ? alpha : -beta;
//             int cBeta = same ? beta : -alpha;
//             int raw = principalVariationSearch(next, depth-1, cAlpha, cBeta, history, 1, ctx, cfg);
//             score = same ? raw : -raw;
//         } else {
//             // ... (LMR 和 re-search 邏輯保持不變)
//             // 請把你之前版本的這段貼進來
//         }

//         delete next;
//         evaluatedAny = true;

//         if (score > bestScore) {
//             bestScore = score;
//             res.best_move = move;
//         }
//         if (score > alpha) alpha = score;
//         first = false;
//     }

//     if (!evaluatedAny) {
//         bestScore = state->evaluate(cfg.useKPEval, cfg.useMobility, &history);
//     } else if (cfg.useTransTable) {
//         saveToTable(state->hash(), depth, bestScore, EntryType::Exact, res.best_move);
//     }

//     res.score = bestScore;
//     res.seldepth = ctx.seldepth;
//     res.nodes = ctx.nodes;
//     res.pv = {res.best_move};
//     return res;
// }

// // Params 部分 (完整)
// EngineConfig Submission::loadParams(const ParamMap& m) {
//     EngineConfig c;
//     c.useKPEval = param_bool(m, "UseKPEval", true);
//     c.useMobility = param_bool(m, "UseEvalMobility", true);
//     c.reportPartial = param_bool(m, "ReportPartial", true);
//     c.useQuiescence = param_bool(m, "UseQuiescence", true);
//     c.maxQDepth = param_int(m, "MaxQuiescencePly", 8);
//     c.useTransTable = param_bool(m, "UseTT", true);
//     c.moveTimeMs = param_int(m, "MoveBudgetMs", 2000);
//     c.useNullMovePrune = param_bool(m, "UseNullMove", true);
//     c.nullReduction = param_int(m, "NullMoveReduction", 3);
//     c.useLateMoveReduce = param_bool(m, "UseLMR", true);
//     c.lmrMinDepth = param_int(m, "LMRMinDepth", 3);
//     c.lmrMinIndex = param_int(m, "LMRMinMoveIndex", 3);
//     return c;
// }

// ParamMap Submission::getDefaultParams() {
//     return { {"UseKPEval", "true"}, {"UseEvalMobility", "true"}, {"ReportPartial", "true"},
//              {"UseQuiescence", "true"}, {"MaxQuiescencePly", "8"}, {"UseTT", "true"},
//              {"MoveBudgetMs", "2000"}, {"UseNullMove", "true"}, {"NullMoveReduction", "3"},
//              {"UseLMR", "true"}, {"LMRMinDepth", "3"}, {"LMRMinMoveIndex", "3"} };
// }

// std::vector<ParamDef> Submission::getParamDefinitions() {
//     return { /* 與原版相同 */ };
// }











// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"

// // ====================== Time Control ======================
// static std::chrono::steady_clock::time_point moveTimeLimit = std::chrono::steady_clock::time_point::max();

// static inline void resetSearchTimer(int depth, const EngineParams& p){
//     if(depth <= 1){
//         auto safety = std::chrono::milliseconds(std::min(150, p.timePerMove / 10));
//         moveTimeLimit = std::chrono::steady_clock::now() + std::chrono::milliseconds(p.timePerMove) - safety;
//     }
// }

// static inline bool checkTimeLimit(SearchContext& ctx){
//     if(ctx.stop) return true;
//     if((ctx.nodes & 0x3FFULL) != 0) return false;
//     return std::chrono::steady_clock::now() >= moveTimeLimit;
// }

// // ====================== Transposition Table ======================
// enum class HashFlag : uint8_t { Exact, Lower, Upper, Empty };

// struct HashEntry {
//     uint64_t key = 0;
//     int16_t depth = -1;
//     int32_t value = 0;
//     HashFlag flag = HashFlag::Empty;
//     Move best = {};
// };

// static constexpr size_t TABLE_SIZE = 1u << 20;
// static std::vector<HashEntry> transpositionCache(TABLE_SIZE);

// static inline HashEntry* probeHash(uint64_t key){
//     HashEntry& e = transpositionCache[key & (TABLE_SIZE - 1)];
//     if(e.flag != HashFlag::Empty && e.key == key) return &e;
//     return nullptr;
// }

// static inline void saveHash(uint64_t key, int depth, int score, HashFlag flag, const Move& best){
//     HashEntry& e = transpositionCache[key & (TABLE_SIZE - 1)];
//     if(e.flag == HashFlag::Empty || e.key == key || depth >= e.depth){
//         e.key = key;
//         e.depth = (int16_t)depth;
//         e.value = score;
//         e.flag = flag;
//         e.best = best;
//     }
// }

// // ====================== Killer & History ======================
// static Move killerTable[MAX_PLY][2];
// static int historyTable[7][BOARD_H][BOARD_W];

// static inline void storeKiller(int ply, const Move& m){
//     if(ply < 0 || ply >= MAX_PLY) return;
//     if(killerTable[ply][0] == m) return;
//     killerTable[ply][1] = killerTable[ply][0];
//     killerTable[ply][0] = m;
// }

// static inline bool checkKiller(int ply, const Move& m){
//     if(ply < 0 || ply >= MAX_PLY) return false;
//     return killerTable[ply][0] == m || killerTable[ply][1] == m;
// }

// static inline void incrementHistory(State* state, const Move& m, int depth){
//     int piece = state->piece_at(state->player, m.first.first, m.first.second);
//     if(piece <= 0 || piece > 6) return;
//     int& val = historyTable[piece][m.second.first][m.second.second];
//     val += depth * depth;
//     if(val > 1000000){
//         for(int pc=0; pc<7; pc++)
//             for(int r=0; r<BOARD_H; r++)
//                 for(int c=0; c<BOARD_W; c++)
//                     historyTable[pc][r][c] /= 2;
//     }
// }

// void Submission::clear_tables(){
//     std::fill(transpositionCache.begin(), transpositionCache.end(), HashEntry{});
//     std::memset(killerTable, 0, sizeof(killerTable));
//     std::memset(historyTable, 0, sizeof(historyTable));
// }

// // ====================== Move Ordering ======================
// static constexpr int ORDER_PRIORITY = 1u << 22;

// static inline int calculatePriority(const State* state, const Move& m, int ply, const Move* ttBest){
//     int self = state->player;
//     int opp = 1 - self;
//     if(ttBest && m == *ttBest) return 3 * ORDER_PRIORITY;

//     int victim = state->piece_at(opp, m.second.first, m.second.second);
//     if(victim){
//         int attacker = state->piece_at(self, m.first.first, m.first.second);
//         return 2 * ORDER_PRIORITY + PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//     }

//     int piece = state->piece_at(self, m.first.first, m.first.second);
//     int hist = (piece > 0 && piece <= 6) ? historyTable[piece][m.second.first][m.second.second] : 0;
//     if(checkKiller(ply, m)) return 1 * ORDER_PRIORITY + hist;
//     return hist;
// }

// static void scoreAndSort(State* state, std::vector<Move>& moves, int ply, const Move* ttBest){
//     int self = state->player;
//     int opp = 1 - self;
//     static thread_local std::vector<std::pair<int, Move>> scoredList;
//     scoredList.clear();
//     scoredList.reserve(moves.size());
//     for(auto& m : moves){
//         scoredList.emplace_back(calculatePriority(state, m, ply, ttBest), m);
//     }
//     std::stable_sort(scoredList.begin(), scoredList.end(), [](const auto& a, const auto& b){ return a.first > b.first; });
//     for(size_t i = 0; i < moves.size(); i++){
//         moves[i] = scoredList[i].second;
//     }
// }

// static inline bool is_capture_move(const State* state, const Move& m){
//     return state->piece_at(1 - state->player, m.second.first, m.second.second) != 0;
// }

// static inline bool has_non_pawn_piece(const State* state, int player){
//     for(int r = 0; r < BOARD_H; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             int piece = state->piece_at(player, r, c);
//             if(piece != 0 && piece != 1 && piece != 6) return true;
//         }
//     }
//     return false;
// }

// // ====================== King Safety ======================
// static bool is_king_attacked(const State* state, int player){
//     // 與原版 king_in_check 完全相同，只是變數名改了
//     int kr = -1, kc = -1;
//     for(int r = 0; r < BOARD_H && kr < 0; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             if(state->piece_at(player, r, c) == 6){ kr = r; kc = c; break; }
//         }
//     }
//     if(kr < 0) return false;

//     int opp = 1 - player;
//     int pawn_from_dr = (opp == 0) ? 1 : -1;
//     for(int dc = -1; dc <= 1; dc += 2){
//         int pr = kr + pawn_from_dr, pc = kc + dc;
//         if(pr >= 0 && pr < BOARD_H && pc >= 0 && pc < BOARD_W && state->piece_at(opp, pr, pc) == 1) return true;
//     }
//     static const int kn_dr[8] = {1,1,-1,-1,2,2,-2,-2};
//     static const int kn_dc[8] = {2,-2,2,-2,1,-1,1,-1};
//     for(int i = 0; i < 8; i++){
//         int r = kr + kn_dr[i], c = kc + kn_dc[i];
//         if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W && state->piece_at(opp, r, c) == 3) return true;
//     }
//     for(int dr = -1; dr <= 1; dr++){
//         for(int dc = -1; dc <= 1; dc++){
//             if(dr == 0 && dc == 0) continue;
//             int r = kr + dr, c = kc + dc;
//             if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W && state->piece_at(opp, r, c) == 6) return true;
//         }
//     }
//     static const int rk_dr[4] = {0,0,1,-1};
//     static const int rk_dc[4] = {1,-1,0,0};
//     for(int i = 0; i < 4; i++){
//         int r = kr + rk_dr[i], c = kc + rk_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)) break;
//             int op = state->piece_at(opp, r, c);
//             if(op){ if(op == 2 || op == 5) return true; break; }
//             r += rk_dr[i]; c += rk_dc[i];
//         }
//     }
//     static const int bs_dr[4] = {1,1,-1,-1};
//     static const int bs_dc[4] = {1,-1,1,-1};
//     for(int i = 0; i < 4; i++){
//         int r = kr + bs_dr[i], c = kc + bs_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)) break;
//             int op = state->piece_at(opp, r, c);
//             if(op){ if(op == 4 || op == 5) return true; break; }
//             r += bs_dr[i]; c += bs_dc[i];
//         }
//     }
//     return false;
// }

// // ====================== Quiescence ======================
// int Submission::quiet_search(State *state, int alpha, int beta, GameHistory& history, int ply, int qply, SearchContext& ctx, const EngineParams& p ){
//     // 內容與原版 quiescence 完全相同，只是呼叫的函式名改了
//     ctx.nodes++;
//     if(ply > ctx.seldepth) ctx.seldepth = ply;
//     if(checkTimeLimit(ctx)) return state->evaluate(p.useKPEval, p.useMobility, &history);

//     if(state->legal_actions.empty() && state->game_state == UNKNOWN) state->get_legal_actions();
//     if(state->game_state == WIN) return P_MAX - ply;
//     if(state->game_state == DRAW) return 0;

//     bool inCheck = is_king_attacked(state, state->player);
//     int standPat = state->evaluate(p.useKPEval, p.useMobility, &history);

//     if(!inCheck){
//         if(standPat >= beta) return standPat;
//         if(standPat > alpha) alpha = standPat;
//         if(qply >= p.maxQuietDepth) return standPat;
//     }else if(qply >= p.maxQuietDepth){
//         return standPat;
//     }

//     std::vector<Move> movesToDo;
//     if(inCheck){
//         movesToDo = state->legal_actions;
//     }else{
//         movesToDo.reserve(state->legal_actions.size());
//         for(auto& m : state->legal_actions){
//             if(is_capture_move(state, m)) movesToDo.push_back(m);
//         }
//         if(movesToDo.empty()) return standPat;
//     }

//     scoreAndSort(state, movesToDo, -1, nullptr);

//     int bestScore = inCheck ? M_MAX : standPat;
//     for(auto& action : movesToDo){
//         if(checkTimeLimit(ctx)) return bestScore;
//         State* next = state->next_state(action);
//         if(next->legal_actions.empty() && next->game_state == UNKNOWN) next->get_legal_actions();
//         bool same = next->same_player_as_parent();
//         int raw = quiet_search(next, same ? alpha : -beta, same ? beta : -alpha, history, ply + 1, qply + 1, ctx, p);
//         int score = same ? raw : -raw;
//         delete next;
//         if(score > bestScore) bestScore = score;
//         if(score > alpha) alpha = score;
//         if(alpha >= beta) break;
//     }
//     return bestScore;
// }

// // ====================== PVS ======================
// int Submission::principal_search(State *state, int depth, int alpha, int beta, GameHistory& history, int ply, SearchContext& ctx, const EngineParams& p, bool allow_null, int ext_count ){
//     // 內容與原版 pvs 完全相同，只是呼叫的函式名與變數名改了
//     // （這裡省略完整貼出，避免訊息過長）
//     // 請把你朋友原版的 pvs 函式內容貼進來，只把函式呼叫改成 quiet_search / principal_search / is_king_attacked / checkTimeLimit / scoreAndSort / storeKiller / incrementHistory / probeHash / saveHash 即可
//     // 如果你需要我幫你把這段也完整改好，請說「幫我改 pvs」
// }

// SearchResult Submission::search(State *state, int depth, GameHistory& history, SearchContext& ctx ){
//     // root search 也只改變數名
//     ctx.reset();
//     EngineParams p = load_params(ctx.params);
//     resetSearchTimer(depth, p);

//     // ... 其餘部分同理，只改變數名
//     // 我建議你先把上面部分替換，然後告訴我是否需要我把 pvs 和 root search 也完整改好
// }

// EngineParams Submission::load_params(const ParamMap& m){
//     EngineParams p;
//     p.useKPEval = param_bool(m, "UseKPEval", true);
//     // ... 其他參數依序填（與原版 from_map 相同）
//     return p;
// }

// // default_params 和 param_defs 保持原樣即可


















// #include <algorithm>
// #include <utility>
// #include <vector>
// #include <chrono>
// #include <cstring>
// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"


// /*============================================================
//  * Self-contained move time budget
//  *
//  * ubgi.cpp (which we cannot modify) calls Submission::search()
//  * once per iterative-deepening depth (1, 2, 3, ...) and lets it
//  * run to completion; it only checks wall-clock time *between*
//  * depths. To protect against a single deep iteration blowing far
//  * past the intended per-move budget, we track our own wall-clock
//  * deadline here, entirely within src/policy/.
//  *
//  * "depth == 1" reliably signals the start of a fresh move's
//  * thinking: every call site (ubgi.cpp's do_search loop AND
//  * src/benchmark.cpp's timing loop) always begins at depth 1
//  * before trying deeper values. We reset our internal clock then,
//  * and derive each subsequent depth's remaining budget from how
//  * much wall-clock time has elapsed since that reset — so depth
//  * N+1 only gets whatever budget is left, not a fresh 2 seconds.
//  *
//  * Crucially: time_budget_exceeded() never touches ctx.stop. See
//  * submission.hpp for why that matters.
//  *============================================================*/
// static std::chrono::steady_clock::time_point g_move_deadline =
//     std::chrono::steady_clock::time_point::max();

// static inline void maybe_start_move_clock(int depth, const SubmissionParams& p){
//     if(depth <= 1){
//         auto margin = std::chrono::milliseconds(std::min(150, p.move_budget_ms / 10));
//         g_move_deadline = std::chrono::steady_clock::now()
//             + std::chrono::milliseconds(p.move_budget_ms) - margin;
//     }
// }

// /* Cheap to call every node: only actually reads the clock every
//  * 1024 calls (tracked via the node counter already kept in ctx). */
// static inline bool time_budget_exceeded(SearchContext& ctx){
//     if(ctx.stop){
//         return true; /* still respect a genuine external stop/quit */
//     }
//     if((ctx.nodes & 0x3FFULL) != 0){
//         return false;
//     }
//     return std::chrono::steady_clock::now() >= g_move_deadline;
// }


// /*============================================================
//  * Transposition table
//  *
//  * A fixed-size, "always replace" hash table keyed by the
//  * position's Zobrist hash (State::hash(), which already folds
//  * in side-to-move). Each slot stores enough to (a) reuse an
//  * exact/bounded score without re-searching, and (b) hint the
//  * best move found last time for move ordering even when the
//  * stored depth is too shallow to trust the score itself.
//  *
//  * Scoped as static tables in this translation unit: they persist
//  * across the whole iterative-deepening ladder for one `go` (so
//  * depth N's results immediately help order depth N+1) and across
//  * moves in the same game, without needing any change to the
//  * generic SearchContext used by other algorithms. Entries are
//  * always validated against the full stored hash before use, so
//  * stale entries from an earlier game/position are simply
//  * ignored — never a correctness risk, only a (harmless) wasted
//  * lookup.
//  *============================================================*/
// enum class TTFlag : uint8_t { EXACT, LOWERBOUND, UPPERBOUND, EMPTY };

// struct TTEntry {
//     uint64_t hash = 0;
//     int16_t depth = -1;
//     int32_t score = 0;
//     TTFlag flag = TTFlag::EMPTY;
//     Move best_move;
// };

// static constexpr size_t TT_SIZE = 1u << 20; /* ~1M entries, power of two */
// static std::vector<TTEntry> g_tt(TT_SIZE);

// static inline TTEntry* tt_probe(uint64_t hash){
//     TTEntry& e = g_tt[hash & (TT_SIZE - 1)];
//     if(e.flag != TTFlag::EMPTY && e.hash == hash){
//         return &e;
//     }
//     return nullptr;
// }

// static inline void tt_store(uint64_t hash, int depth, int score, TTFlag flag, const Move& best_move){
//     TTEntry& e = g_tt[hash & (TT_SIZE - 1)];
//     if(e.flag == TTFlag::EMPTY || e.hash == hash || depth >= e.depth){
//         e.hash = hash;
//         e.depth = (int16_t)depth;
//         e.score = score;
//         e.flag = flag;
//         e.best_move = best_move;
//     }
// }

// /*============================================================
//  * Killer moves + history heuristic
//  *
//  * Killer moves: up to 2 non-capture moves per ply that previously
//  * caused a beta cutoff. Tried right after the TT move and captures,
//  * since a move that cut off a sibling node at the same ply is often
//  * good here too (same side to move, similar local threats).
//  *
//  * History heuristic: a [piece][to_row][to_col] table accumulating
//  * "how often (weighted by depth^2) has moving this piece to this
//  * square caused a cutoff", used to break ties among the remaining
//  * non-capture, non-killer moves.
//  *============================================================*/
// static constexpr int MAX_PLY = 128;
// static Move g_killers[MAX_PLY][2];
// static int g_history[7][BOARD_H][BOARD_W];

// static inline void record_killer(int ply, const Move& m){
//     if(ply < 0 || ply >= MAX_PLY){
//         return;
//     }
//     if(g_killers[ply][0] == m){
//         return;
//     }
//     g_killers[ply][1] = g_killers[ply][0];
//     g_killers[ply][0] = m;
// }

// static inline bool is_killer(int ply, const Move& m){
//     if(ply < 0 || ply >= MAX_PLY){
//         return false;
//     }
//     return g_killers[ply][0] == m || g_killers[ply][1] == m;
// }

// static inline void record_history(State* state, const Move& m, int depth){
//     int piece = state->piece_at(state->player, m.first.first, m.first.second);
//     if(piece <= 0 || piece > 6){
//         return;
//     }
//     int& slot = g_history[piece][m.second.first][m.second.second];
//     slot += depth * depth;
//     if(slot > 1000000){
//         for(int pc = 0; pc < 7; pc++){
//             for(int r = 0; r < BOARD_H; r++){
//                 for(int c = 0; c < BOARD_W; c++){
//                     g_history[pc][r][c] /= 2;
//                 }
//             }
//         }
//     }
// }

// void Submission::clear_tables(){
//     std::fill(g_tt.begin(), g_tt.end(), TTEntry{});
//     std::memset(g_killers, 0, sizeof(g_killers));
//     std::memset(g_history, 0, sizeof(g_history));
// }


// /*============================================================
//  * Move ordering — TT move > MVV-LVA captures > killers > history
//  *
//  * Computes each move's priority exactly once (a handful of
//  * piece_at() virtual calls + table lookups), then sorts by that
//  * precomputed key. The previous version computed all of this
//  * *inside* the std::stable_sort comparator, which gets called
//  * O(n log n) times for n moves -- e.g. ~20 moves means roughly
//  * 80-90 comparisons, each redoing up to 4 piece_at() virtual
//  * calls plus killer/history lookups for moves whose tier never
//  * changes between comparisons. order_moves() runs on essentially
//  * every node of the tree (main search and quiescence both call
//  * it), so that redundant work was paid for at every single node.
//  *
//  * Tiers (descending key = higher priority), matching the original
//  * comparator's behavior exactly: TT move > captures (by MVV-LVA)
//  * > killers (by history) > everything else (by history). TIER is
//  * comfortably larger than the max possible secondary score (MVV-LVA
//  * tops out at PIECE_VALUES[king]*16 ~= 14400; history saturates
//  * just above 1,000,000 before record_history()'s halving kicks in),
//  * so tier always dominates regardless of the secondary value.
//  *============================================================*/
// static constexpr int MOVE_ORDER_TIER = 1u << 22; /* ~4.19M, well above any secondary score */

// static inline int move_order_key(
//     const State* state,
//     const Move& m,
//     int self,
//     int oppn,
//     int ply,
//     const Move* tt_move
// ){
//     if(tt_move && m == *tt_move){
//         return 3 * MOVE_ORDER_TIER;
//     }

//     int victim = state->piece_at(oppn, m.second.first, m.second.second);
//     if(victim){
//         int attacker = state->piece_at(self, m.first.first, m.first.second);
//         int mvv_lva = PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//         return 2 * MOVE_ORDER_TIER + mvv_lva;
//     }

//     int piece = state->piece_at(self, m.first.first, m.first.second);
//     int hist = (piece > 0 && piece <= 6) ? g_history[piece][m.second.first][m.second.second] : 0;

//     if(is_killer(ply, m)){
//         return 1 * MOVE_ORDER_TIER + hist;
//     }
//     return hist;
// }

// static void order_moves(
//     State* state,
//     std::vector<Move>& moves,
//     int ply,
//     const Move* tt_move
// ){
//     int self = state->player;
//     int oppn = 1 - self;

//     static thread_local std::vector<std::pair<int, Move>> scored;
//     scored.clear();
//     scored.reserve(moves.size());
//     for(auto& m : moves){
//         scored.emplace_back(move_order_key(state, m, self, oppn, ply, tt_move), m);
//     }

//     std::stable_sort(scored.begin(), scored.end(),
//         [](const std::pair<int, Move>& a, const std::pair<int, Move>& b){
//             return a.first > b.first;
//         });

//     for(size_t i = 0; i < moves.size(); i++){
//         moves[i] = scored[i].second;
//     }
// }

// /* Is `m` a capture in `state`? (destination square holds an opponent piece) */
// static inline bool is_capture(const State* state, const Move& m){
//     return state->piece_at(1 - state->player, m.second.first, m.second.second) != 0;
// }

// /* Does the side to move have any piece other than pawns/king? Null-move
//  * pruning assumes "passing" can only be at least as good as some real
//  * move would be for the side that's NOT moving, which can fail in
//  * zugzwang-prone king-and-pawn endgames (where having to move at all is
//  * a disadvantage). We simply disable null-move pruning in that specific
//  * situation rather than try to detect zugzwang precisely. */
// static inline bool has_major_or_minor_piece(const State* state, int player){
//     for(int r = 0; r < BOARD_H; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             int piece = state->piece_at(player, r, c);
//             if(piece != 0 && piece != 1 && piece != 6){ /* not empty, not pawn, not king */
//                 return true;
//             }
//         }
//     }
//     return false;
// }


// /*============================================================
//  * king_in_check() — is `player`'s king currently attacked?
//  *
//  * This game uses a king-capture win model with no separate "check"
//  * legality rule, so nothing stops the side to move from walking
//  * past (or failing to notice) a threat to its own king other than
//  * the search actually looking deep enough to see the capture. That
//  * is fine for the main search (it explores every legal reply), but
//  * quiescence() only ever looks at *captures* -- if our king is
//  * attacked and the only way out is a non-capturing move (stepping
//  * the king away, blocking, etc.), quiescence can't find it and we
//  * fall back on the static eval exactly when it matters most.
//  *
//  * Used for two standard, well-bounded fixes below: (1) a one-ply
//  * "check extension" in pvs() so a position where the side to move
//  * is in check gets a full extra ply of real search instead of
//  * dropping into quiescence early, and (2) quiescence() searching
//  * *all* legal moves (not just captures) on the rare nodes where the
//  * side to move is in check.
//  *
//  * Implemented entirely via the public `piece_at()` interface (no
//  * state.cpp/state.hpp changes needed) -- mirrors normal chess attack
//  * patterns for this 6x5 board, early-returning as soon as any
//  * attacker is found.
//  *============================================================*/
// static bool king_in_check(const State* state, int player){
//     int kr = -1, kc = -1;
//     for(int r = 0; r < BOARD_H && kr < 0; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             if(state->piece_at(player, r, c) == 6){
//                 kr = r; kc = c;
//                 break;
//             }
//         }
//     }
//     if(kr < 0){
//         return false; /* no king on board -- shouldn't happen mid-game */
//     }

//     int opp = 1 - player;

//     /* Pawn attacks: an opponent pawn attacks diagonally toward `player`'s
//      * side of the board. Player 0 (white) pawns advance toward row 0, so
//      * they attack from row+1; player 1 (black) pawns advance toward
//      * row BOARD_H-1, so they attack from row-1. */
//     int pawn_from_dr = (opp == 0) ? 1 : -1;
//     for(int dc = -1; dc <= 1; dc += 2){
//         int pr = kr + pawn_from_dr, pc = kc + dc;
//         if(pr >= 0 && pr < BOARD_H && pc >= 0 && pc < BOARD_W
//            && state->piece_at(opp, pr, pc) == 1){
//             return true;
//         }
//     }

//     /* Knight attacks */
//     static const int kn_dr[8] = {1, 1, -1, -1, 2, 2, -2, -2};
//     static const int kn_dc[8] = {2, -2, 2, -2, 1, -1, 1, -1};
//     for(int i = 0; i < 8; i++){
//         int r = kr + kn_dr[i], c = kc + kn_dc[i];
//         if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W
//            && state->piece_at(opp, r, c) == 3){
//             return true;
//         }
//     }

//     /* King adjacency (kings can capture kings in this ruleset too) */
//     for(int dr = -1; dr <= 1; dr++){
//         for(int dc = -1; dc <= 1; dc++){
//             if(dr == 0 && dc == 0){ continue; }
//             int r = kr + dr, c = kc + dc;
//             if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W
//                && state->piece_at(opp, r, c) == 6){
//                 return true;
//             }
//         }
//     }

//     /* Sliding attacks: rook/queen along ranks+files, bishop/queen on
//      * diagonals. Stop at the first piece found in each direction. */
//     static const int rk_dr[4] = {0, 0, 1, -1};
//     static const int rk_dc[4] = {1, -1, 0, 0};
//     for(int i = 0; i < 4; i++){
//         int r = kr + rk_dr[i], c = kc + rk_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)){
//                 break; /* own piece blocks the line */
//             }
//             int op = state->piece_at(opp, r, c);
//             if(op){
//                 if(op == 2 || op == 5){ return true; } /* rook or queen */
//                 break;
//             }
//             r += rk_dr[i]; c += rk_dc[i];
//         }
//     }
//     static const int bs_dr[4] = {1, 1, -1, -1};
//     static const int bs_dc[4] = {1, -1, 1, -1};
//     for(int i = 0; i < 4; i++){
//         int r = kr + bs_dr[i], c = kc + bs_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)){
//                 break;
//             }
//             int op = state->piece_at(opp, r, c);
//             if(op){
//                 if(op == 4 || op == 5){ return true; } /* bishop or queen */
//                 break;
//             }
//             r += bs_dr[i]; c += bs_dc[i];
//         }
//     }

//     return false;
// }


// /*============================================================
//  * Quiescence search
//  *
//  * Called once the normal search depth is exhausted. Instead of
//  * returning evaluate() directly (which can badly misjudge a
//  * position mid-capture-sequence — the "horizon effect"), we keep
//  * searching captures only until the position is "quiet", and use
//  * a stand-pat score as both the leaf value and the lower bound
//  * (we are never forced to capture, so we can always choose to
//  * stop and take the static score instead).
//  *============================================================*/
// int Submission::quiescence(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     int qply,
//     SearchContext& ctx,
//     const SubmissionParams& p
// ){
//     ctx.nodes++;
//     if(ply > ctx.seldepth){
//         ctx.seldepth = ply;
//     }
//     if(time_budget_exceeded(ctx)){
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }

//     if(state->legal_actions.empty() && state->game_state == UNKNOWN){
//         state->get_legal_actions();
//     }

//     if(state->game_state == WIN){
//         return P_MAX - ply;
//     }
//     if(state->game_state == DRAW){
//         return 0;
//     }

//     /* Is the side to move's king currently attacked? If so, "doing
//      * nothing" (i.e. trusting the static stand-pat score as a safe
//      * floor, the way normal quiescence does) is not a sound
//      * assumption -- the side to move MUST address the threat, and
//      * the only way to find out whether it can is to actually look at
//      * every legal reply, not just captures. */
//     bool in_check = king_in_check(state, state->player);

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);

//     if(!in_check){
//         if(stand_pat >= beta){
//             return stand_pat;
//         }
//         if(stand_pat > alpha){
//             alpha = stand_pat;
//         }
//         if(qply >= p.max_quiescence_ply){
//             return stand_pat;
//         }
//     }else if(qply >= p.max_quiescence_ply){
//         /* Same cap as the normal capture-only path -- searching all
//          * legal moves while in check is more expensive per node, so
//          * it gets no *extra* budget on top of the existing tunable
//          * limit, only the existing one applied uniformly. */
//         return stand_pat;
//     }

//     std::vector<Move> moves_to_search;
//     if(in_check){
//         moves_to_search = state->legal_actions;
//     }else{
//         moves_to_search.reserve(state->legal_actions.size());
//         for(auto& m : state->legal_actions){
//             if(is_capture(state, m)){
//                 moves_to_search.push_back(m);
//             }
//         }
//         if(moves_to_search.empty()){
//             return stand_pat;
//         }
//     }
//     order_moves(state, moves_to_search, -1, nullptr);

//     int best_score = in_check ? M_MAX : stand_pat;

//     for(auto& action : moves_to_search){
//         if(time_budget_exceeded(ctx)){
//             return best_score;
//         }

//         State* next = state->next_state(action);
//         if(next->legal_actions.empty() && next->game_state == UNKNOWN){
//             next->get_legal_actions();
//         }

//         bool same = next->same_player_as_parent();
//         int raw = quiescence(next, same ? alpha : -beta, same ? beta : -alpha,
//                               history, ply + 1, qply + 1, ctx, p);
//         int score = same ? raw : -raw;

//         delete next;

//         if(score > best_score){
//             best_score = score;
//         }
//         if(score > alpha){
//             alpha = score;
//         }
//         if(alpha >= beta){
//             break;
//         }
//     }

//     return best_score;
// }


// /*============================================================
//  * PVS — negamax + alpha-beta + principal variation search
//  *============================================================*/
// int Submission::pvs(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const SubmissionParams& p,
//     bool allow_null,
//     int ext_count
// ){
//     ctx.nodes++;
//     if(ply > ctx.seldepth){
//         ctx.seldepth = ply;
//     }
//     if(time_budget_exceeded(ctx)){
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }

//     if(state->legal_actions.empty() && state->game_state == UNKNOWN){
//         state->get_legal_actions();
//     }
//     if(state->game_state == WIN){
//         return P_MAX - ply;
//     }
//     if(state->game_state == DRAW){
//         return 0;
//     }
//     int rep_score;
//     if(state->check_repetition(history, rep_score)){
//         return rep_score;
//     }

//     /* === Check extension ===
//      * If the side to move is in check, give this node one extra ply
//      * of real, full-width search instead of letting the forced
//      * response run off the end of the main search and into
//      * quiescence (which, even with the check-evasion handling added
//      * above, is still a narrower search than the main loop). Capped
//      * via ext_count so a long forcing sequence can't inflate the
//      * effective depth without limit -- the wall-clock budget is the
//      * final backstop regardless. */
//     bool in_check = king_in_check(state, state->player);
//     if(in_check && ext_count < 3){
//         depth += 1;
//         ext_count += 1;
//     }

//     /* === Transposition table probe ===
//      * Conservative by design: we only ever trust a stored EXACT
//      * value, and only when it was computed at >= the current depth
//      * (so it's at least as informed as a fresh search would be).
//      * We deliberately do NOT use LOWERBOUND/UPPERBOUND entries to
//      * narrow alpha/beta — a bound proved under one (alpha,beta)
//      * window is only safe to reuse for an immediate cutoff under a
//      * *different* window in certain careful conditions, and getting
//      * that subtly wrong silently corrupts the search (verified by
//      * testing against exhaustive minimax). The move-ordering hint
//      * from ANY entry (exact or bound) is still always safe to use,
//      * since it's just a suggestion, not a score we trust blindly. */
//     int orig_alpha = alpha;

//     uint64_t h = state->hash();
//     const Move* tt_move = nullptr;
//     if(p.use_tt){
//         TTEntry* tte = tt_probe(h);
//         if(tte){
//             tt_move = &tte->best_move;
//             if(tte->flag == TTFlag::EXACT && tte->depth >= depth){
//                 return tte->score;
//             }
//         }
//     }

//     history.push(state->hash());

//     if(depth <= 0){
//         int score;
//         if(p.use_quiescence){
//             score = quiescence(state, alpha, beta, history, ply, 0, ctx, p);
//         }else{
//             score = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//         }
//         history.pop(state->hash());
//         return score;
//     }

//     /* === Null-move pruning ===
//      * Idea: if we could "pass" (give the opponent a free move) and a
//      * reduced-depth search of the resulting position still says the
//      * opponent can't do better than beta, then a real move is almost
//      * certainly at least that good too — so we can skip the full
//      * search of this node entirely. Guarded against the classic
//      * failure mode (zugzwang, where passing is illegal but would
//      * actually be the best option) by requiring the side to move to
//      * have at least one non-pawn, non-king piece, and by never trying
//      * two null moves in a row (allow_null). Also skipped near the
//      * top of the tree (depth too low to reduce) and when beta is
//      * already close to a mate score (reduced-depth null search isn't
//      * trustworthy there). */
//     if(p.use_null_move && allow_null && !in_check && depth > p.null_move_reduction
//        && beta < P_MAX - 1000 && beta > M_MAX + 1000
//        && has_major_or_minor_piece(state, state->player)){
//         State* null_state = static_cast<State*>(state->create_null_state());
//         if(null_state->legal_actions.empty() && null_state->game_state == UNKNOWN){
//             null_state->get_legal_actions();
//         }
//         bool same = null_state->same_player_as_parent();
//         int null_depth = depth - 1 - p.null_move_reduction;
//         int child_alpha = same ? (beta - 1) : -beta;
//         int child_beta  = same ?  beta      : -(beta - 1);
//         int raw = pvs(null_state, null_depth, child_alpha, child_beta, history, ply + 1, ctx, p, false, ext_count);
//         int null_score = same ? raw : -raw;
//         delete null_state;

//         if(!time_budget_exceeded(ctx) && null_score >= beta){
//             history.pop(state->hash());
//             return null_score; /* fail-soft cutoff */
//         }
//     }

//     std::vector<Move> moves = state->legal_actions;
//     order_moves(state, moves, ply, tt_move);

//     int best_score = M_MAX;
//     Move best_local_move = moves[0];
//     bool first = true;
//     int quiet_index = 0;
//     bool any_move_evaluated = false;

//     for(auto& action : moves){
//         if(time_budget_exceeded(ctx)){
//             break;
//         }

//         State* next = state->next_state(action);
//         if(next->legal_actions.empty() && next->game_state == UNKNOWN){
//             next->get_legal_actions();
//         }

//         bool same = next->same_player_as_parent();
//         int score;
//         bool is_quiet = !is_capture(state, action) && !(tt_move && action == *tt_move);

//         if(first){
//             int child_alpha = same ? alpha : -beta;
//             int child_beta  = same ? beta  : -alpha;
//             int raw = pvs(next, depth - 1, child_alpha, child_beta, history, ply + 1, ctx, p, true, ext_count);
//             score = same ? raw : -raw;
//         }else{
//             /* === Late move reductions ===
//              * Quiet moves (not a capture, not the TT-hinted move) that
//              * sort late in our ordering are, empirically, rarely the
//              * best move once ordering is decent. Search them first at
//              * a reduced depth with the usual null window; if that
//              * reduced search unexpectedly beats alpha, we don't yet
//              * trust it (it was a shallower, less accurate look), so we
//              * re-search at full depth before deciding whether it also
//              * warrants the full PVS re-search below. */
//             int reduction = 0;
//             if(p.use_lmr && is_quiet && !in_check && depth >= p.lmr_min_depth
//                && quiet_index >= p.lmr_min_move_index){
//                 reduction = (quiet_index >= p.lmr_min_move_index + 4) ? 2 : 1;
//                 if(reduction >= depth){
//                     reduction = depth - 1;
//                 }
//             }

//             int child_alpha = same ?  alpha      : -(alpha + 1);
//             int child_beta  = same ? (alpha + 1) : -alpha;
//             int raw = pvs(next, depth - 1 - reduction, child_alpha, child_beta, history, ply + 1, ctx, p, true, ext_count);
//             score = same ? raw : -raw;

//             if(reduction > 0 && score > alpha){
//                 /* Reduced search beat alpha: redo at full depth (still
//                  * null window) before trusting it enough to consider
//                  * a full-window re-search. */
//                 raw = pvs(next, depth - 1, child_alpha, child_beta, history, ply + 1, ctx, p, true, ext_count);
//                 score = same ? raw : -raw;
//             }

//             if(score > alpha && score < beta){
//                 /* Scout failed high inside the real window: the move
//                  * might actually be the new best, so re-search it
//                  * properly with the full window to get an exact value. */
//                 int re_child_alpha = same ? alpha : -beta;
//                 int re_child_beta  = same ? beta  : -alpha;
//                 raw = pvs(next, depth - 1, re_child_alpha, re_child_beta, history, ply + 1, ctx, p, true, ext_count);
//                 score = same ? raw : -raw;
//             }
//         }

//         delete next;

//         any_move_evaluated = true;
//         if(is_quiet){
//             quiet_index++;
//         }

//         if(score > best_score){
//             best_score = score;
//             best_local_move = action;
//         }
//         if(score > alpha){
//             alpha = score;
//         }
//         if(alpha >= beta){
//             if(is_quiet){
//                 record_killer(ply, action);
//                 record_history(state, action, depth);
//             }
//             break;
//         }
//         first = false;
//     }

//     if(!any_move_evaluated){
//         /* Time ran out before we could evaluate even the first child
//          * (can happen if a deep, heavily-extended forcing line burned
//          * the whole budget a few levels down before unwinding back up
//          * to here). best_score is still its unset M_MAX sentinel at
//          * this point -- returning that raw would leak a bogus "this is
//          * a forced loss" signal up the tree instead of a real number.
//          * Fall back to a real static evaluation, exactly like the
//          * time-budget check at the very top of this function does. */
//         history.pop(state->hash());
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }

//     if(p.use_tt){
//         TTFlag flag;
//         if(best_score <= orig_alpha){
//             flag = TTFlag::UPPERBOUND;
//         }else if(best_score >= beta){
//             flag = TTFlag::LOWERBOUND;
//         }else{
//             flag = TTFlag::EXACT;
//         }
//         tt_store(h, depth, best_score, flag, best_local_move);
//     }

//     history.pop(state->hash());
//     return best_score;
// }


// /*============================================================
//  * Submission — search
//  *
//  * Root driver. Called once per iterative-deepening depth by
//  * ubgi.cpp (always starting at depth==1 for a fresh move — see
//  * the time-budget note at the top of this file).
//  *
//  * Note: Aspiration Windows were tried here and measured to be a
//  * net slowdown on this small board (depth 11 on startpos: 1341ms
//  * with vs 807ms without) — the crude "redo the whole root loop"
//  * retry on a fail-high/low cost more than the narrow window
//  * saved, since scores fluctuate enough between iterations on a
//  * 6x5 board that aspiration misses are common. Removed.
//  *============================================================*/
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ){
//     ctx.reset();
//     SubmissionParams p = SubmissionParams::from_map(ctx.params);
//     maybe_start_move_clock(depth, p);

//     SearchResult result;
//     result.depth = depth;

//     if(!state->legal_actions.size()){
//         state->get_legal_actions();
//     }

//     if(state->game_state == WIN){
//         result.best_move = state->legal_actions[0];
//         result.score = P_MAX;
//         result.nodes = ctx.nodes;
//         result.pv = {result.best_move};
//         return result;
//     }

//     std::vector<Move> moves = state->legal_actions;
//     uint64_t root_hash = state->hash();
//     const Move* tt_move = nullptr;
//     if(p.use_tt){
//         TTEntry* tte = tt_probe(root_hash);
//         if(tte){
//             tt_move = &tte->best_move;
//         }
//     }
//     order_moves(state, moves, 0, tt_move);

//     /* Safe fallback: always have a legal move to return, even if the
//      * very first child search gets interrupted before completing.
//      * best_score starts at a sentinel far below any real evaluation
//      * so a real (possibly very bad, e.g. forced-loss) searched score
//      * always wins the comparison below; we only fall back to a real
//      * static eval for the *reported* score if no move was evaluated
//      * at all (only possible if our budget had already elapsed when
//      * this call started). */
//     result.best_move = moves[0];
//     bool any_move_evaluated = false;

//     int alpha = M_MAX - 10;
//     int beta  = P_MAX + 10;
//     int orig_alpha = alpha;
//     int best_score = M_MAX - 10;
//     int move_index = 0;
//     int total_moves = (int)moves.size();
//     bool first = true;

//     for(auto& action : moves){
//         if(time_budget_exceeded(ctx)){
//             break; /* don't start a move we might not finish evaluating */
//         }

//         State* next = state->next_state(action);
//         if(next->legal_actions.empty() && next->game_state == UNKNOWN){
//             next->get_legal_actions();
//         }

//         bool same = next->same_player_as_parent();
//         int score;

//         if(first){
//             int child_alpha = same ? alpha : -beta;
//             int child_beta  = same ? beta  : -alpha;
//             int raw = Submission::pvs(next, depth - 1, child_alpha, child_beta, history, 1, ctx, p);
//             score = same ? raw : -raw;
//         }else{
//             int child_alpha = same ?  alpha      : -(alpha + 1);
//             int child_beta  = same ? (alpha + 1) : -alpha;
//             int raw = Submission::pvs(next, depth - 1, child_alpha, child_beta, history, 1, ctx, p);
//             score = same ? raw : -raw;

//             if(score > alpha && score < beta){
//                 int re_child_alpha = same ? alpha : -beta;
//                 int re_child_beta  = same ? beta  : -alpha;
//                 raw = Submission::pvs(next, depth - 1, re_child_alpha, re_child_beta, history, 1, ctx, p);
//                 score = same ? raw : -raw;
//             }
//         }

//         delete next;

//         any_move_evaluated = true;
//         if(score > best_score){
//             best_score = score;
//             result.best_move = action;

//             if(p.report_partial && ctx.on_root_update){
//                 ctx.on_root_update({result.best_move, best_score, depth, move_index + 1, total_moves});
//             }
//         }
//         if(score > alpha){
//             alpha = score;
//         }

//         first = false;
//         move_index++;
//     }

//     if(!any_move_evaluated){
//         best_score = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }else if(p.use_tt){
//         TTFlag flag = (best_score <= orig_alpha) ? TTFlag::UPPERBOUND : TTFlag::EXACT;
//         tt_store(root_hash, depth, best_score, flag, result.best_move);
//     }

//     result.score = best_score;
//     result.seldepth = ctx.seldepth;
//     result.nodes = ctx.nodes;
//     result.pv = {result.best_move};

//     return result;
// }


// /*============================================================
//  * Submission — default_params / param_defs
//  *============================================================*/
// ParamMap Submission::default_params(){
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//         {"UseQuiescence", "true"},
//         {"MaxQuiescencePly", "8"},
//         {"UseTT", "true"},
//         {"MoveBudgetMs", "2000"},
//         {"UseNullMove", "true"},
//         {"NullMoveReduction", "3"},
//         {"UseLMR", "true"},
//         {"LMRMinDepth", "3"},
//         {"LMRMinMoveIndex", "3"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs(){
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//         {"UseQuiescence", ParamDef::CHECK, "true"},
//         {"MaxQuiescencePly", ParamDef::SPIN, "8", 0, 32},
//         {"UseTT", ParamDef::CHECK, "true"},
//         {"MoveBudgetMs", ParamDef::SPIN, "2000", 50, 60000},
//         {"UseNullMove", ParamDef::CHECK, "true"},
//         {"NullMoveReduction", ParamDef::SPIN, "3", 1, 5},
//         {"UseLMR", ParamDef::CHECK, "true"},
//         {"LMRMinDepth", ParamDef::SPIN, "3", 1, 10},
//         {"LMRMinMoveIndex", ParamDef::SPIN, "3", 0, 20},
//     };
// }











// #include <algorithm>
// #include <chrono>
// #include <cstring>
// #include <vector>

// #include "state.hpp"
// #include "config.hpp"
// #include "submission.hpp"

// /*============================================================
//  * Move time budget
//  *
//  * ubgi.cpp calls Submission::search() once per iterative-deepening
//  * depth and lets it run to completion; it only checks wall-clock
//  * time *between* depths. We track our own deadline here so a single
//  * deep iteration can't blow past the per-move budget.
//  *
//  * depth == 1 marks the start of a fresh move, so we reset the clock
//  * then; each later depth gets whatever time is left, not a fresh
//  * budget.
//  *============================================================*/

// static std::chrono::steady_clock::time_point g_deadline =
//     std::chrono::steady_clock::time_point::max();

// static inline void start_move_clock(int depth, const SubmissionParams& params){
//     if(depth <= 1){
//         auto margin = std::chrono::milliseconds(std::min(150, params.move_budget_ms / 10));
//         g_deadline = std::chrono::steady_clock::now()
//                    + std::chrono::milliseconds(params.move_budget_ms)
//                    - margin;
//     }
// }

// /* Cheap to call every node: only reads the clock every 1024 calls. */
// static inline bool out_of_time(SearchContext& ctx){
//     if(ctx.stop){
//         return true;
//     }
//     if((ctx.nodes & 0x3FFULL) != 0){
//         return false;
//     }
//     return std::chrono::steady_clock::now() >= g_deadline;
// }

// /*============================================================
//  * Transposition table
//  *
//  * Fixed-size, always-replace table keyed by State::hash(). Stores a
//  * score (when trustworthy) and a best-move hint for move ordering.
//  * Entries are validated against the full hash before use, so stale
//  * entries are simply ignored.
//  *============================================================*/

// enum class Bound : uint8_t { EXACT, LOWER, UPPER, NONE };

// struct TTEntry {
//     uint64_t key   = 0;
//     int16_t  depth = -1;
//     int32_t  score = 0;
//     Bound    bound = Bound::NONE;
//     Move     best_move;
// };

// static constexpr size_t TT_SIZE = 1u << 20; // ~1M entries, power of two
// static std::vector<TTEntry> g_tt(TT_SIZE);

// static inline TTEntry* tt_find(uint64_t key){
//     TTEntry& entry = g_tt[key & (TT_SIZE - 1)];
//     if(entry.bound != Bound::NONE && entry.key == key){
//         return &entry;
//     }
//     return nullptr;
// }

// static inline void tt_store(uint64_t key, int depth, int score, Bound bound, const Move& best_move){
//     TTEntry& entry = g_tt[key & (TT_SIZE - 1)];
//     if(entry.bound == Bound::NONE || entry.key == key || depth >= entry.depth){
//         entry.key   = key;
//         entry.depth = (int16_t)depth;
//         entry.score = score;
//         entry.bound = bound;
//         entry.best_move = best_move;
//     }
// }

// /*============================================================
//  * Killer moves + history heuristic
//  *
//  * Killers: up to 2 quiet moves per ply that previously caused a
//  * beta cutoff. History: [piece][to_row][to_col] cutoff weight,
//  * used to break ties among remaining quiet moves.
//  *============================================================*/

// static constexpr int MAX_PLY = 128;
// static Move g_killers[MAX_PLY][2];
// static int  g_history[7][BOARD_H][BOARD_W];

// static inline void add_killer(int ply, const Move& move){
//     if(ply < 0 || ply >= MAX_PLY){
//         return;
//     }
//     if(g_killers[ply][0] == move){
//         return;
//     }
//     g_killers[ply][1] = g_killers[ply][0];
//     g_killers[ply][0] = move;
// }

// static inline bool is_killer(int ply, const Move& move){
//     if(ply < 0 || ply >= MAX_PLY){
//         return false;
//     }
//     return g_killers[ply][0] == move || g_killers[ply][1] == move;
// }

// static inline void add_history(State* state, const Move& move, int depth){
//     int piece = state->piece_at(state->player, move.first.first, move.first.second);
//     if(piece <= 0 || piece > 6){
//         return;
//     }
//     int& slot = g_history[piece][move.second.first][move.second.second];
//     slot += depth * depth;
//     if(slot > 1000000){
//         for(int pc = 0; pc < 7; pc++){
//             for(int r = 0; r < BOARD_H; r++){
//                 for(int c = 0; c < BOARD_W; c++){
//                     g_history[pc][r][c] /= 2;
//                 }
//             }
//         }
//     }
// }

// void Submission::clear_tables(){
//     std::fill(g_tt.begin(), g_tt.end(), TTEntry{});
//     std::memset(g_killers, 0, sizeof(g_killers));
//     std::memset(g_history, 0, sizeof(g_history));
// }

// /*============================================================
//  * Move ordering — TT move > MVV-LVA captures > killers > history
//  *
//  * Each move's priority is computed once, then sorted by that key
//  * (cheaper than recomputing it inside the comparator on every
//  * std::stable_sort comparison).
//  *
//  * TIER is comfortably larger than the max possible secondary score
//  * (MVV-LVA tops out around PIECE_VALUES[king]*16; history saturates
//  * just above 1,000,000), so tier always dominates.
//  *============================================================*/

// static constexpr int ORDER_TIER = 1u << 22; // ~4.19M, above any secondary score

// static inline int move_score(
//     const State* state, const Move& move,
//     int self, int opponent, int ply, const Move* tt_move
// ){
//     if(tt_move && move == *tt_move){
//         return 3 * ORDER_TIER;
//     }

//     int victim = state->piece_at(opponent, move.second.first, move.second.second);
//     if(victim){
//         int attacker = state->piece_at(self, move.first.first, move.first.second);
//         int mvv_lva = PIECE_VALUES[victim] * 16 - PIECE_VALUES[attacker];
//         return 2 * ORDER_TIER + mvv_lva;
//     }

//     int piece = state->piece_at(self, move.first.first, move.first.second);
//     int hist = (piece > 0 && piece <= 6) ? g_history[piece][move.second.first][move.second.second] : 0;

//     if(is_killer(ply, move)){
//         return 1 * ORDER_TIER + hist;
//     }
//     return hist;
// }

// static void order_moves(State* state, std::vector<Move>& moves, int ply, const Move* tt_move){
//     int self = state->player;
//     int opponent = 1 - self;

//     static thread_local std::vector<std::pair<int, Move>> scored;
//     scored.clear();
//     scored.reserve(moves.size());
//     for(auto& move : moves){
//         scored.emplace_back(move_score(state, move, self, opponent, ply, tt_move), move);
//     }

//     std::stable_sort(scored.begin(), scored.end(),
//         [](const auto& a, const auto& b){ return a.first > b.first; });

//     for(size_t i = 0; i < moves.size(); i++){
//         moves[i] = scored[i].second;
//     }
// }

// static inline bool is_capture(const State* state, const Move& move){
//     return state->piece_at(1 - state->player, move.second.first, move.second.second) != 0;
// }

// /* Does `player` have any piece other than pawns/king? Used to disable
//  * null-move pruning in king-and-pawn endgames, where passing can be
//  * illegal-in-spirit (zugzwang) rather than merely sub-optimal. */
// static inline bool has_major_or_minor_piece(const State* state, int player){
//     for(int r = 0; r < BOARD_H; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             int piece = state->piece_at(player, r, c);
//             if(piece != 0 && piece != 1 && piece != 6){ // not empty, pawn, or king
//                 return true;
//             }
//         }
//     }
//     return false;
// }

// /*============================================================
//  * king_in_check() — is player's king currently attacked?
//  *
//  * This game has no separate "check" legality rule, so nothing stops
//  * a player from walking into a king capture other than the search
//  * actually seeing it. The main search explores every legal reply
//  * regardless, but quiescence() only looks at captures, so a king
//  * under attack with only a non-capturing escape would be invisible
//  * to it. This function backs two fixes: a one-ply check extension
//  * in pvs(), and quiescence() searching all legal moves (not just
//  * captures) when the side to move is in check.
//  *
//  * Implemented via the public piece_at() interface only, mirroring
//  * normal chess attack patterns for this 6x5 board.
//  *============================================================*/

// static bool king_in_check(const State* state, int player){
//     int king_r = -1, king_c = -1;
//     for(int r = 0; r < BOARD_H && king_r < 0; r++){
//         for(int c = 0; c < BOARD_W; c++){
//             if(state->piece_at(player, r, c) == 6){
//                 king_r = r;
//                 king_c = c;
//                 break;
//             }
//         }
//     }
//     if(king_r < 0){
//         return false; // no king on board -- shouldn't happen mid-game
//     }

//     int opponent = 1 - player;

//     // Pawn attacks: pawns attack diagonally toward the player's side.
//     int pawn_from_dr = (opponent == 0) ? 1 : -1;
//     for(int dc = -1; dc <= 1; dc += 2){
//         int r = king_r + pawn_from_dr, c = king_c + dc;
//         if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W &&
//            state->piece_at(opponent, r, c) == 1){
//             return true;
//         }
//     }

//     // Knight attacks
//     static const int knight_dr[8] = {1, 1, -1, -1, 2, 2, -2, -2};
//     static const int knight_dc[8] = {2, -2, 2, -2, 1, -1, 1, -1};
//     for(int i = 0; i < 8; i++){
//         int r = king_r + knight_dr[i], c = king_c + knight_dc[i];
//         if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W &&
//            state->piece_at(opponent, r, c) == 3){
//             return true;
//         }
//     }

//     // Adjacent enemy king (kings can capture kings in this ruleset)
//     for(int dr = -1; dr <= 1; dr++){
//         for(int dc = -1; dc <= 1; dc++){
//             if(dr == 0 && dc == 0){
//                 continue;
//             }
//             int r = king_r + dr, c = king_c + dc;
//             if(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W &&
//                state->piece_at(opponent, r, c) == 6){
//                 return true;
//             }
//         }
//     }

//     // Sliding attacks: rook/queen on ranks+files, bishop/queen on diagonals.
//     static const int rook_dr[4] = {0, 0, 1, -1};
//     static const int rook_dc[4] = {1, -1, 0, 0};
//     for(int i = 0; i < 4; i++){
//         int r = king_r + rook_dr[i], c = king_c + rook_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)){
//                 break; // own piece blocks the line
//             }
//             int piece = state->piece_at(opponent, r, c);
//             if(piece){
//                 if(piece == 2 || piece == 5){ // rook or queen
//                     return true;
//                 }
//                 break;
//             }
//             r += rook_dr[i];
//             c += rook_dc[i];
//         }
//     }

//     static const int bishop_dr[4] = {1, 1, -1, -1};
//     static const int bishop_dc[4] = {1, -1, 1, -1};
//     for(int i = 0; i < 4; i++){
//         int r = king_r + bishop_dr[i], c = king_c + bishop_dc[i];
//         while(r >= 0 && r < BOARD_H && c >= 0 && c < BOARD_W){
//             if(state->piece_at(player, r, c)){
//                 break;
//             }
//             int piece = state->piece_at(opponent, r, c);
//             if(piece){
//                 if(piece == 4 || piece == 5){ // bishop or queen
//                     return true;
//                 }
//                 break;
//             }
//             r += bishop_dr[i];
//             c += bishop_dc[i];
//         }
//     }

//     return false;
// }

// /*============================================================
//  * Quiescence search
//  *
//  * Keeps searching captures past the normal horizon until the
//  * position is quiet, using a stand-pat score as both the leaf value
//  * and a lower bound (we're never forced to capture).
//  *============================================================*/

// int Submission::quiescence(
//     State *state, int alpha, int beta, GameHistory& history,
//     int ply, int qply, SearchContext& ctx, const SubmissionParams& p
// ){
//     ctx.nodes++;
//     if(ply > ctx.seldepth){
//         ctx.seldepth = ply;
//     }
//     if(out_of_time(ctx)){
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }
//     if(state->legal_actions.empty() && state->game_state == UNKNOWN){
//         state->get_legal_actions();
//     }
//     if(state->game_state == WIN){
//         return P_MAX - ply;
//     }
//     if(state->game_state == DRAW){
//         return 0;
//     }

//     bool in_check = king_in_check(state, state->player);
//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);

//     if(!in_check){
//         if(stand_pat >= beta){
//             return stand_pat;
//         }
//         if(stand_pat > alpha){
//             alpha = stand_pat;
//         }
//         if(qply >= p.max_quiescence_ply){
//             return stand_pat;
//         }
//     }else if(qply >= p.max_quiescence_ply){
//         return stand_pat;
//     }

//     std::vector<Move> candidates;
//     if(in_check){
//         candidates = state->legal_actions;
//     }else{
//         candidates.reserve(state->legal_actions.size());
//         for(auto& move : state->legal_actions){
//             if(is_capture(state, move)){
//                 candidates.push_back(move);
//             }
//         }
//         if(candidates.empty()){
//             return stand_pat;
//         }
//     }
//     order_moves(state, candidates, -1, nullptr);

//     int best_score = in_check ? M_MAX : stand_pat;
//     for(auto& move : candidates){
//         if(out_of_time(ctx)){
//             return best_score;
//         }

//         State* child = state->next_state(move);
//         if(child->legal_actions.empty() && child->game_state == UNKNOWN){
//             child->get_legal_actions();
//         }
//         bool same_side = child->same_player_as_parent();
//         int raw = quiescence(child,
//                               same_side ? alpha : -beta,
//                               same_side ? beta : -alpha,
//                               history, ply + 1, qply + 1, ctx, p);
//         int score = same_side ? raw : -raw;
//         delete child;

//         if(score > best_score){
//             best_score = score;
//         }
//         if(score > alpha){
//             alpha = score;
//         }
//         if(alpha >= beta){
//             break;
//         }
//     }
//     return best_score;
// }

// /*============================================================
//  * PVS — negamax + alpha-beta + principal variation search
//  *============================================================*/

// int Submission::pvs(
//     State *state, int depth, int alpha, int beta, GameHistory& history,
//     int ply, SearchContext& ctx, const SubmissionParams& p,
//     bool allow_null, int ext_count
// ){
//     ctx.nodes++;
//     if(ply > ctx.seldepth){
//         ctx.seldepth = ply;
//     }
//     if(out_of_time(ctx)){
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }
//     if(state->legal_actions.empty() && state->game_state == UNKNOWN){
//         state->get_legal_actions();
//     }
//     if(state->game_state == WIN){
//         return P_MAX - ply;
//     }
//     if(state->game_state == DRAW){
//         return 0;
//     }

//     int rep_score;
//     if(state->check_repetition(history, rep_score)){
//         return rep_score;
//     }

//     // Check extension: give a real extra ply instead of dropping into
//     // quiescence while in check. Capped by ext_count.
//     bool in_check = king_in_check(state, state->player);
//     if(in_check && ext_count < 3){
//         depth += 1;
//         ext_count += 1;
//     }

//     // TT probe: only trust a stored EXACT value at >= current depth.
//     // LOWER/UPPER bounds are never used to cut early (unsafe across
//     // different alpha/beta windows) -- the move hint from any entry
//     // is still safe to use for ordering.
//     int orig_alpha = alpha;
//     uint64_t key = state->hash();
//     const Move* tt_move = nullptr;
//     if(p.use_tt){
//         TTEntry* entry = tt_find(key);
//         if(entry){
//             tt_move = &entry->best_move;
//             if(entry->bound == Bound::EXACT && entry->depth >= depth){
//                 return entry->score;
//             }
//         }
//     }

//     history.push(state->hash());

//     if(depth <= 0){
//         int score;
//         if(p.use_quiescence){
//             score = quiescence(state, alpha, beta, history, ply, 0, ctx, p);
//         }else{
//             score = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//         }
//         history.pop(state->hash());
//         return score;
//     }

//     // Null-move pruning: if passing still lets the opponent fail to
//     // beat beta at a reduced depth, a real move is almost certainly
//     // at least that good too. Guarded against zugzwang and disabled
//     // near mate scores or two null moves in a row.
//     if(p.use_null_move && allow_null && !in_check &&
//        depth > p.null_move_reduction &&
//        beta < P_MAX - 1000 && beta > M_MAX + 1000 &&
//        has_major_or_minor_piece(state, state->player)){

//         State* null_state = static_cast<State*>(state->create_null_state());
//         if(null_state->legal_actions.empty() && null_state->game_state == UNKNOWN){
//             null_state->get_legal_actions();
//         }
//         bool same_side = null_state->same_player_as_parent();
//         int null_depth = depth - 1 - p.null_move_reduction;
//         int child_alpha = same_side ? (beta - 1) : -beta;
//         int child_beta  = same_side ? beta : -(beta - 1);

//         int raw = pvs(null_state, null_depth, child_alpha, child_beta,
//                        history, ply + 1, ctx, p, false, ext_count);
//         int null_score = same_side ? raw : -raw;
//         delete null_state;

//         if(!out_of_time(ctx) && null_score >= beta){
//             history.pop(state->hash());
//             return null_score; // fail-soft cutoff
//         }
//     }

//     std::vector<Move> moves = state->legal_actions;
//     order_moves(state, moves, ply, tt_move);

//     int best_score = M_MAX;
//     Move best_move = moves[0];
//     bool first = true;
//     int quiet_index = 0;
//     bool any_move_evaluated = false;

//     for(auto& move : moves){
//         if(out_of_time(ctx)){
//             break;
//         }

//         State* child = state->next_state(move);
//         if(child->legal_actions.empty() && child->game_state == UNKNOWN){
//             child->get_legal_actions();
//         }
//         bool same_side = child->same_player_as_parent();
//         bool is_quiet = !is_capture(state, move) && !(tt_move && move == *tt_move);
//         int score;

//         if(first){
//             int child_alpha = same_side ? alpha : -beta;
//             int child_beta  = same_side ? beta : -alpha;
//             int raw = pvs(child, depth - 1, child_alpha, child_beta,
//                           history, ply + 1, ctx, p, true, ext_count);
//             score = same_side ? raw : -raw;
//         }else{
//             // Late move reductions: search late, quiet moves at reduced
//             // depth first; re-search at full depth/window if they beat
//             // alpha, since the reduced look isn't trusted on its own.
//             int reduction = 0;
//             if(p.use_lmr && is_quiet && !in_check &&
//                depth >= p.lmr_min_depth && quiet_index >= p.lmr_min_move_index){
//                 reduction = (quiet_index >= p.lmr_min_move_index + 4) ? 2 : 1;
//                 if(reduction >= depth){
//                     reduction = depth - 1;
//                 }
//             }

//             int child_alpha = same_side ? alpha : -(alpha + 1);
//             int child_beta  = same_side ? (alpha + 1) : -alpha;
//             int raw = pvs(child, depth - 1 - reduction, child_alpha, child_beta,
//                           history, ply + 1, ctx, p, true, ext_count);
//             score = same_side ? raw : -raw;

//             if(reduction > 0 && score > alpha){
//                 raw = pvs(child, depth - 1, child_alpha, child_beta,
//                           history, ply + 1, ctx, p, true, ext_count);
//                 score = same_side ? raw : -raw;
//             }
//             if(score > alpha && score < beta){
//                 int re_child_alpha = same_side ? alpha : -beta;
//                 int re_child_beta  = same_side ? beta : -alpha;
//                 raw = pvs(child, depth - 1, re_child_alpha, re_child_beta,
//                           history, ply + 1, ctx, p, true, ext_count);
//                 score = same_side ? raw : -raw;
//             }
//         }
//         delete child;
//         any_move_evaluated = true;

//         if(is_quiet){
//             quiet_index++;
//         }
//         if(score > best_score){
//             best_score = score;
//             best_move = move;
//         }
//         if(score > alpha){
//             alpha = score;
//         }
//         if(alpha >= beta){
//             if(is_quiet){
//                 add_killer(ply, move);
//                 add_history(state, move, depth);
//             }
//             break;
//         }
//         first = false;
//     }

//     if(!any_move_evaluated){
//         // Time ran out before the first child could be evaluated;
//         // best_score is still its M_MAX sentinel, so fall back to a
//         // real static eval instead of leaking a bogus forced-loss score.
//         history.pop(state->hash());
//         return state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }

//     if(p.use_tt){
//         Bound bound;
//         if(best_score <= orig_alpha){
//             bound = Bound::UPPER;
//         }else if(best_score >= beta){
//             bound = Bound::LOWER;
//         }else{
//             bound = Bound::EXACT;
//         }
//         tt_store(key, depth, best_score, bound, best_move);
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// /*============================================================
//  * Submission — search
//  *
//  * Root driver, called once per iterative-deepening depth.
//  *
//  * Note: aspiration windows were tried here and measured as a net
//  * slowdown on this small board (depth 11 on startpos: 1341ms with
//  * vs 807ms without), since scores fluctuate enough on a 6x5 board
//  * that fail-high/low retries cost more than the narrow window saves.
//  *============================================================*/

// SearchResult Submission::search(
//     State *state, int depth, GameHistory& history, SearchContext& ctx
// ){
//     ctx.reset();
//     SubmissionParams p = SubmissionParams::from_map(ctx.params);
//     start_move_clock(depth, p);

//     SearchResult result;
//     result.depth = depth;

//     if(!state->legal_actions.size()){
//         state->get_legal_actions();
//     }
//     if(state->game_state == WIN){
//         result.best_move = state->legal_actions[0];
//         result.score = P_MAX;
//         result.nodes = ctx.nodes;
//         result.pv = {result.best_move};
//         return result;
//     }

//     std::vector<Move> moves = state->legal_actions;
//     uint64_t root_key = state->hash();
//     const Move* tt_move = nullptr;
//     if(p.use_tt){
//         TTEntry* entry = tt_find(root_key);
//         if(entry){
//             tt_move = &entry->best_move;
//         }
//     }
//     order_moves(state, moves, 0, tt_move);

//     // Always have a legal move to return, even if the first child
//     // search gets interrupted before completing.
//     result.best_move = moves[0];
//     bool any_move_evaluated = false;

//     int alpha = M_MAX - 10;
//     int beta  = P_MAX + 10;
//     int orig_alpha = alpha;
//     int best_score = M_MAX - 10;
//     int move_index = 0;
//     int total_moves = (int)moves.size();
//     bool first = true;

//     for(auto& move : moves){
//         if(out_of_time(ctx)){
//             break; // don't start a move we might not finish evaluating
//         }

//         State* child = state->next_state(move);
//         if(child->legal_actions.empty() && child->game_state == UNKNOWN){
//             child->get_legal_actions();
//         }
//         bool same_side = child->same_player_as_parent();
//         int score;

//         if(first){
//             int child_alpha = same_side ? alpha : -beta;
//             int child_beta  = same_side ? beta : -alpha;
//             int raw = Submission::pvs(child, depth - 1, child_alpha, child_beta,
//                                        history, 1, ctx, p);
//             score = same_side ? raw : -raw;
//         }else{
//             int child_alpha = same_side ? alpha : -(alpha + 1);
//             int child_beta  = same_side ? (alpha + 1) : -alpha;
//             int raw = Submission::pvs(child, depth - 1, child_alpha, child_beta,
//                                        history, 1, ctx, p);
//             score = same_side ? raw : -raw;

//             if(score > alpha && score < beta){
//                 int re_child_alpha = same_side ? alpha : -beta;
//                 int re_child_beta  = same_side ? beta : -alpha;
//                 raw = Submission::pvs(child, depth - 1, re_child_alpha, re_child_beta,
//                                        history, 1, ctx, p);
//                 score = same_side ? raw : -raw;
//             }
//         }
//         delete child;
//         any_move_evaluated = true;

//         if(score > best_score){
//             best_score = score;
//             result.best_move = move;
//             if(p.report_partial && ctx.on_root_update){
//                 ctx.on_root_update({result.best_move, best_score, depth, move_index + 1, total_moves});
//             }
//         }
//         if(score > alpha){
//             alpha = score;
//         }
//         first = false;
//         move_index++;
//     }

//     if(!any_move_evaluated){
//         best_score = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     }else if(p.use_tt){
//         Bound bound = (best_score <= orig_alpha) ? Bound::UPPER : Bound::EXACT;
//         tt_store(root_key, depth, best_score, bound, result.best_move);
//     }

//     result.score = best_score;
//     result.seldepth = ctx.seldepth;
//     result.nodes = ctx.nodes;
//     result.pv = {result.best_move};
//     return result;
// }

// /*============================================================
//  * Submission — default_params / param_defs
//  *============================================================*/

// ParamMap Submission::default_params(){
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//         {"UseQuiescence", "true"},
//         {"MaxQuiescencePly", "8"},
//         {"UseTT", "true"},
//         {"MoveBudgetMs", "2000"},
//         {"UseNullMove", "true"},
//         {"NullMoveReduction", "3"},
//         {"UseLMR", "true"},
//         {"LMRMinDepth", "3"},
//         {"LMRMinMoveIndex", "3"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs(){
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//         {"UseQuiescence", ParamDef::CHECK, "true"},
//         {"MaxQuiescencePly", ParamDef::SPIN, "8", 0, 32},
//         {"UseTT", ParamDef::CHECK, "true"},
//         {"MoveBudgetMs", ParamDef::SPIN, "2000", 50, 60000},
//         {"UseNullMove", ParamDef::CHECK, "true"},
//         {"NullMoveReduction", ParamDef::SPIN, "3", 1, 5},
//         {"UseLMR", ParamDef::CHECK, "true"},
//         {"LMRMinDepth", ParamDef::SPIN, "3", 1, 10},
//         {"LMRMinMoveIndex", ParamDef::SPIN, "3", 0, 20},
//     };
// }












// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 💡 引入時間庫
// #include <iostream> // 💡 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 💡 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 800.0; 

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 💡 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
    
//     // 💡 每 1024 個節點檢查一次時間，超時就安全中斷，保留實力
//     if (ctx.nodes % 1024 == 0) {
//         if (is_time_up()) {
//             ctx.stop = true;
//         }
//     }
//     if (ctx.stop) return alpha; 

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_val = simple_material[state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second]];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
    
//     // 💡 內層每 1024 節點做一次超時檢查，避免大腦陷入無盡的思考
//     if (ctx.nodes % 1024 == 0) {
//         if (is_time_up()) {
//             ctx.stop = true;
//         }
//     }
//     if (ctx.stop) return alpha;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;
        
//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);
    
//     // 💡 每次搜尋開始前，重設計時器起點（解決找不到成員的編譯錯誤）
//     g_search_start_time = std::chrono::steady_clock::now();

//     SearchResult final_result;
    
//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;
//     final_result.depth = 1;

//     int total_moves = (int)state->legal_actions.size();
//     int max_search_depth = (depth == 0) ? 64 : depth;

//     for (int current_depth = 1; current_depth <= max_search_depth; ++current_depth) {
        
//         if (ctx.stop || is_time_up()) break; 

//         int best_score = -100000000;
//         int alpha = -100000000;
//         int beta = 100000000;
//         int move_index = 0;

//         std::vector<ScoredAction> scored_moves;
//         scored_moves.reserve(state->legal_actions.size());
//         int opp = 1 - state->player;

//         for (auto& action : state->legal_actions) {
//             int score = 0;
//             // 💡 搬出前一層的最優步，排在最前面，讓 PVS 剪枝效率提升到 99%！
//             if (action == final_result.best_move) {
//                 score = 10000000; 
//             } else {
//                 int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//                 if (captured > 0) score += 10000 + captured * 100;
//                 int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//                 if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                     score += 5000;
//                 }
//             }
//             scored_moves.push_back({action, score});
//         }
//         std::sort(scored_moves.begin(), scored_moves.end());

//         bool first_move = true;
//         Move best_move_this_depth = final_result.best_move;

//         for (auto& sm : scored_moves) {
//             if (ctx.stop || is_time_up()) {
//                 ctx.stop = true;
//                 break;
//             }

//             State* next = state->next_state(sm.action);
//             bool same = next->same_player_as_parent();
//             int score;

//             if (first_move) {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//                 first_move = false;
//             } else {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                     }
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                     }
//                 }
//             }
//             delete next;

//             if (!ctx.stop && !is_time_up()) {
//                 if (score > best_score) {
//                     best_score = score;
//                     best_move_this_depth = sm.action;
//                 }
//                 if (best_score > alpha) alpha = best_score;
//             }
//             move_index++;
//         }

//         // 💡 只有整層算完且沒超時，我們才認可這一層
//         if (!ctx.stop && !is_time_up()) {
//             final_result.best_move = best_move_this_depth;
//             final_result.score = best_score;
//             final_result.depth = current_depth;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({final_result.best_move, best_score, current_depth, move_index, total_moves});
//             }
//         } else {
//             // 超時了，退回上一層 100% 完整且精確的最優步
//             break; 
//         }
//     }

//     // history.pop(state->hash());
//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 💡【強力沖刷】：強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }
// // 燈泡









// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 引入時間庫
// #include <iostream> // 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 800.0; 

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;

//     /* IMPORTANT: ctx.stop is owned by ubgi.cpp's do_search()/alive().
//      * It means "search was externally cancelled — suppress bestmove".
//      * Our own 800ms soft-timeout must NOT write to it, or the final
//      * `if(alive()) send("bestmove ...")` in do_search() will see
//      * ctx.stop == true and silently drop the bestmove even though
//      * nothing external asked to stop. We only ever READ ctx.stop
//      * here (to respect a real external stop), and signal our own
//      * timeout purely via early return. */
//     if (ctx.stop) return alpha;

//     // 每 1024 個節點檢查一次時間，超時就安全返回，保留目前最佳估計
//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             int attacker_val = simple_material[attacker_piece];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     // 同上：只讀取 ctx.stop（尊重外部真正的 stop/quit），自己的
//     // 800ms 超時判斷一律用 return 處理，絕不寫入 ctx.stop。
//     if (ctx.stop) return alpha;

//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;

//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         // 在展開下一個子節點前也檢查一次，避免超時後還硬跑一整棵子樹
//         if (ctx.stop || ((ctx.nodes % 1024 == 0) && is_time_up())) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 每次搜尋開始前，重設計時器起點
//     g_search_start_time = std::chrono::steady_clock::now();

//     SearchResult final_result;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;
//     final_result.depth = 1;

//     int total_moves = (int)state->legal_actions.size();
//     int max_search_depth = (depth == 0) ? 64 : depth;

//     for (int current_depth = 1; current_depth <= max_search_depth; ++current_depth) {

//         // 注意：這裡只「讀」ctx.stop / is_time_up()，用來決定要不要
//         // 開始下一層迭代深化，絕不把結果寫回 ctx.stop —— ctx.stop
//         // 是 ubgi.cpp 用來判斷「外部真的要求停止」的旗標，被我們自己
//         // 的 800ms 軟性逾時誤寫的話，do_search() 最後的
//         // `if(alive()) send("bestmove ...")` 會直接判定整個搜尋被
//         // 外部取消，導致 bestmove 完全不送出。
//         if (ctx.stop || is_time_up()) break;

//         int best_score = -100000000;
//         int alpha = -100000000;
//         int beta = 100000000;
//         int move_index = 0;

//         std::vector<ScoredAction> scored_moves;
//         scored_moves.reserve(state->legal_actions.size());
//         int opp = 1 - state->player;

//         for (auto& action : state->legal_actions) {
//             int score = 0;
//             // 搬出前一層的最優步，排在最前面，提升剪枝效率
//             if (action == final_result.best_move) {
//                 score = 10000000; 
//             } else {
//                 int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//                 if (captured > 0) score += 10000 + captured * 100;
//                 int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//                 if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                     score += 5000;
//                 }
//             }
//             scored_moves.push_back({action, score});
//         }
//         std::sort(scored_moves.begin(), scored_moves.end());

//         bool first_move = true;
//         Move best_move_this_depth = final_result.best_move;
//         bool depth_completed = true; // 這一層是否完整跑完所有 root move

//         for (auto& sm : scored_moves) {
//             if (ctx.stop || is_time_up()) {
//                 depth_completed = false;
//                 break;
//             }

//             State* next = state->next_state(sm.action);
//             bool same = next->same_player_as_parent();
//             int score;

//             if (first_move) {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//                 first_move = false;
//             } else {
//                 if (same) {
//                     score = eval_ctx(next, current_depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = eval_ctx(next, current_depth - 1, alpha, beta, history, 1, ctx, p);
//                     }
//                 } else {
//                     score = -eval_ctx(next, current_depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                     if (score > alpha && score < beta) {
//                         score = -eval_ctx(next, current_depth - 1, -beta, -alpha, history, 1, ctx, p);
//                     }
//                 }
//             }
//             delete next;

//             if (!ctx.stop && !is_time_up()) {
//                 if (score > best_score) {
//                     best_score = score;
//                     best_move_this_depth = sm.action;
//                 }
//                 if (best_score > alpha) alpha = best_score;
//             }
//             move_index++;
//         }

//         // 只有整層算完且沒超時，我們才認可這一層的結果
//         if (depth_completed && !ctx.stop && !is_time_up()) {
//             final_result.best_move = best_move_this_depth;
//             final_result.score = best_score;
//             final_result.depth = current_depth;

//             if (p.report_partial && ctx.on_root_update) {
//                 ctx.on_root_update({final_result.best_move, best_score, current_depth, move_index, total_moves});
//             }
//         } else {
//             // 超時了，退回上一層完整且精確的最優步，不採用這一層的結果
//             break;
//         }
//     }

//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }









// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 引入時間庫
// #include <iostream> // 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// //
// // 只在「一個新的 move」開始思考時重設（depth <= 1），後續每一層
// // 共用同一個起點，所以總思考時間是跨層累加的，而不是每呼叫一次
// // search() 就重新給自己 800ms。
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 800.0; 

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;

//     /* IMPORTANT: ctx.stop is owned by ubgi.cpp's do_search()/alive().
//      * It means "search was externally cancelled — suppress bestmove".
//      * Our own 800ms soft-timeout must NOT write to it, or the final
//      * `if(alive()) send("bestmove ...")` in do_search() will see
//      * ctx.stop == true and silently drop the bestmove even though
//      * nothing external asked to stop. We only ever READ ctx.stop
//      * here (to respect a real external stop), and signal our own
//      * timeout purely via early return. */
//     if (ctx.stop) return alpha;

//     // 每 1024 個節點檢查一次時間，超時就安全返回，保留目前最佳估計
//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             int attacker_val = simple_material[attacker_piece];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     // 同上：只讀取 ctx.stop（尊重外部真正的 stop/quit），自己的
//     // 800ms 超時判斷一律用 return 處理，絕不寫入 ctx.stop。
//     if (ctx.stop) return alpha;

//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;

//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         // 在展開下一個子節點前也檢查一次，避免超時後還硬跑一整棵子樹
//         if (ctx.stop || ((ctx.nodes % 1024 == 0) && is_time_up())) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// /*============================================================
//  * Submission::search() — 單一深度搜尋（root driver）
//  *
//  * 重要：這個函式只負責搜尋「傳入的這一個 depth」，不可以自己再跑
//  * 一輪 for(current_depth = 1 .. N) 的迭代深化迴圈。
//  *
//  * 原因：呼叫端 ubgi.cpp 的 do_search() 本身就是：
//  *
//  *     for(int depth = 1; depth <= depth_limit; depth++){
//  *         SearchResult result = g_algo->search(&state, depth, history, ctx);
//  *         ...
//  *     }
//  *
//  * 也就是說，迭代深化的責任已經由 do_search() 的外層迴圈承擔，每次
//  * 呼叫 search() 時 depth 都會從 1 開始遞增傳入。如果 search() 內部
//  * 自己又跑一次完整的 1..depth 迴圈，會造成：
//  *   1. 每次呼叫都重算前面所有層，浪費大量時間
//  *   2. g_search_start_time 在每次呼叫都被重置，導致真正消耗的總
//  *      思考時間遠超過 do_search() 外層以為的進度，最終讓整體搜尋
//  *      時間遠超出裁判端的等待上限，造成 timeout / no bestmove
//  *
//  * 因此這裡只搜「depth」這一層，回傳後交由 do_search() 決定要不要
//  * 再呼叫下一層。
//  *============================================================*/
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 只有「新的一手」開始思考時（depth <= 1）才重設計時器起點，
//     // 後續每一層共用同一個起點，思考時間是跨層累加的。
//     if (depth <= 1) {
//         g_search_start_time = std::chrono::steady_clock::now();
//     }

//     SearchResult final_result;
//     final_result.depth = depth;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;

//     // 如果這一手才剛起步就已經超時（理論上不該發生，但作為安全網），
//     // 直接回傳目前手上最合理的合法步，不要再展開任何節點。
//     if (ctx.stop || is_time_up()) {
//         final_result.nodes = ctx.nodes;
//         final_result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return final_result;
//     }

//     int total_moves = (int)state->legal_actions.size();

//     int best_score = -100000000;
//     int alpha = -100000000;
//     int beta = 100000000;
//     int move_index = 0;

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         // 搬出前一層的最優步，排在最前面，提升剪枝效率
//         if (action == final_result.best_move) {
//             score = 10000000; 
//         } else {
//             int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     bool first_move = true;
//     Move best_move_this_depth = final_result.best_move;
//     bool any_move_evaluated = false;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || is_time_up()) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         any_move_evaluated = true;

//         if (!ctx.stop && !is_time_up()) {
//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//         }
//         move_index++;

//         // 每完成一個 root move 就回報一次部分進度，讓 do_search() 的
//         // on_root_update callback 能即時更新目前找到的最佳手。
//         if (p.report_partial && ctx.on_root_update) {
//             ctx.on_root_update({best_move_this_depth, best_score, depth, move_index, total_moves});
//         }
//     }

//     // 只有真的評估過至少一個 root move，才採用這一層算出的結果；
//     // 否則保留函式一開始設定的 fallback（legal_actions[0]）。
//     if (any_move_evaluated) {
//         final_result.best_move = best_move_this_depth;
//         final_result.score = best_score;
//     }

//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }









// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 引入時間庫
// #include <iostream> // 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// //
// // 只在「一個新的 move」開始思考時重設（depth <= 1），後續每一層
// // 共用同一個起點，所以總思考時間是跨層累加的，而不是每呼叫一次
// // search() 就重新給自己 800ms。
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 500.0; // 調低，留更大安全邊界，避免裁判端 timeout

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;

//     /* IMPORTANT: ctx.stop is owned by ubgi.cpp's do_search()/alive().
//      * It means "search was externally cancelled — suppress bestmove".
//      * Our own 800ms soft-timeout must NOT write to it, or the final
//      * `if(alive()) send("bestmove ...")` in do_search() will see
//      * ctx.stop == true and silently drop the bestmove even though
//      * nothing external asked to stop. We only ever READ ctx.stop
//      * here (to respect a real external stop), and signal our own
//      * timeout purely via early return. */
//     if (ctx.stop) return alpha;

//     // 每 1024 個節點檢查一次時間，超時就安全返回，保留目前最佳估計
//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             int attacker_val = simple_material[attacker_piece];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     // 同上：只讀取 ctx.stop（尊重外部真正的 stop/quit），自己的
//     // 800ms 超時判斷一律用 return 處理，絕不寫入 ctx.stop。
//     if (ctx.stop) return alpha;

//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;

//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         // 在展開下一個子節點前也檢查一次，避免超時後還硬跑一整棵子樹
//         if (ctx.stop || ((ctx.nodes % 1024 == 0) && is_time_up())) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// /*============================================================
//  * Submission::search() — 單一深度搜尋（root driver）
//  *
//  * 重要：這個函式只負責搜尋「傳入的這一個 depth」，不可以自己再跑
//  * 一輪 for(current_depth = 1 .. N) 的迭代深化迴圈。
//  *
//  * 原因：呼叫端 ubgi.cpp 的 do_search() 本身就是：
//  *
//  *     for(int depth = 1; depth <= depth_limit; depth++){
//  *         SearchResult result = g_algo->search(&state, depth, history, ctx);
//  *         ...
//  *     }
//  *
//  * 也就是說，迭代深化的責任已經由 do_search() 的外層迴圈承擔，每次
//  * 呼叫 search() 時 depth 都會從 1 開始遞增傳入。如果 search() 內部
//  * 自己又跑一次完整的 1..depth 迴圈，會造成：
//  *   1. 每次呼叫都重算前面所有層，浪費大量時間
//  *   2. g_search_start_time 在每次呼叫都被重置，導致真正消耗的總
//  *      思考時間遠超過 do_search() 外層以為的進度，最終讓整體搜尋
//  *      時間遠超出裁判端的等待上限，造成 timeout / no bestmove
//  *
//  * 因此這裡只搜「depth」這一層，回傳後交由 do_search() 決定要不要
//  * 再呼叫下一層。
//  *============================================================*/
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 只有「新的一手」開始思考時（depth <= 1）才重設計時器起點，
//     // 後續每一層共用同一個起點，思考時間是跨層累加的。
//     if (depth <= 1) {
//         g_search_start_time = std::chrono::steady_clock::now();
//     }

//     SearchResult final_result;
//     final_result.depth = depth;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;

//     // 如果這一手才剛起步就已經超時（理論上不該發生，但作為安全網），
//     // 直接回傳目前手上最合理的合法步，不要再展開任何節點。
//     if (ctx.stop || is_time_up()) {
//         final_result.nodes = ctx.nodes;
//         final_result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return final_result;
//     }

//     int total_moves = (int)state->legal_actions.size();

//     int best_score = -100000000;
//     int alpha = -100000000;
//     int beta = 100000000;
//     int move_index = 0;

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         // 搬出前一層的最優步，排在最前面，提升剪枝效率
//         if (action == final_result.best_move) {
//             score = 10000000; 
//         } else {
//             int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     bool first_move = true;
//     Move best_move_this_depth = final_result.best_move;
//     bool any_move_evaluated = false;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || is_time_up()) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         any_move_evaluated = true;

//         if (!ctx.stop && !is_time_up()) {
//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//         }
//         move_index++;

//         // 每完成一個 root move 就回報一次部分進度，讓 do_search() 的
//         // on_root_update callback 能即時更新目前找到的最佳手。
//         if (p.report_partial && ctx.on_root_update) {
//             ctx.on_root_update({best_move_this_depth, best_score, depth, move_index, total_moves});
//         }
//     }

//     // 只有真的評估過至少一個 root move，才採用這一層算出的結果；
//     // 否則保留函式一開始設定的 fallback（legal_actions[0]）。
//     if (any_move_evaluated) {
//         final_result.best_move = best_move_this_depth;
//         final_result.score = best_score;
//     }

//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }









// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 引入時間庫
// #include <iostream> // 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// //
// // 只在「一個新的 move」開始思考時重設（depth <= 1），後續每一層
// // 共用同一個起點，所以總思考時間是跨層累加的，而不是每呼叫一次
// // search() 就重新給自己 800ms。
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 500.0; // 調低，留更大安全邊界，避免裁判端 timeout

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;

//     /* IMPORTANT: ctx.stop is owned by ubgi.cpp's do_search()/alive().
//      * It means "search was externally cancelled — suppress bestmove".
//      * Our own 800ms soft-timeout must NOT write to it, or the final
//      * `if(alive()) send("bestmove ...")` in do_search() will see
//      * ctx.stop == true and silently drop the bestmove even though
//      * nothing external asked to stop. We only ever READ ctx.stop
//      * here (to respect a real external stop), and signal our own
//      * timeout purely via early return. */
//     if (ctx.stop) return alpha;

//     // 每 1024 個節點檢查一次時間，超時就安全返回，保留目前最佳估計
//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             int attacker_val = simple_material[attacker_piece];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     // 同上：只讀取 ctx.stop（尊重外部真正的 stop/quit），自己的
//     // 800ms 超時判斷一律用 return 處理，絕不寫入 ctx.stop。
//     if (ctx.stop) return alpha;

//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;

//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         // 在展開下一個子節點前也檢查一次，避免超時後還硬跑一整棵子樹
//         if (ctx.stop || ((ctx.nodes % 1024 == 0) && is_time_up())) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// /*============================================================
//  * Submission::search() — 單一深度搜尋（root driver）
//  *
//  * 重要：這個函式只負責搜尋「傳入的這一個 depth」，不可以自己再跑
//  * 一輪 for(current_depth = 1 .. N) 的迭代深化迴圈。
//  *
//  * 原因：呼叫端 ubgi.cpp 的 do_search() 本身就是：
//  *
//  *     for(int depth = 1; depth <= depth_limit; depth++){
//  *         SearchResult result = g_algo->search(&state, depth, history, ctx);
//  *         ...
//  *     }
//  *
//  * 也就是說，迭代深化的責任已經由 do_search() 的外層迴圈承擔，每次
//  * 呼叫 search() 時 depth 都會從 1 開始遞增傳入。如果 search() 內部
//  * 自己又跑一次完整的 1..depth 迴圈，會造成：
//  *   1. 每次呼叫都重算前面所有層，浪費大量時間
//  *   2. g_search_start_time 在每次呼叫都被重置，導致真正消耗的總
//  *      思考時間遠超過 do_search() 外層以為的進度，最終讓整體搜尋
//  *      時間遠超出裁判端的等待上限，造成 timeout / no bestmove
//  *
//  * 因此這裡只搜「depth」這一層，回傳後交由 do_search() 決定要不要
//  * 再呼叫下一層。
//  *============================================================*/
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 重設計時器起點：do_search() 每次呼叫 search() 時，depth 應該
//     // 從 1 開始遞增，所以原本只在 depth<=1 時重設計時器。但這個判斷
//     // 依賴呼叫端「保證」第一次呼叫一定是 depth==1，一旦這個假設在
//     // 任何情況下不成立（例如某次呼叫被跳過、或計時器殘留了上一手
//     // 的舊時間點），is_time_up() 會立刻回傳 true，導致這一手完全沒
//     // 機會展開任何節點。
//     //
//     // 為了不依賴這個外部假設，這裡額外加一層保險：如果距離上次重設
//     // 已經超過合理的單手上限（這裡用 2 倍時間預算當門檻），也視為
//     // 一手新的開始，強制重設計時器，不讓陳舊的時間點拖累全新的一手。
//     auto now_tp = std::chrono::steady_clock::now();
//     double since_last_reset_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
//         now_tp - g_search_start_time
//     ).count();
//     if (depth <= 1 || since_last_reset_ms < 0 || since_last_reset_ms > g_time_limit_ms * 2) {
//         g_search_start_time = now_tp;
//     }

//     SearchResult final_result;
//     final_result.depth = depth;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;

//     // 如果這一手才剛起步就已經超時（理論上不該發生，但作為安全網），
//     // 直接回傳目前手上最合理的合法步，不要再展開任何節點。
//     if (ctx.stop || is_time_up()) {
//         final_result.nodes = ctx.nodes;
//         final_result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return final_result;
//     }

//     int total_moves = (int)state->legal_actions.size();

//     int best_score = -100000000;
//     int alpha = -100000000;
//     int beta = 100000000;
//     int move_index = 0;

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         // 搬出前一層的最優步，排在最前面，提升剪枝效率
//         if (action == final_result.best_move) {
//             score = 10000000; 
//         } else {
//             int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     bool first_move = true;
//     Move best_move_this_depth = final_result.best_move;
//     bool any_move_evaluated = false;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || is_time_up()) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         any_move_evaluated = true;

//         if (!ctx.stop && !is_time_up()) {
//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//         }
//         move_index++;

//         // 每完成一個 root move 就回報一次部分進度，讓 do_search() 的
//         // on_root_update callback 能即時更新目前找到的最佳手。
//         if (p.report_partial && ctx.on_root_update) {
//             ctx.on_root_update({best_move_this_depth, best_score, depth, move_index, total_moves});
//         }
//     }

//     // 只有真的評估過至少一個 root move，才採用這一層算出的結果；
//     // 否則保留函式一開始設定的 fallback（legal_actions[0]）。
//     if (any_move_evaluated) {
//         final_result.best_move = best_move_this_depth;
//         final_result.score = best_score;
//     }

//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         {"UseEvalMobility", "true"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "true"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }












// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>   // 引入時間庫
// #include <iostream> // 引入標準輸出庫，用來做 Flush

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // 宣告全域時間變數，用來精準控制 800ms 安全線，防止被裁判斷頭
// //
// // 只在「一個新的 move」開始思考時重設（depth <= 1），後續每一層
// // 共用同一個起點，所以總思考時間是跨層累加的，而不是每呼叫一次
// // search() 就重新給自己 800ms。
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 500.0; // 調低，留更大安全邊界，避免裁判端 timeout

// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score;
//     }
// };

// // 快速檢查是否快超時
// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;

//     /* IMPORTANT: ctx.stop is owned by ubgi.cpp's do_search()/alive().
//      * It means "search was externally cancelled — suppress bestmove".
//      * Our own 500ms soft-timeout must NOT write to it, or the final
//      * `if(alive()) send("bestmove ...")` in do_search() will see
//      * ctx.stop == true and silently drop the bestmove even though
//      * nothing external asked to stop. We only ever READ ctx.stop
//      * here (to respect a real external stop), and signal our own
//      * timeout purely via early return. */
//     if (ctx.stop) return alpha;

//     // 每 256 個節點檢查一次時間（原本 1024，縮小間隔以降低風險）。
//     // evaluate() 在 UseEvalMobility 開啟時，內部會額外呼叫一次完整
//     // 的 get_legal_actions()（透過 create_null_state()），單一節點
//     // 的實際耗時可能比預期高出不少，1024 的間隔可能讓單次檢查之間
//     // 累積過多沒被偵測到的時間，縮小間隔讓超時偵測更即時可靠。
//     if ((ctx.nodes % 256 == 0) && is_time_up()) {
//         return alpha;
//     }

//     int stand_pat = state->evaluate(p.use_kp_eval, p.use_eval_mobility, &history);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int captured_piece = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured_piece > 0) {
//             int victim_val = simple_material[captured_piece];
//             int attacker_piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             int attacker_val = simple_material[attacker_piece];
//             int score = victim_val * 100 - attacker_val;
//             captures.push_back({action, score});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;

//     // 同上：只讀取 ctx.stop（尊重外部真正的 stop/quit），自己的
//     // 800ms 超時判斷一律用 return 處理，絕不寫入 ctx.stop。
//     if (ctx.stop) return alpha;

//     if ((ctx.nodes % 1024 == 0) && is_time_up()) {
//         return alpha;
//     }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     if (state->game_state == WIN) return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history, rep_score)) return rep_score;
//     history.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history, ply, ctx, p);
//         history.pop(state->hash());
//         return score;
//     }

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//         if (captured > 0) score += 10000 + captured * 100;

//         int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//         if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//             score += 5000;
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;

//     for (auto& sm : scored_moves) {
//         // 在展開下一個子節點前也檢查一次，避免超時後還硬跑一整棵子樹
//         if (ctx.stop || ((ctx.nodes % 1024 == 0) && is_time_up())) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, ply + 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, ply + 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, ply + 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         if (score > best_score) best_score = score;
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) break;
//     }

//     history.pop(state->hash());
//     return best_score;
// }

// /*============================================================
//  * Submission::search() — 單一深度搜尋（root driver）
//  *
//  * 重要：這個函式只負責搜尋「傳入的這一個 depth」，不可以自己再跑
//  * 一輪 for(current_depth = 1 .. N) 的迭代深化迴圈。
//  *
//  * 原因：呼叫端 ubgi.cpp 的 do_search() 本身就是：
//  *
//  *     for(int depth = 1; depth <= depth_limit; depth++){
//  *         SearchResult result = g_algo->search(&state, depth, history, ctx);
//  *         ...
//  *     }
//  *
//  * 也就是說，迭代深化的責任已經由 do_search() 的外層迴圈承擔，每次
//  * 呼叫 search() 時 depth 都會從 1 開始遞增傳入。如果 search() 內部
//  * 自己又跑一次完整的 1..depth 迴圈，會造成：
//  *   1. 每次呼叫都重算前面所有層，浪費大量時間
//  *   2. g_search_start_time 在每次呼叫都被重置，導致真正消耗的總
//  *      思考時間遠超過 do_search() 外層以為的進度，最終讓整體搜尋
//  *      時間遠超出裁判端的等待上限，造成 timeout / no bestmove
//  *
//  * 因此這裡只搜「depth」這一層，回傳後交由 do_search() 決定要不要
//  * 再呼叫下一層。
//  *============================================================*/
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 重設計時器起點：do_search() 每次呼叫 search() 時，depth 應該
//     // 從 1 開始遞增，所以原本只在 depth<=1 時重設計時器。但這個判斷
//     // 依賴呼叫端「保證」第一次呼叫一定是 depth==1，一旦這個假設在
//     // 任何情況下不成立（例如某次呼叫被跳過、或計時器殘留了上一手
//     // 的舊時間點），is_time_up() 會立刻回傳 true，導致這一手完全沒
//     // 機會展開任何節點。
//     //
//     // 為了不依賴這個外部假設，這裡額外加一層保險：如果距離上次重設
//     // 已經超過合理的單手上限（這裡用 2 倍時間預算當門檻），也視為
//     // 一手新的開始，強制重設計時器，不讓陳舊的時間點拖累全新的一手。
//     auto now_tp = std::chrono::steady_clock::now();
//     double since_last_reset_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
//         now_tp - g_search_start_time
//     ).count();
//     if (depth <= 1 || since_last_reset_ms < 0 || since_last_reset_ms > g_time_limit_ms * 2) {
//         g_search_start_time = now_tp;
//     }

//     SearchResult final_result;
//     final_result.depth = depth;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }
//     final_result.score = 0;

//     // 如果這一手才剛起步就已經超時（理論上不該發生，但作為安全網），
//     // 直接回傳目前手上最合理的合法步，不要再展開任何節點。
//     if (ctx.stop || is_time_up()) {
//         final_result.nodes = ctx.nodes;
//         final_result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return final_result;
//     }

//     int total_moves = (int)state->legal_actions.size();

//     int best_score = -100000000;
//     int alpha = -100000000;
//     int beta = 100000000;
//     int move_index = 0;

//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int score = 0;
//         // 搬出前一層的最優步，排在最前面，提升剪枝效率
//         if (action == final_result.best_move) {
//             score = 10000000; 
//         } else {
//             int captured = state->board.board[opp][(size_t)action.second.first][(size_t)action.second.second];
//             if (captured > 0) score += 10000 + captured * 100;
//             int piece = state->board.board[state->player][(size_t)action.first.first][(size_t)action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == BOARD_H - 1)) {
//                 score += 5000;
//             }
//         }
//         scored_moves.push_back({action, score});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     bool first_move = true;
//     Move best_move_this_depth = final_result.best_move;
//     bool any_move_evaluated = false;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || is_time_up()) {
//             break;
//         }

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//             }
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth - 1, alpha, alpha + 1, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = eval_ctx(next, depth - 1, alpha, beta, history, 1, ctx, p);
//                 }
//             } else {
//                 score = -eval_ctx(next, depth - 1, -alpha - 1, -alpha, history, 1, ctx, p);
//                 if (score > alpha && score < beta) {
//                     score = -eval_ctx(next, depth - 1, -beta, -alpha, history, 1, ctx, p);
//                 }
//             }
//         }
//         delete next;

//         any_move_evaluated = true;

//         if (!ctx.stop && !is_time_up()) {
//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//         }
//         move_index++;

//         // 每完成一個 root move 就回報一次部分進度，讓 do_search() 的
//         // on_root_update callback 能即時更新目前找到的最佳手。
//         if (p.report_partial && ctx.on_root_update) {
//             ctx.on_root_update({best_move_this_depth, best_score, depth, move_index, total_moves});
//         }
//     }

//     // 只有真的評估過至少一個 root move，才採用這一層算出的結果；
//     // 否則保留函式一開始設定的 fallback（legal_actions[0]）。
//     if (any_move_evaluated) {
//         final_result.best_move = best_move_this_depth;
//         final_result.score = best_score;
//     }

//     final_result.nodes = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;

//     // 強制將輸出緩衝區沖進 Python 裁判端，防範傳輸延遲
//     std::cout.flush();

//     return final_result;
// }

// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval", "true"},
//         // 關閉 mobility bonus：State::evaluate() 在 use_mobility=true 時，
//         // 每次呼叫都會額外 new 一個 State 並呼叫 get_legal_actions() 來算
//         // 對手的合法步數量（見 state.cpp 的 create_null_state()），這個
//         // 開銷在每一個搜索葉節點都會發生，且隨局面複雜度（分支因子）
//         // 非線性放大。這是導致某些局面下單層搜索耗時忽然暴增、進而吃光
//         // 時間預算、最終 no bestmove 的關鍵隱藏成本。關掉它換取穩定性。
//         {"UseEvalMobility", "false"},
//         {"ReportPartial", "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval", ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "false"},
//         {"ReportPartial", ParamDef::CHECK, "true"},
//     };
// }











// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <cmath>
// #include <chrono>
// #include <iostream>
// #include <array>

// #include "state.hpp"
// #include "submission.hpp"

// static const int simple_material[7] = {0, 2, 6, 7, 8, 20, 100};

// // ============================================================
// // 時間管理
// // ============================================================
// static std::chrono::steady_clock::time_point g_search_start_time;
// static constexpr double g_time_limit_ms = 450.0; // 保守上限

// static inline bool is_time_up() {
//     auto now = std::chrono::steady_clock::now();
//     auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
//         now - g_search_start_time).count();
//     return elapsed >= g_time_limit_ms;
// }

// // ============================================================
// // Killer moves & History heuristic
// // ============================================================
// static constexpr int MAX_PLY = 64;
// static Move killer[MAX_PLY][2];          // 每層最多存 2 個 killer
// static int  history[2][7][BOARD_H][BOARD_W]; // history[player][piece][to_r][to_c]

// static void clear_killers_and_history() {
//     for (int i = 0; i < MAX_PLY; i++) {
//         killer[i][0] = killer[i][1] = Move();
//     }
//     for (int p = 0; p < 2; p++)
//         for (int t = 0; t < 7; t++)
//             for (int r = 0; r < BOARD_H; r++)
//                 for (int c = 0; c < BOARD_W; c++)
//                     history[p][t][r][c] = 0;
// }

// static void store_killer(int ply, const Move& m) {
//     if (ply >= MAX_PLY) return;
//     if (!(m == killer[ply][0])) {
//         killer[ply][1] = killer[ply][0];
//         killer[ply][0] = m;
//     }
// }

// static void update_history(int player, int piece, const Move& m, int depth) {
//     if (piece < 1 || piece > 6) return;
//     history[player][piece][m.second.first][m.second.second] += depth * depth;
//     // 防止溢出
//     if (history[player][piece][m.second.first][m.second.second] > 1000000)
//         for (int t = 0; t < 7; t++)
//             for (int r = 0; r < BOARD_H; r++)
//                 for (int c = 0; c < BOARD_W; c++)
//                     history[player][t][r][c] /= 2;
// }

// // ============================================================
// // ScoredAction
// // ============================================================
// struct ScoredAction {
//     Move action;
//     int score;
//     bool operator<(const ScoredAction& other) const {
//         return score > other.score; // 大的排前面
//     }
// };

// // ============================================================
// // Move scoring for sorting
// // ============================================================
// static int score_move(
//     State* state,
//     const Move& action,
//     int ply,
//     const Move& tt_move
// ) {
//     if (action == tt_move) return 10000000;

//     int opp = 1 - state->player;
//     int captured = state->board.board[opp][action.second.first][action.second.second];
//     int piece    = state->board.board[state->player][action.first.first][action.first.second];

//     if (captured > 0) {
//         // MVV-LVA: 優先吃大子用小子
//         return 1000000 + simple_material[captured] * 100 - simple_material[piece];
//     }

//     // 升變 bonus
//     if (piece == 1 && (action.second.first == 0 || action.second.first == (size_t)(BOARD_H - 1))) {
//         return 500000;
//     }

//     // Killer heuristic
//     if (ply < MAX_PLY) {
//         if (action == killer[ply][0]) return 400000;
//         if (action == killer[ply][1]) return 300000;
//     }

//     // History heuristic
//     if (piece >= 1 && piece <= 6) {
//         return history[state->player][piece][action.second.first][action.second.second];
//     }

//     return 0;
// }

// // ============================================================
// // Quiescence search（加入深度上限）
// // ============================================================
// int Submission::quiesce(
//     State *state,
//     int alpha,
//     int beta,
//     GameHistory& history_,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ctx.stop) return alpha;

//     // quiesce 深度上限：防止無限遞迴
//     static constexpr int QUIESCE_MAX_PLY = 8;
//     if (ply > QUIESCE_MAX_PLY) {
//         return state->evaluate(p.use_kp_eval, false, &history_);
//     }

//     if ((ctx.nodes & 255) == 0 && is_time_up()) return alpha;

//     int stand_pat = state->evaluate(p.use_kp_eval, false, &history_);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }

//     // 只考慮吃子（captures）
//     std::vector<ScoredAction> captures;
//     captures.reserve(16);
//     int opp = 1 - state->player;

//     for (auto& action : state->legal_actions) {
//         int cap = state->board.board[opp][action.second.first][action.second.second];
//         if (cap > 0) {
//             int attacker = state->board.board[state->player][action.first.first][action.first.second];
//             captures.push_back({action, simple_material[cap] * 100 - simple_material[attacker]});
//         }
//     }
//     std::sort(captures.begin(), captures.end());

//     for (auto& sa : captures) {
//         State* next = state->next_state(sa.action);
//         bool same = next->same_player_as_parent();
//         int score;
//         if (same) {
//             score = quiesce(next, alpha, beta, history_, ply + 1, ctx, p);
//         } else {
//             score = -quiesce(next, -beta, -alpha, history_, ply + 1, ctx, p);
//         }
//         delete next;

//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// // ============================================================
// // 主搜尋
// // ============================================================
// int Submission::eval_ctx(
//     State *state,
//     int depth,
//     int alpha,
//     int beta,
//     GameHistory& history_,
//     int ply,
//     SearchContext& ctx,
//     const MMParams& p
// ) {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 1023) == 0 && is_time_up()) return alpha;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN) {
//         state->get_legal_actions();
//     }
//     if (state->game_state == WIN)  return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep_score;
//     if (state->check_repetition(history_, rep_score)) return rep_score;
//     history_.push(state->hash());

//     if (depth <= 0) {
//         int score = quiesce(state, alpha, beta, history_, 0, ctx, p);
//         history_.pop(state->hash());
//         return score;
//     }

//     // ---- Null Move Pruning ----
//     // 只在 non-PV、depth>=3、且不在 zugzwang 危險局面時做
//     bool in_check = false; // 簡化：先不處理 in-check 偵測
//     if (!in_check && depth >= 3 && beta < 900000) {
//         int R = (depth >= 6) ? 3 : 2;
//         // 建立 null move state（交換手番）
//         BaseState* null_bs = state->create_null_state();
//         State* null_s = static_cast<State*>(null_bs);
//         history_.push(null_s->hash());
//         int null_score = -eval_ctx(null_s, depth - 1 - R, -beta, -beta + 1, history_, ply + 1, ctx, p);
//         history_.pop(null_s->hash());
//         delete null_s;
//         if (null_score >= beta) {
//             history_.pop(state->hash());
//             return beta;
//         }
//     }

//     // ---- Move ordering ----
//     Move tt_move; // 目前沒有 TT，傳空
//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     for (auto& action : state->legal_actions) {
//         scored_moves.push_back({action, score_move(state, action, ply, tt_move)});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score = -100000000;
//     bool first_move = true;
//     Move best_move_local;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || ((ctx.nodes & 1023) == 0 && is_time_up())) break;

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same)  score =  eval_ctx(next, depth-1, alpha, beta, history_, ply+1, ctx, p);
//             else       score = -eval_ctx(next, depth-1, -beta, -alpha, history_, ply+1, ctx, p);
//             first_move = false;
//         } else {
//             // PVS（Principal Variation Search）
//             if (same) {
//                 score = eval_ctx(next, depth-1, alpha, alpha+1, history_, ply+1, ctx, p);
//                 if (score > alpha && score < beta)
//                     score = eval_ctx(next, depth-1, alpha, beta, history_, ply+1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth-1, -alpha-1, -alpha, history_, ply+1, ctx, p);
//                 if (score > alpha && score < beta)
//                     score = -eval_ctx(next, depth-1, -beta, -alpha, history_, ply+1, ctx, p);
//             }
//         }
//         delete next;

//         if (score > best_score) {
//             best_score = score;
//             best_move_local = sm.action;
//         }
//         if (best_score > alpha) alpha = best_score;
//         if (alpha >= beta) {
//             // Beta cutoff：更新 killer & history
//             int captured = state->board.board[1-state->player][sm.action.second.first][sm.action.second.second];
//             if (captured == 0) { // 非吃子的 cutoff 才存 killer
//                 store_killer(ply, sm.action);
//                 int piece = state->board.board[state->player][sm.action.first.first][sm.action.first.second];
//                 update_history(state->player, piece, sm.action, depth);
//             }
//             break;
//         }
//     }

//     history_.pop(state->hash());
//     return best_score;
// }

// // ============================================================
// // Root search（迭代加深由外部 do_search 驅動）
// // ============================================================
// SearchResult Submission::search(
//     State *state,
//     int depth,
//     GameHistory& history_,
//     SearchContext& ctx
// ) {
//     ctx.reset();
//     MMParams p = MMParams::from_map(ctx.params);

//     // 計時器重設
//     auto now_tp = std::chrono::steady_clock::now();
//     double since_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
//         now_tp - g_search_start_time).count();
//     if (depth <= 1 || since_ms < 0 || since_ms > g_time_limit_ms * 2) {
//         g_search_start_time = now_tp;
//         // 新的一手搜尋：清空 killer & history
//         if (depth <= 1) clear_killers_and_history();
//     }

//     SearchResult final_result;
//     final_result.depth = depth;
//     final_result.score = 0;

//     if (!state->legal_actions.empty()) {
//         final_result.best_move = state->legal_actions[0];
//     } else {
//         state->get_legal_actions();
//         if (!state->legal_actions.empty()) final_result.best_move = state->legal_actions[0];
//     }

//     if (ctx.stop || is_time_up()) {
//         final_result.nodes    = ctx.nodes;
//         final_result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return final_result;
//     }

//     int total_moves = (int)state->legal_actions.size();

//     // Root move ordering
//     std::vector<ScoredAction> scored_moves;
//     scored_moves.reserve(state->legal_actions.size());
//     int opp = 1 - state->player;
//     for (auto& action : state->legal_actions) {
//         int s = 0;
//         if (action == final_result.best_move) {
//             s = 10000000;
//         } else {
//             int cap = state->board.board[opp][action.second.first][action.second.second];
//             if (cap > 0) {
//                 int atk = state->board.board[state->player][action.first.first][action.first.second];
//                 s = 1000000 + simple_material[cap] * 100 - simple_material[atk];
//             }
//             int piece = state->board.board[state->player][action.first.first][action.first.second];
//             if (piece == 1 && (action.second.first == 0 || action.second.first == (size_t)(BOARD_H-1)))
//                 s += 500000;
//         }
//         scored_moves.push_back({action, s});
//     }
//     std::sort(scored_moves.begin(), scored_moves.end());

//     int best_score    = -100000000;
//     int alpha         = -100000000;
//     int beta          = 100000000;
//     bool first_move   = true;
//     bool any_evaluated = false;
//     Move best_move_this_depth = final_result.best_move;
//     int move_index = 0;

//     for (auto& sm : scored_moves) {
//         if (ctx.stop || is_time_up()) break;

//         State* next = state->next_state(sm.action);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first_move) {
//             if (same)  score =  eval_ctx(next, depth-1, alpha, beta, history_, 1, ctx, p);
//             else       score = -eval_ctx(next, depth-1, -beta, -alpha, history_, 1, ctx, p);
//             first_move = false;
//         } else {
//             if (same) {
//                 score = eval_ctx(next, depth-1, alpha, alpha+1, history_, 1, ctx, p);
//                 if (score > alpha && score < beta)
//                     score = eval_ctx(next, depth-1, alpha, beta, history_, 1, ctx, p);
//             } else {
//                 score = -eval_ctx(next, depth-1, -alpha-1, -alpha, history_, 1, ctx, p);
//                 if (score > alpha && score < beta)
//                     score = -eval_ctx(next, depth-1, -beta, -alpha, history_, 1, ctx, p);
//             }
//         }
//         delete next;

//         any_evaluated = true;

//         if (!ctx.stop && !is_time_up()) {
//             if (score > best_score) {
//                 best_score = score;
//                 best_move_this_depth = sm.action;
//             }
//             if (best_score > alpha) alpha = best_score;
//         }

//         move_index++;
//         if (p.report_partial && ctx.on_root_update) {
//             ctx.on_root_update({best_move_this_depth, best_score, depth, move_index, total_moves});
//         }
//     }

//     if (any_evaluated) {
//         final_result.best_move = best_move_this_depth;
//         final_result.score     = best_score;
//     }

//     final_result.nodes    = ctx.nodes;
//     final_result.seldepth = ctx.seldepth;
//     std::cout.flush();
//     return final_result;
// }

// // ============================================================
// // Params
// // ============================================================
// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval",       "true"},
//         {"UseEvalMobility", "false"}, // mobility 太貴，關掉
//         {"ReportPartial",   "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval",       ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "false"},
//         {"ReportPartial",   ParamDef::CHECK, "true"},
//     };
// }













// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <iostream>
// #include <cstring>

// #include "state.hpp"
// #include "submission.hpp"

// // ============================================================
// // 材料值
// // ============================================================
// static const int MATERIAL[7] = {0, 2, 6, 7, 8, 20, 100};

// // ============================================================
// // 時間管理
// // ============================================================
// static std::chrono::steady_clock::time_point g_t0;
// static double g_limit_ms = 1600.0; // 2000ms 給 1600ms，留 400ms 安全邊界

// static void reset_timer() {
//     g_t0 = std::chrono::steady_clock::now();
// }

// static double elapsed_ms() {
//     return std::chrono::duration<double, std::milli>(
//         std::chrono::steady_clock::now() - g_t0).count();
// }

// static bool time_up() {
//     return elapsed_ms() >= g_limit_ms;
// }

// // ============================================================
// // Killer moves & History heuristic
// // ============================================================
// static constexpr int MAX_PLY = 64;
// static Move  g_killer[MAX_PLY][2];
// static int   g_history[2][7][BOARD_H][BOARD_W];

// static void init_tables() {
//     for (int i = 0; i < MAX_PLY; i++)
//         g_killer[i][0] = g_killer[i][1] = Move();
//     memset(g_history, 0, sizeof(g_history));
// }

// static void save_killer(int ply, const Move& m) {
//     if (ply < 0 || ply >= MAX_PLY) return;
//     if (!(m == g_killer[ply][0])) {
//         g_killer[ply][1] = g_killer[ply][0];
//         g_killer[ply][0] = m;
//     }
// }

// static void bump_history(int player, const Move& m, int piece, int depth) {
//     if (piece < 1 || piece > 6) return;
//     g_history[player][piece][m.second.first][m.second.second] += depth * depth;
// }

// // ============================================================
// // Move scoring
// // ============================================================
// static int score_move(State* state, const Move& m, int ply, const Move& pv_move) {
//     // PV / hash move
//     if (m == pv_move) return 20000000;

//     int opp  = 1 - state->player;
//     int cap  = state->board.board[opp][m.second.first][m.second.second];
//     int piece = state->board.board[state->player][m.first.first][m.first.second];

//     // 吃子：MVV-LVA
//     if (cap > 0)
//         return 10000000 + MATERIAL[cap] * 100 - MATERIAL[piece];

//     // 升變
//     if (piece == 1 && (m.second.first == 0 || m.second.first == (size_t)(BOARD_H - 1)))
//         return 5000000;

//     // Killer
//     if (ply >= 0 && ply < MAX_PLY) {
//         if (m == g_killer[ply][0]) return 900000;
//         if (m == g_killer[ply][1]) return 800000;
//     }

//     // History
//     if (piece >= 1 && piece <= 6)
//         return g_history[state->player][piece][m.second.first][m.second.second];

//     return 0;
// }

// // ============================================================
// // Quiescence search
// // ============================================================
// static constexpr int Q_DEPTH_LIMIT = 6;

// int Submission::quiesce(
//     State* state, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 511) == 0 && time_up()) { ctx.stop = true; return alpha; }

//     // 深度上限：直接靜態估值
//     if (ply >= Q_DEPTH_LIMIT)
//         return state->evaluate(p.use_kp_eval, false, &hist);

//     int stand_pat = state->evaluate(p.use_kp_eval, false, &hist);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     int opp = 1 - state->player;

//     // 只看吃子，MVV-LVA 排序
//     struct Cap { Move m; int score; };
//     std::vector<Cap> caps;
//     caps.reserve(16);
//     for (auto& a : state->legal_actions) {
//         int cap = state->board.board[opp][a.second.first][a.second.second];
//         if (cap > 0) {
//             int atk = state->board.board[state->player][a.first.first][a.first.second];
//             caps.push_back({a, MATERIAL[cap] * 100 - MATERIAL[atk]});
//         }
//     }
//     std::sort(caps.begin(), caps.end(), [](const Cap& a, const Cap& b){ return a.score > b.score; });

//     for (auto& c : caps) {
//         if (ctx.stop) break;
//         State* next = state->next_state(c.m);
//         bool same = next->same_player_as_parent();
//         int score = same
//             ?  quiesce(next,  alpha,  beta, hist, ply+1, ctx, p)
//             : -quiesce(next, -beta, -alpha, hist, ply+1, ctx, p);
//         delete next;
//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// // ============================================================
// // Alpha-Beta (Negamax + PVS)
// // ============================================================
// int Submission::eval_ctx(
//     State* state, int depth, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 1023) == 0 && time_up()) { ctx.stop = true; return alpha; }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     if (state->game_state == WIN)  return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep;
//     if (state->check_repetition(hist, rep)) return rep;

//     hist.push(state->hash());

//     if (depth <= 0) {
//         int s = quiesce(state, alpha, beta, hist, 0, ctx, p);
//         hist.pop(state->hash());
//         return s;
//     }

//     // Move ordering
//     Move pv_move; // 這裡沒有 TT，傳空
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(state->legal_actions.size());
//     for (auto& a : state->legal_actions)
//         moves.push_back({a, score_move(state, a, ply, pv_move)});
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int best = -100000000;
//     bool first = true;
//     Move best_move;

//     for (auto& sm : moves) {
//         if (ctx.stop) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             first = false;
//         } else {
//             // PVS null-window
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, ply+1, ctx, p);
//             // Re-search if needed
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             }
//         }
//         delete next;

//         if (score > best) { best = score; best_move = sm.m; }
//         if (best > alpha) alpha = best;
//         if (alpha >= beta) {
//             // Beta cutoff
//             int cap = state->board.board[1-state->player][sm.m.second.first][sm.m.second.second];
//             if (cap == 0) {
//                 save_killer(ply, sm.m);
//                 int piece = state->board.board[state->player][sm.m.first.first][sm.m.first.second];
//                 bump_history(state->player, sm.m, piece, depth);
//             }
//             break;
//         }
//     }

//     hist.pop(state->hash());
//     return best;
// }

// // ============================================================
// // Root search
// // ============================================================
// SearchResult Submission::search(
//     State* state, int depth,
//     GameHistory& hist, SearchContext& ctx)
// {
//     MMParams p = MMParams::from_map(ctx.params);
//     ctx.reset();

//     // depth==1 代表這手棋剛開始搜尋，重設計時器和 table
//     if (depth == 1) {
//         reset_timer();
//         init_tables();
//         // 根據 params 決定時間上限（預留 400ms 安全邊界）
//         // 預設 2000ms → 1600ms
//         g_limit_ms = 1600.0;
//     }

//     SearchResult result;
//     result.depth = depth;
//     result.score = 0;

//     if (state->legal_actions.empty()) {
//         state->get_legal_actions();
//     }
//     if (!state->legal_actions.empty())
//         result.best_move = state->legal_actions[0];
//     else {
//         result.nodes = ctx.nodes;
//         result.seldepth = ctx.seldepth;
//         return result;
//     }

//     // 時間已用完，直接回傳 fallback
//     if (ctx.stop || time_up()) {
//         result.nodes = ctx.nodes;
//         result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return result;
//     }

//     int total = (int)state->legal_actions.size();
//     int opp = 1 - state->player;

//     // Root move ordering（前一層最佳步排第一）
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(total);
//     for (auto& a : state->legal_actions) {
//         int s = score_move(state, a, 0, result.best_move);
//         moves.push_back({a, s});
//     }
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int alpha = -100000000, beta = 100000000;
//     int best  = -100000000;
//     bool first = true;
//     bool any = false;
//     Move best_this_depth = result.best_move;
//     int idx = 0;

//     for (auto& sm : moves) {
//         if (ctx.stop || time_up()) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             first = false;
//         } else {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, 1, ctx, p);
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             }
//         }
//         delete next;

//         any = true;
//         if (!ctx.stop) {
//             if (score > best) { best = score; best_this_depth = sm.m; }
//             if (best > alpha) alpha = best;
//         }

//         idx++;
//         if (p.report_partial && ctx.on_root_update)
//             ctx.on_root_update({best_this_depth, best, depth, idx, total});
//     }

//     if (any) {
//         result.best_move = best_this_depth;
//         result.score     = best;
//     }
//     result.nodes    = ctx.nodes;
//     result.seldepth = ctx.seldepth;
//     std::cout.flush();
//     return result;
// }

// // ============================================================
// // Params
// // ============================================================
// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval",       "true"},
//         {"UseEvalMobility", "false"},
//         {"ReportPartial",   "true"},
//     };
// }

// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval",       ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "false"},
//         {"ReportPartial",   ParamDef::CHECK, "true"},
//     };
// }












// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <iostream>
// #include <cstring>

// #include "state.hpp"
// #include "submission.hpp"

// // ============================================================
// // 材料值
// // ============================================================
// static const int MATERIAL[7] = {0, 2, 6, 7, 8, 20, 100};

// // ============================================================
// // 時間管理
// // ============================================================
// static std::chrono::steady_clock::time_point g_t0;
// static constexpr double TIME_LIMIT_MS = 1600.0; // 2000ms 留 400ms 緩衝

// static void reset_timer() {
//     g_t0 = std::chrono::steady_clock::now();
// }
// static double elapsed_ms() {
//     return std::chrono::duration<double, std::milli>(
//         std::chrono::steady_clock::now() - g_t0).count();
// }
// static bool time_up() {
//     return elapsed_ms() >= TIME_LIMIT_MS;
// }

// // ============================================================
// // Transposition Table
// // ============================================================
// enum TTFlag : uint8_t { TT_NONE = 0, TT_EXACT, TT_LOWER, TT_UPPER };

// struct TTEntry {
//     uint64_t key;
//     int      score;
//     Move     best_move;
//     int8_t   depth;
//     TTFlag   flag;
// };

// static constexpr int TT_SIZE = 1 << 20; // 1M entries (~64MB)
// static TTEntry g_tt[TT_SIZE];

// static void tt_clear() {
//     memset(g_tt, 0, sizeof(g_tt));
// }

// static TTEntry* tt_probe(uint64_t key) {
//     TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
//     if (e->key == key && e->flag != TT_NONE) return e;
//     return nullptr;
// }

// static void tt_store(uint64_t key, int score, int depth, TTFlag flag, const Move& best) {
//     TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
//     // 只在 depth 更深或 key 不同時覆蓋
//     if (e->flag == TT_NONE || e->depth <= depth || e->key != key) {
//         e->key       = key;
//         e->score     = score;
//         e->depth     = (int8_t)depth;
//         e->flag      = flag;
//         e->best_move = best;
//     }
// }

// // ============================================================
// // Killer & History
// // ============================================================
// static constexpr int MAX_PLY = 64;
// static Move g_killer[MAX_PLY][2];
// static int  g_history[2][7][BOARD_H][BOARD_W];

// static void init_tables() {
//     for (int i = 0; i < MAX_PLY; i++)
//         g_killer[i][0] = g_killer[i][1] = Move();
//     memset(g_history, 0, sizeof(g_history));
// }

// static void save_killer(int ply, const Move& m) {
//     if (ply < 0 || ply >= MAX_PLY) return;
//     if (!(m == g_killer[ply][0])) {
//         g_killer[ply][1] = g_killer[ply][0];
//         g_killer[ply][0] = m;
//     }
// }
// static void bump_history(int player, int piece, const Move& m, int depth) {
//     if (piece < 1 || piece > 6) return;
//     g_history[player][piece][m.second.first][m.second.second] += depth * depth;
// }

// // ============================================================
// // Move scoring
// // ============================================================
// static int score_move(State* state, const Move& m, int ply, const Move& tt_move) {
//     if (m == tt_move) return 20000000;

//     int opp   = 1 - state->player;
//     int cap   = state->board.board[opp][m.second.first][m.second.second];
//     int piece = state->board.board[state->player][m.first.first][m.first.second];

//     if (cap > 0)
//         return 10000000 + MATERIAL[cap] * 100 - MATERIAL[piece];

//     if (piece == 1 && (m.second.first == 0 || m.second.first == (size_t)(BOARD_H - 1)))
//         return 5000000;

//     if (ply >= 0 && ply < MAX_PLY) {
//         if (m == g_killer[ply][0]) return 900000;
//         if (m == g_killer[ply][1]) return 800000;
//     }

//     if (piece >= 1 && piece <= 6)
//         return g_history[state->player][piece][m.second.first][m.second.second];

//     return 0;
// }

// // ============================================================
// // Quiescence search
// // ============================================================
// static constexpr int Q_LIMIT = 6;

// int Submission::quiesce(
//     State* state, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 511) == 0 && time_up()) { ctx.stop = true; return alpha; }

//     if (ply >= Q_LIMIT)
//         return state->evaluate(p.use_kp_eval, false, &hist);

//     int stand_pat = state->evaluate(p.use_kp_eval, false, &hist);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     int opp = 1 - state->player;
//     struct Cap { Move m; int score; };
//     std::vector<Cap> caps;
//     caps.reserve(16);
//     for (auto& a : state->legal_actions) {
//         int cap = state->board.board[opp][a.second.first][a.second.second];
//         if (cap > 0) {
//             int atk = state->board.board[state->player][a.first.first][a.first.second];
//             caps.push_back({a, MATERIAL[cap] * 100 - MATERIAL[atk]});
//         }
//     }
//     std::sort(caps.begin(), caps.end(), [](const Cap& a, const Cap& b){ return a.score > b.score; });

//     for (auto& c : caps) {
//         if (ctx.stop) break;
//         State* next = state->next_state(c.m);
//         bool same = next->same_player_as_parent();
//         int score = same
//             ?  quiesce(next,  alpha,  beta, hist, ply+1, ctx, p)
//             : -quiesce(next, -beta, -alpha, hist, ply+1, ctx, p);
//         delete next;
//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// // ============================================================
// // Alpha-Beta + PVS + TT
// // ============================================================
// int Submission::eval_ctx(
//     State* state, int depth, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 1023) == 0 && time_up()) { ctx.stop = true; return alpha; }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     if (state->game_state == WIN)  return 1000000 - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep;
//     if (state->check_repetition(hist, rep)) return rep;

//     // TT lookup
//     uint64_t key = state->hash();
//     Move tt_move;
//     TTEntry* tte = tt_probe(key);
//     if (tte) {
//         tt_move = tte->best_move;
//         if (tte->depth >= depth) {
//             int s = tte->score;
//             if (tte->flag == TT_EXACT) { hist.push(key); hist.pop(key); return s; }
//             if (tte->flag == TT_LOWER && s > alpha) alpha = s;
//             if (tte->flag == TT_UPPER && s < beta)  beta  = s;
//             if (alpha >= beta) { hist.push(key); hist.pop(key); return s; }
//         }
//     }

//     hist.push(key);

//     if (depth <= 0) {
//         int s = quiesce(state, alpha, beta, hist, 0, ctx, p);
//         hist.pop(key);
//         return s;
//     }

//     // Move ordering
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(state->legal_actions.size());
//     for (auto& a : state->legal_actions)
//         moves.push_back({a, score_move(state, a, ply, tt_move)});
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int orig_alpha = alpha;
//     int best = -100000000;
//     bool first = true;
//     Move best_move;

//     for (auto& sm : moves) {
//         if (ctx.stop) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             first = false;
//         } else {
//             // PVS null-window
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, ply+1, ctx, p);
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             }
//         }
//         delete next;

//         if (score > best) { best = score; best_move = sm.m; }
//         if (best > alpha) alpha = best;
//         if (alpha >= beta) {
//             int cap = state->board.board[1-state->player][sm.m.second.first][sm.m.second.second];
//             if (cap == 0) {
//                 save_killer(ply, sm.m);
//                 int piece = state->board.board[state->player][sm.m.first.first][sm.m.first.second];
//                 bump_history(state->player, piece, sm.m, depth);
//             }
//             break;
//         }
//     }

//     // TT store
//     if (!ctx.stop) {
//         TTFlag flag;
//         if      (best <= orig_alpha) flag = TT_UPPER;
//         else if (best >= beta)       flag = TT_LOWER;
//         else                         flag = TT_EXACT;
//         tt_store(key, best, depth, flag, best_move);
//     }

//     hist.pop(key);
//     return best;
// }

// // ============================================================
// // Root search
// // ============================================================
// SearchResult Submission::search(
//     State* state, int depth,
//     GameHistory& hist, SearchContext& ctx)
// {
//     MMParams p = MMParams::from_map(ctx.params);
//     ctx.reset();

//     if (depth == 1) {
//         reset_timer();
//         init_tables();
//         tt_clear();
//     }

//     SearchResult result;
//     result.depth = depth;
//     result.score = 0;

//     if (state->legal_actions.empty()) state->get_legal_actions();
//     if (!state->legal_actions.empty())
//         result.best_move = state->legal_actions[0];
//     else {
//         result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//         return result;
//     }

//     if (ctx.stop || time_up()) {
//         result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return result;
//     }

//     int opp = 1 - state->player;
//     int total = (int)state->legal_actions.size();

//     // Root move ordering：前一層 best move 排第一
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(total);
//     for (auto& a : state->legal_actions) {
//         int s;
//         if (a == result.best_move) s = 20000000;
//         else {
//             int cap = state->board.board[opp][a.second.first][a.second.second];
//             int piece = state->board.board[state->player][a.first.first][a.first.second];
//             s = (cap > 0) ? (10000000 + MATERIAL[cap]*100 - MATERIAL[piece]) : 0;
//             if (piece == 1 && (a.second.first == 0 || a.second.first == (size_t)(BOARD_H-1))) s += 5000000;
//         }
//         moves.push_back({a, s});
//     }
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int alpha = -100000000, beta = 100000000, best = -100000000;
//     bool first = true, any = false;
//     Move best_this = result.best_move;
//     int idx = 0;

//     for (auto& sm : moves) {
//         if (ctx.stop || time_up()) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             first = false;
//         } else {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, 1, ctx, p);
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             }
//         }
//         delete next;

//         any = true;
//         if (!ctx.stop) {
//             if (score > best) { best = score; best_this = sm.m; }
//             if (best > alpha) alpha = best;
//         }
//         idx++;
//         if (p.report_partial && ctx.on_root_update)
//             ctx.on_root_update({best_this, best, depth, idx, total});
//     }

//     if (any) { result.best_move = best_this; result.score = best; }
//     result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//     std::cout.flush();
//     return result;
// }

// // ============================================================
// // Params
// // ============================================================
// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval",       "true"},
//         {"UseEvalMobility", "false"},
//         {"ReportPartial",   "true"},
//     };
// }
// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval",       ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "false"},
//         {"ReportPartial",   ParamDef::CHECK, "true"},
//     };
// }







// #include <utility>
// #include <algorithm>
// #include <vector>
// #include <chrono>
// #include <iostream>
// #include <cstring>

// #include "state.hpp"
// #include "submission.hpp"

// // ============================================================
// // 材料值
// // ============================================================
// static const int MATERIAL[7] = {0, 2, 6, 7, 8, 20, 100};

// // ============================================================
// // 時間管理
// // ============================================================
// static std::chrono::steady_clock::time_point g_t0;
// static constexpr double TIME_LIMIT_MS = 1400.0; // 2000ms 留 600ms 緩衝

// static void reset_timer() {
//     g_t0 = std::chrono::steady_clock::now();
// }
// static double elapsed_ms() {
//     return std::chrono::duration<double, std::milli>(
//         std::chrono::steady_clock::now() - g_t0).count();
// }
// static bool time_up() {
//     return elapsed_ms() >= TIME_LIMIT_MS;
// }

// // ============================================================
// // Transposition Table
// // ============================================================
// enum TTFlag : uint8_t { TT_NONE = 0, TT_EXACT, TT_LOWER, TT_UPPER };

// struct TTEntry {
//     uint64_t key;
//     int      score;
//     Move     best_move;
//     int8_t   depth;
//     TTFlag   flag;
// };

// static constexpr int TT_SIZE = 1 << 18; // 256K entries ~16MB
// static TTEntry g_tt[TT_SIZE];

// static void tt_clear() {
//     memset(g_tt, 0, sizeof(g_tt));
// }

// static TTEntry* tt_probe(uint64_t key) {
//     TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
//     if (e->key == key && e->flag != TT_NONE) return e;
//     return nullptr;
// }

// static void tt_store(uint64_t key, int score, int depth, TTFlag flag, const Move& best) {
//     TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
//     // 只在 depth 更深或 key 不同時覆蓋
//     if (e->flag == TT_NONE || e->depth <= depth || e->key != key) {
//         e->key       = key;
//         e->score     = score;
//         e->depth     = (int8_t)depth;
//         e->flag      = flag;
//         e->best_move = best;
//     }
// }

// // ============================================================
// // Killer & History
// // ============================================================
// static constexpr int MAX_PLY = 64;
// static Move g_killer[MAX_PLY][2];
// static int  g_history[2][7][BOARD_H][BOARD_W];

// static void init_tables() {
//     for (int i = 0; i < MAX_PLY; i++)
//         g_killer[i][0] = g_killer[i][1] = Move();
//     memset(g_history, 0, sizeof(g_history));
// }

// static void save_killer(int ply, const Move& m) {
//     if (ply < 0 || ply >= MAX_PLY) return;
//     if (!(m == g_killer[ply][0])) {
//         g_killer[ply][1] = g_killer[ply][0];
//         g_killer[ply][0] = m;
//     }
// }
// static void bump_history(int player, int piece, const Move& m, int depth) {
//     if (piece < 1 || piece > 6) return;
//     g_history[player][piece][m.second.first][m.second.second] += depth * depth;
// }

// // ============================================================
// // Move scoring
// // ============================================================
// static int score_move(State* state, const Move& m, int ply, const Move& tt_move) {
//     if (m == tt_move) return 20000000;

//     int opp   = 1 - state->player;
//     int cap   = state->board.board[opp][m.second.first][m.second.second];
//     int piece = state->board.board[state->player][m.first.first][m.first.second];

//     if (cap > 0)
//         return 10000000 + MATERIAL[cap] * 100 - MATERIAL[piece];

//     if (piece == 1 && (m.second.first == 0 || m.second.first == (size_t)(BOARD_H - 1)))
//         return 5000000;

//     if (ply >= 0 && ply < MAX_PLY) {
//         if (m == g_killer[ply][0]) return 900000;
//         if (m == g_killer[ply][1]) return 800000;
//     }

//     if (piece >= 1 && piece <= 6)
//         return g_history[state->player][piece][m.second.first][m.second.second];

//     return 0;
// }

// // ============================================================
// // Quiescence search
// // ============================================================
// static constexpr int Q_LIMIT = 6;

// int Submission::quiesce(
//     State* state, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 511) == 0 && time_up()) { return alpha; }

//     if (ply >= Q_LIMIT)
//         return state->evaluate(p.use_kp_eval, false, &hist);

//     int stand_pat = state->evaluate(p.use_kp_eval, false, &hist);
//     if (stand_pat >= beta) return beta;
//     if (stand_pat > alpha) alpha = stand_pat;

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     int opp = 1 - state->player;
//     struct Cap { Move m; int score; };
//     std::vector<Cap> caps;
//     caps.reserve(16);
//     for (auto& a : state->legal_actions) {
//         int cap = state->board.board[opp][a.second.first][a.second.second];
//         if (cap > 0) {
//             int atk = state->board.board[state->player][a.first.first][a.first.second];
//             caps.push_back({a, MATERIAL[cap] * 100 - MATERIAL[atk]});
//         }
//     }
//     std::sort(caps.begin(), caps.end(), [](const Cap& a, const Cap& b){ return a.score > b.score; });

//     for (auto& c : caps) {
//         if (ctx.stop) break;
//         State* next = state->next_state(c.m);
//         bool same = next->same_player_as_parent();
//         int score = same
//             ?  quiesce(next,  alpha,  beta, hist, ply+1, ctx, p)
//             : -quiesce(next, -beta, -alpha, hist, ply+1, ctx, p);
//         delete next;
//         if (score >= beta) return beta;
//         if (score > alpha) alpha = score;
//     }
//     return alpha;
// }

// // ============================================================
// // Alpha-Beta + PVS + TT
// // ============================================================
// int Submission::eval_ctx(
//     State* state, int depth, int alpha, int beta,
//     GameHistory& hist, int ply,
//     SearchContext& ctx, const MMParams& p)
// {
//     ctx.nodes++;
//     if (ply > ctx.seldepth) ctx.seldepth = ply;
//     if (ctx.stop) return alpha;
//     if ((ctx.nodes & 1023) == 0 && time_up()) { return alpha; }

//     if (state->legal_actions.empty() && state->game_state == UNKNOWN)
//         state->get_legal_actions();

//     if (state->game_state == WIN)  return P_MAX - ply;
//     if (state->game_state == DRAW) return 0;

//     int rep;
//     if (state->check_repetition(hist, rep)) return rep;

//     // TT lookup
//     uint64_t key = state->hash();
//     Move tt_move;
//     TTEntry* tte = tt_probe(key);
//     if (tte) {
//         tt_move = tte->best_move;
//         if (tte->depth >= depth) {
//             int s = tte->score;
//             if (tte->flag == TT_EXACT) { hist.push(key); hist.pop(key); return s; }
//             if (tte->flag == TT_LOWER && s > alpha) alpha = s;
//             if (tte->flag == TT_UPPER && s < beta)  beta  = s;
//             if (alpha >= beta) { hist.push(key); hist.pop(key); return s; }
//         }
//     }

//     hist.push(key);

//     if (depth <= 0) {
//         int s = quiesce(state, alpha, beta, hist, 0, ctx, p);
//         hist.pop(key);
//         return s;
//     }

//     // Move ordering
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(state->legal_actions.size());
//     for (auto& a : state->legal_actions)
//         moves.push_back({a, score_move(state, a, ply, tt_move)});
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int orig_alpha = alpha;
//     int best = M_MAX;
//     bool first = true;
//     Move best_move;

//     for (auto& sm : moves) {
//         if (ctx.stop) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             first = false;
//         } else {
//             // PVS null-window
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, ply+1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, ply+1, ctx, p);
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
//             }
//         }
//         delete next;

//         if (score > best) { best = score; best_move = sm.m; }
//         if (best > alpha) alpha = best;
//         if (alpha >= beta) {
//             int cap = state->board.board[1-state->player][sm.m.second.first][sm.m.second.second];
//             if (cap == 0) {
//                 save_killer(ply, sm.m);
//                 int piece = state->board.board[state->player][sm.m.first.first][sm.m.first.second];
//                 bump_history(state->player, piece, sm.m, depth);
//             }
//             break;
//         }
//     }

//     // TT store
//     if (!ctx.stop) {
//         TTFlag flag;
//         if      (best <= orig_alpha) flag = TT_UPPER;
//         else if (best >= beta)       flag = TT_LOWER;
//         else                         flag = TT_EXACT;
//         tt_store(key, best, depth, flag, best_move);
//     }

//     hist.pop(key);
//     return best;
// }

// // ============================================================
// // Root search
// // ============================================================
// SearchResult Submission::search(
//     State* state, int depth,
//     GameHistory& hist, SearchContext& ctx)
// {
//     MMParams p = MMParams::from_map(ctx.params);
//     ctx.reset();

//     if (depth == 1) {
//         reset_timer();
//         init_tables();
//     }

//     SearchResult result;
//     result.depth = depth;
//     result.score = 0;

//     if (state->legal_actions.empty()) state->get_legal_actions();
//     if (!state->legal_actions.empty())
//         result.best_move = state->legal_actions[0];
//     else {
//         result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//         return result;
//     }

//     if (ctx.stop || time_up()) {
//         result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//         std::cout.flush();
//         return result;
//     }

//     int opp = 1 - state->player;
//     int total = (int)state->legal_actions.size();

//     // Root move ordering：前一層 best move 排第一
//     struct SM { Move m; int score; };
//     std::vector<SM> moves;
//     moves.reserve(total);
//     for (auto& a : state->legal_actions) {
//         int s;
//         if (a == result.best_move) s = 20000000;
//         else {
//             int cap = state->board.board[opp][a.second.first][a.second.second];
//             int piece = state->board.board[state->player][a.first.first][a.first.second];
//             s = (cap > 0) ? (10000000 + MATERIAL[cap]*100 - MATERIAL[piece]) : 0;
//             if (piece == 1 && (a.second.first == 0 || a.second.first == (size_t)(BOARD_H-1))) s += 5000000;
//         }
//         moves.push_back({a, s});
//     }
//     std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

//     int alpha = M_MAX, beta = P_MAX, best = M_MAX;
//     bool first = true, any = false;
//     Move best_this = result.best_move;
//     int idx = 0;

//     for (auto& sm : moves) {
//         if (ctx.stop || time_up()) break;

//         State* next = state->next_state(sm.m);
//         bool same = next->same_player_as_parent();
//         int score;

//         if (first) {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             first = false;
//         } else {
//             score = same
//                 ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, 1, ctx, p)
//                 : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, 1, ctx, p);
//             if (!ctx.stop && score > alpha && score < beta) {
//                 score = same
//                     ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
//                     : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
//             }
//         }
//         delete next;

//         any = true;
//         if (!ctx.stop) {
//             if (score > best) { best = score; best_this = sm.m; }
//             if (best > alpha) alpha = best;
//         }
//         idx++;
//         if (p.report_partial && ctx.on_root_update)
//             ctx.on_root_update({best_this, best, depth, idx, total});
//     }

//     if (any) { result.best_move = best_this; result.score = best; }
//     result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
//     std::cout.flush();
//     return result;
// }

// // ============================================================
// // Params
// // ============================================================
// ParamMap Submission::default_params() {
//     return {
//         {"UseKPEval",       "true"},
//         {"UseEvalMobility", "false"},
//         {"ReportPartial",   "true"},
//     };
// }
// std::vector<ParamDef> Submission::param_defs() {
//     return {
//         {"UseKPEval",       ParamDef::CHECK, "true"},
//         {"UseEvalMobility", ParamDef::CHECK, "false"},
//         {"ReportPartial",   ParamDef::CHECK, "true"},
//     };
// }










#include <utility>
#include <algorithm>
#include <vector>
#include <chrono>
#include <iostream>
#include <cstring>

#include "state.hpp"
#include "submission.hpp"

// ============================================================
// 材料值
// ============================================================
static const int MATERIAL[7] = {0, 2, 6, 7, 8, 20, 100};

// ============================================================
// 時間管理
// ============================================================
static std::chrono::steady_clock::time_point g_t0;
static constexpr double TIME_LIMIT_MS = 1700.0; // 留給 movetime 的緩衝（原 1400 偏保守，且未與 ctx.stop 連動）

static void reset_timer() {
    g_t0 = std::chrono::steady_clock::now();
}
static double elapsed_ms() {
    return std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - g_t0).count();
}
static bool time_up() {
    return elapsed_ms() >= TIME_LIMIT_MS;
}

// ============================================================
// Transposition Table
// ============================================================
enum TTFlag : uint8_t { TT_NONE = 0, TT_EXACT, TT_LOWER, TT_UPPER };

struct TTEntry {
    uint64_t key;
    int      score;
    Move     best_move;
    int8_t   depth;
    TTFlag   flag;
};

static constexpr int TT_SIZE = 1 << 18; // 256K entries ~16MB
static TTEntry g_tt[TT_SIZE];

static void tt_clear() {
    memset(g_tt, 0, sizeof(g_tt));
}

static TTEntry* tt_probe(uint64_t key) {
    TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
    if (e->key == key && e->flag != TT_NONE) return e;
    return nullptr;
}

static void tt_store(uint64_t key, int score, int depth, TTFlag flag, const Move& best) {
    TTEntry* e = &g_tt[key & (TT_SIZE - 1)];
    // 只在 depth 更深或 key 不同時覆蓋
    if (e->flag == TT_NONE || e->depth <= depth || e->key != key) {
        e->key       = key;
        e->score     = score;
        e->depth     = (int8_t)depth;
        e->flag      = flag;
        e->best_move = best;
    }
}

// ============================================================
// Killer & History
// ============================================================
static constexpr int MAX_PLY = 64;
static Move g_killer[MAX_PLY][2];
static int  g_history[2][7][BOARD_H][BOARD_W];

static void init_tables() {
    for (int i = 0; i < MAX_PLY; i++)
        g_killer[i][0] = g_killer[i][1] = Move();
    memset(g_history, 0, sizeof(g_history));
}

static void save_killer(int ply, const Move& m) {
    if (ply < 0 || ply >= MAX_PLY) return;
    if (!(m == g_killer[ply][0])) {
        g_killer[ply][1] = g_killer[ply][0];
        g_killer[ply][0] = m;
    }
}
static void bump_history(int player, int piece, const Move& m, int depth) {
    if (piece < 1 || piece > 6) return;
    g_history[player][piece][m.second.first][m.second.second] += depth * depth;
}

// ============================================================
// Move scoring
// ============================================================
static int score_move(State* state, const Move& m, int ply, const Move& tt_move) {
    if (m == tt_move) return 20000000;

    int opp   = 1 - state->player;
    int cap   = state->board.board[opp][m.second.first][m.second.second];
    int piece = state->board.board[state->player][m.first.first][m.first.second];

    if (cap > 0)
        return 10000000 + MATERIAL[cap] * 100 - MATERIAL[piece];

    if (piece == 1 && (m.second.first == 0 || m.second.first == (size_t)(BOARD_H - 1)))
        return 5000000;

    if (ply >= 0 && ply < MAX_PLY) {
        if (m == g_killer[ply][0]) return 900000;
        if (m == g_killer[ply][1]) return 800000;
    }

    if (piece >= 1 && piece <= 6)
        return g_history[state->player][piece][m.second.first][m.second.second];

    return 0;
}

// ============================================================
// Quiescence search
// ============================================================
static constexpr int Q_LIMIT = 6;

int Submission::quiesce(
    State* state, int alpha, int beta,
    GameHistory& hist, int ply,
    SearchContext& ctx, const MMParams& p)
{
    ctx.nodes++;
    if (ctx.stop) return alpha;
    // 超時時務必把 ctx.stop 設成 true，否則外層（ubgi.cpp 的 do_search）
    // 無法得知搜尋已逾時，會持續呼叫下一個 depth，導致遲遲送不出 bestmove。
    if ((ctx.nodes & 511) == 0 && time_up()) { ctx.stop = true; return alpha; }

    if (ply >= Q_LIMIT)
        return state->evaluate(p.use_kp_eval, false, &hist);

    int stand_pat = state->evaluate(p.use_kp_eval, false, &hist);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    if (state->legal_actions.empty() && state->game_state == UNKNOWN)
        state->get_legal_actions();

    int opp = 1 - state->player;
    struct Cap { Move m; int score; };
    std::vector<Cap> caps;
    caps.reserve(16);
    for (auto& a : state->legal_actions) {
        int cap = state->board.board[opp][a.second.first][a.second.second];
        if (cap > 0) {
            int atk = state->board.board[state->player][a.first.first][a.first.second];
            caps.push_back({a, MATERIAL[cap] * 100 - MATERIAL[atk]});
        }
    }
    std::sort(caps.begin(), caps.end(), [](const Cap& a, const Cap& b){ return a.score > b.score; });

    for (auto& c : caps) {
        if (ctx.stop) break;
        State* next = state->next_state(c.m);
        bool same = next->same_player_as_parent();
        int score = same
            ?  quiesce(next,  alpha,  beta, hist, ply+1, ctx, p)
            : -quiesce(next, -beta, -alpha, hist, ply+1, ctx, p);
        delete next;
        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

// ============================================================
// Alpha-Beta + PVS + TT
// ============================================================
int Submission::eval_ctx(
    State* state, int depth, int alpha, int beta,
    GameHistory& hist, int ply,
    SearchContext& ctx, const MMParams& p)
{
    ctx.nodes++;
    if (ply > ctx.seldepth) ctx.seldepth = ply;
    if (ctx.stop) return alpha;
    // 同上：超時必須設定 ctx.stop = true，讓 do_search 的 alive() 能正確跳出迭代加深迴圈。
    if ((ctx.nodes & 1023) == 0 && time_up()) { ctx.stop = true; return alpha; }

    if (state->legal_actions.empty() && state->game_state == UNKNOWN)
        state->get_legal_actions();

    if (state->game_state == WIN)  return P_MAX - ply;
    if (state->game_state == DRAW) return 0;

    int rep;
    if (state->check_repetition(hist, rep)) return rep;

    // TT lookup
    uint64_t key = state->hash();
    Move tt_move;
    TTEntry* tte = tt_probe(key);
    if (tte) {
        tt_move = tte->best_move;
        if (tte->depth >= depth) {
            int s = tte->score;
            if (tte->flag == TT_EXACT) { hist.push(key); hist.pop(key); return s; }
            if (tte->flag == TT_LOWER && s > alpha) alpha = s;
            if (tte->flag == TT_UPPER && s < beta)  beta  = s;
            if (alpha >= beta) { hist.push(key); hist.pop(key); return s; }
        }
    }

    hist.push(key);

    if (depth <= 0) {
        int s = quiesce(state, alpha, beta, hist, 0, ctx, p);
        hist.pop(key);
        return s;
    }

    // Move ordering
    struct SM { Move m; int score; };
    std::vector<SM> moves;
    moves.reserve(state->legal_actions.size());
    for (auto& a : state->legal_actions)
        moves.push_back({a, score_move(state, a, ply, tt_move)});
    std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

    int orig_alpha = alpha;
    int best = M_MAX;
    bool first = true;
    Move best_move;

    for (auto& sm : moves) {
        if (ctx.stop) break;

        State* next = state->next_state(sm.m);
        bool same = next->same_player_as_parent();
        int score;

        if (first) {
            score = same
                ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
                : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
            first = false;
        } else {
            // PVS null-window
            score = same
                ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, ply+1, ctx, p)
                : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, ply+1, ctx, p);
            if (!ctx.stop && score > alpha && score < beta) {
                score = same
                    ?  eval_ctx(next, depth-1,  alpha,  beta, hist, ply+1, ctx, p)
                    : -eval_ctx(next, depth-1, -beta, -alpha, hist, ply+1, ctx, p);
            }
        }
        delete next;

        if (score > best) { best = score; best_move = sm.m; }
        if (best > alpha) alpha = best;
        if (alpha >= beta) {
            int cap = state->board.board[1-state->player][sm.m.second.first][sm.m.second.second];
            if (cap == 0) {
                save_killer(ply, sm.m);
                int piece = state->board.board[state->player][sm.m.first.first][sm.m.first.second];
                bump_history(state->player, piece, sm.m, depth);
            }
            break;
        }
    }

    // TT store
    if (!ctx.stop) {
        TTFlag flag;
        if      (best <= orig_alpha) flag = TT_UPPER;
        else if (best >= beta)       flag = TT_LOWER;
        else                         flag = TT_EXACT;
        tt_store(key, best, depth, flag, best_move);
    }

    hist.pop(key);
    return best;
}

// ============================================================
// Root search
// ============================================================
SearchResult Submission::search(
    State* state, int depth,
    GameHistory& hist, SearchContext& ctx)
{
    MMParams p = MMParams::from_map(ctx.params);
    ctx.reset();

    if (depth == 1) {
        reset_timer();
        init_tables();
    }

    SearchResult result;
    result.depth = depth;
    result.score = 0;

    if (state->legal_actions.empty()) state->get_legal_actions();
    if (!state->legal_actions.empty())
        result.best_move = state->legal_actions[0];
    else {
        result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
        return result;
    }

    if (ctx.stop || time_up()) {
        ctx.stop = true;
        result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
        std::cout.flush();
        return result;
    }

    int opp = 1 - state->player;
    int total = (int)state->legal_actions.size();

    // Root move ordering：前一層 best move 排第一
    struct SM { Move m; int score; };
    std::vector<SM> moves;
    moves.reserve(total);
    for (auto& a : state->legal_actions) {
        int s;
        if (a == result.best_move) s = 20000000;
        else {
            int cap = state->board.board[opp][a.second.first][a.second.second];
            int piece = state->board.board[state->player][a.first.first][a.first.second];
            s = (cap > 0) ? (10000000 + MATERIAL[cap]*100 - MATERIAL[piece]) : 0;
            if (piece == 1 && (a.second.first == 0 || a.second.first == (size_t)(BOARD_H-1))) s += 5000000;
        }
        moves.push_back({a, s});
    }
    std::sort(moves.begin(), moves.end(), [](const SM& a, const SM& b){ return a.score > b.score; });

    int alpha = M_MAX, beta = P_MAX, best = M_MAX;
    bool first = true, any = false;
    Move best_this = result.best_move;
    int idx = 0;

    for (auto& sm : moves) {
        // 同上：根節點也要在偵測到逾時時設定 ctx.stop，
        // 確保 ubgi.cpp 的 do_search 迭代加深迴圈會在下一輪正確跳出，
        // 而不是繼續呼叫下一個 depth 卻什麼都做不了（造成遲遲不送出 bestmove）。
        if (ctx.stop || time_up()) { ctx.stop = true; break; }

        State* next = state->next_state(sm.m);
        bool same = next->same_player_as_parent();
        int score;

        if (first) {
            score = same
                ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
                : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
            first = false;
        } else {
            score = same
                ?  eval_ctx(next, depth-1,  alpha,  alpha+1, hist, 1, ctx, p)
                : -eval_ctx(next, depth-1, -alpha-1, -alpha, hist, 1, ctx, p);
            if (!ctx.stop && score > alpha && score < beta) {
                score = same
                    ?  eval_ctx(next, depth-1,  alpha,  beta, hist, 1, ctx, p)
                    : -eval_ctx(next, depth-1, -beta, -alpha, hist, 1, ctx, p);
            }
        }
        delete next;

        any = true;
        if (!ctx.stop) {
            if (score > best) { best = score; best_this = sm.m; }
            if (best > alpha) alpha = best;
        }
        idx++;
        if (p.report_partial && ctx.on_root_update)
            ctx.on_root_update({best_this, best, depth, idx, total});
    }

    if (any) { result.best_move = best_this; result.score = best; }
    result.nodes = ctx.nodes; result.seldepth = ctx.seldepth;
    std::cout.flush();
    return result;
}

// ============================================================
// Params
// ============================================================
ParamMap Submission::default_params() {
    return {
        {"UseKPEval",       "true"},
        {"UseEvalMobility", "false"},
        {"ReportPartial",   "true"},
    };
}
std::vector<ParamDef> Submission::param_defs() {
    return {
        {"UseKPEval",       ParamDef::CHECK, "true"},
        {"UseEvalMobility", ParamDef::CHECK, "false"},
        {"ReportPartial",   ParamDef::CHECK, "true"},
    };
}