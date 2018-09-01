/*
  Chameleon, a UCI chinese chess playing engine derived from Stockfish
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad
  Copyright (C) 2015-2017 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad
  Copyright (C) 2017 Wilbert Lee

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <algorithm>
#include <cassert>
#include <cstring>   // For std::memset
#include <iomanip>
#include <sstream>

#include "bitcount.h"
#include "evaluate.h"
#include "material.h"
#include "pawns.h"

namespace Trace {
	// First 8 entries are for PieceType
	enum Term
	{
		MATERIAL = 8, IMBALANCE, MOBILITY,
		THREAT, PASSED, SPACE, TOTAL, TERM_NB
	};

	double scores[TERM_NB][COLOR_NB][PHASE_NB];

	double to_cp(Value v) { return double(v) / PAWN_VALUE_EG; }

	void add(int idx, Color c, Score s) {
		scores[idx][c][MG] = to_cp(mg_value(s));
		scores[idx][c][EG] = to_cp(eg_value(s));
	}

	void add(int idx, Score w, Score b = SCORE_ZERO) {
		add(idx, WHITE, w); add(idx, BLACK, b);
	}

	std::ostream& operator<<(std::ostream& os, Term t) {

		if (t == MATERIAL || t == IMBALANCE || t == Term(PAWN) || t == TOTAL)
			os << "  ---   --- |   ---   --- | ";
		else
			os << std::setw(5) << scores[t][WHITE][MG] << " "
			<< std::setw(5) << scores[t][WHITE][EG] << " | "
			<< std::setw(5) << scores[t][BLACK][MG] << " "
			<< std::setw(5) << scores[t][BLACK][EG] << " | ";

		os << std::setw(5) << scores[t][WHITE][MG] - scores[t][BLACK][MG] << " "
			<< std::setw(5) << scores[t][WHITE][EG] - scores[t][BLACK][EG] << " \n";

		return os;
	}
}

using namespace Trace;

// Struct EvalInfo contains various information computed and collected
// by the evaluation functions.
struct EvalInfo {

	// attackedBy[color][piece type] is a bitboard representing all squares
	// attacked by a given color and piece type (can be also ALL_PIECES).
	Bitboard attackedBy[COLOR_NB][PIECE_TYPE_NB];

	// kingRing[color] is the zone around the king which is considered
	// by the king safety evaluation. This consists of the squares directly
	// adjacent to the king, and the three (or two, for a king on an edge file)
	// squares two ranks in front of the king. For instance, if black's king
	// is on g8, kingRing[BLACK] is a bitboard containing the squares f8, h8,
	// f7, g7, h7, f6, g6 and h6.
	Bitboard kingRing[COLOR_NB];

	// kingAttackersCount[color] is the number of pieces of the given color
	// which attack a square in the kingRing of the enemy king.
	int kingAttackersCount[COLOR_NB];

	// kingAttackersWeight[color] is the sum of the "weight" of the pieces of the
	// given color which attack a square in the kingRing of the enemy king. The
	// weights of the individual piece types are given by the elements in the
	// KingAttackWeights array.
	int kingAttackersWeight[COLOR_NB];

	// kingAdjacentZoneAttacksCount[color] is the number of attacks by the given
	// color to squares directly adjacent to the enemy king. Pieces which attack
	// more than one square are counted multiple times. For instance, if there is
	// a white knight on g5 and black's king is on g8, this white knight adds 2
	// to kingAdjacentZoneAttacksCount[WHITE].
	int kingAdjacentZoneAttacksCount[COLOR_NB];

	Bitboard pinnedPieces[COLOR_NB];
	Material::Entry* me;
	Pawns::Entry* pi;
};


// Evaluation weights, indexed by the corresponding evaluation term
enum { PawnStructure, PassedPawns, Space, KingSafety };

const struct Weight { int mg, eg; } Weights[] = {
  {214, 203}, {193, 262}, {47, 0}, {330, 0} };

Score operator*(Score s, const Weight& w) {
	return make_score(mg_value(s) * w.mg / 256, eg_value(s) * w.eg / 256);
}


#define V(v) Value(v)
#define S(mg, eg) make_score(mg, eg)

// MobilityBonus[PieceType][attacked] contains bonuses for middle and end
// game, indexed by piece type and number of attacked squares in the MobilityArea.
const Score MobilityBonus[][32] = {
  {},
  {},//PAWN EMPTY
  {},//ADVISOR
  {},//BISHOP
  {},//KNIGHT
  {},//CANNON
  {} //ROOK
};

// Outpost[knight/bishop][supported by pawn] contains bonuses for knights and
// bishops outposts, bigger if outpost piece is supported by a pawn.
const Score Outpost[][2] = {
  { S(42,11), S(63,17) }, // Knights
  { S(18, 5), S(27, 8) }  // Bishops
};

// ReachableOutpost[knight/bishop][supported by pawn] contains bonuses for
// knights and bishops which can reach an outpost square in one move, bigger
// if outpost square is supported by a pawn.
const Score ReachableOutpost[][2] = {
  { S(21, 5), S(31, 8) }, // Knights
  { S(8, 2), S(13, 4) }  // Bishops
};

// RookOnFile[semiopen/open] contains bonuses for each rook when there is no
// friendly pawn on the rook file.
const Score RookOnFile[2] = { S(19, 10), S(43, 21) };

// ThreatBySafePawn[PieceType] contains bonuses according to which piece
// type is attacked by a pawn which is protected or not attacked.
const Score ThreatBySafePawn[PIECE_TYPE_NB] = {
  S(0, 0), S(0, 0), S(176, 139), S(176, 139), S(131, 127), S(217, 218), S(203, 215) };

// Threat[by minor/by rook][attacked PieceType] contains
// bonuses according to which piece type attacks which one.
// Attacks on lesser pieces which are pawn defended are not considered.
const Score Threat[][PIECE_TYPE_NB] = {
  { S(0, 0), S(0, 33),  S(45, 43),S(45, 43), S(46, 47), S(72,107), S(48,118) }, // by Minor
  { S(0, 0), S(0, 25),  S(40, 62),S(40, 62), S(40, 59), S(0, 34), S(35, 48) }  // by Rook
};

// ThreatByKing[on one/on many] contains bonuses for King attacks on
// pawns or pieces which are not pawn defended.
const Score ThreatByKing[2] = { S(3, 62), S(9, 138) };

// Passed[mg/eg][Rank] contains midgame and endgame bonuses for passed pawns.
// We don't use a Score because we process the two components independently.
const Value Passed[][RANK_NB] = {
  { V(0), V(1), V(34), V(90), V(214), V(328) , V(328) , V(328) , V(328) , V(328) },
  { V(7), V(14), V(37), V(63), V(134), V(189) , V(189) , V(189) , V(189) , V(189) }
};

// PassedFile[File] contains a bonus according to the file of a passed pawn
const Score PassedFile[FILE_NB] = {
  S(12, 10), S(3, 10), S(1, -8), S(-27,-12),
  S(-27,-12),
  S(-27,-12), S(1, -8), S(3, 10), S(12, 10)
};

// Assorted bonuses and penalties used by evaluation
const Score MinorBehindPawn = S(16, 0);
const Score BishopPawns = S(8, 12);
const Score RookOnPawn = S(7, 27);
const Score TrappedRook = S(92, 0);
const Score Checked = S(20, 20);
const Score ThreatByHangingPawn = S(70, 63);
const Score Hanging = S(48, 28);
const Score ThreatByPawnPush = S(31, 19);
const Score Unstoppable = S(0, 20);

// Penalty for a bishop on a1/h1 (a8/h8 for black) which is trapped by
// a friendly pawn on b2/g2 (b7/g7 for black). This can obviously only
// happen in Chess960 games.
const Score TrappedBishopA1H1 = S(50, 50);

#undef S
#undef V

// King danger constants and variables. The king danger scores are looked-up
// in KingDanger[]. Various little "meta-bonuses" measuring the strength
// of the enemy attack are added up into an integer, which is used as an
// index to KingDanger[].
Score KingDanger[512];

// KingAttackWeights[PieceType] contains king attack weights by piece type
const int KingAttackWeights[PIECE_TYPE_NB] = { 0, 0, 7, 5, 4, 1 ,1 };

// Penalties for enemy's safe checks
const int QueenContactCheck = 89;
const int QueenCheck = 50;
const int RookCheck = 45;
const int BishopCheck = 6;
const int KnightCheck = 14;


// eval_init() initializes king and attack bitboards for given color
// adding pawn attacks. To be done at the beginning of the evaluation.
template<Color Us>
void eval_init(const Position& pos, EvalInfo& ei) {

	const Color  Them = (Us == WHITE ? BLACK : WHITE);
	const Square Down = (Us == WHITE ? DELTA_S : DELTA_N);

	ei.pinnedPieces[Us] = pos.pinned_pieces(Us);
	Bitboard b = ei.attackedBy[Them][KING] = pos.attacks_from<KING>(pos.square<KING>(Them));
	ei.attackedBy[Them][ALL_PIECES] |= b;
	ei.attackedBy[Us][ALL_PIECES] |= ei.attackedBy[Us][PAWN] = ei.pi->pawn_attacks(Us);

	// Init king safety tables only if we are going to use them
	if (pos.non_pawn_material(Us) >= ROOK_VALUE_MG)
	{
		ei.kingRing[Them] = b | shift_bb(b, Down);
		b &= ei.attackedBy[Us][PAWN];
		ei.kingAttackersCount[Us] = b ? popcount(b) : Bitboard();
		ei.kingAdjacentZoneAttacksCount[Us] = ei.kingAttackersWeight[Us] = Bitboard();
	}
	else
	{
		ei.kingRing[Them] = Bitboard();
		ei.kingAttackersCount[Us] = 0;
	}
}


// evaluate_pieces() assigns bonuses and penalties to the pieces of a given
// color and type.
template<bool DoTrace, Color Us = WHITE, PieceType Pt = KNIGHT>
Score evaluate_pieces(const Position& pos, EvalInfo& ei, Score* mobility,
	const Bitboard* mobilityArea) {
	Bitboard b, bb;
	Square s;
	Score score = SCORE_ZERO;
	return score;
}

template<>
Score evaluate_pieces<false, WHITE, KING>(const Position&, EvalInfo&, Score*, const Bitboard*) { return SCORE_ZERO; }
template<>
Score evaluate_pieces< true, WHITE, KING>(const Position&, EvalInfo&, Score*, const Bitboard*) { return SCORE_ZERO; }


// evaluate() is the main evaluation function. It returns a static evaluation
// of the position from the point of view of the side to move.
template<bool DoTrace>
Value Eval::evaluate(const Position& pos) {

	assert(!pos.checkers());

	EvalInfo ei;
	Score score, mobility[COLOR_NB] = { SCORE_ZERO, SCORE_ZERO };

	// Initialize score by reading the incrementally updated scores included in
	// the position object (material + piece square tables). Score is computed
	// internally from the white point of view.
	score = pos.psq_score();

	ScaleFactor sf = SCALE_FACTOR_NONE;

	// Interpolate between a middlegame and a (scaled by 'sf') endgame score
	Value v = mg_value(score) * int(PHASE_MIDGAME)
		+ eg_value(score) * int(PHASE_MIDGAME - PHASE_MIDGAME) * sf / SCALE_FACTOR_NORMAL;

	v /= int(PHASE_MIDGAME);

	return (pos.side_to_move() == WHITE ? v : -v) + Eval::Tempo;
}

// Explicit template instantiations
template Value Eval::evaluate<true >(const Position&);
template Value Eval::evaluate<false>(const Position&);

// init() computes evaluation weights, usually at startup
void Eval::init() {
	const int MaxSlope = 8700;
	const int Peak = 1280000;

	for (int i = 0, t = 0; i < 400; ++i)
	{
		t = std::min(Peak, std::min(i * i * 27, t + MaxSlope));
		KingDanger[i] = make_score(t / 1000, 0) * Weights[KingSafety];
	}
}