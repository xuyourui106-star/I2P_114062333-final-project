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










