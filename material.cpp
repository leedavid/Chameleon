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

#include <algorithm> // For std::min
#include <cassert>
#include <cstring>   // For std::memset

#include "material.h"
#include "thread.h"

using namespace std;

// Polynomial material imbalance parameters
//                      pair  pawn advisor bishop knight cannon  rook 
const int Linear[7] =
{
	1667, -168, -168,
	-166, -1027,-138,  238,
};

const int QuadraticOurs[][PIECE_TYPE_NB] =
{
	//            OUR PIECES
	// pair pawn advisor bishop knight bishop rook
	{  1667                                       }, // Bishop pair
	{  40,    2                                   }, // Pawn
	{                                             }, // advisor
	{  0,  104,   4,    0                         }, // Bishop
	{  32,  255,  -3                              }, // Knight      OUR PIECES
	{ -26,   -2,  47,   105,  -149                }, // Cannon
	{ -185,   24,  122,   137,  -134, 0           }  // Rook    
};

const int QuadraticTheirs[][PIECE_TYPE_NB] =
{
	//           THEIR PIECES
	// pair pawn knight bishop rook queen
	{  0                                }, // Bishop pair
	{ 36,    0                          }, // Pawn
	{                                   }, // Advisor
	{ 59,   65,  42,     0              }, // Bishop
	{  9,   63,  0                      }, // Knight
	{  46,   39,  24,   -24,    0       }, // Cannon
	{ 101,  100, -37,   141,  268,    0 }  // Rook
};

// imbalance() calculates the imbalance by comparing the piece count of each
// piece type for both colors.
template<Color Us>
int imbalance(const int pieceCount[][PIECE_TYPE_NB])
{
	const Color Them = (Us == WHITE ? BLACK : WHITE);

	int bonus = 0;

	// Second-degree polynomial material imbalance by Tord Romstad
	for (int pt1 = NO_PIECE_TYPE; pt1 <= ROOK; ++pt1)
	{
		if (!pieceCount[Us][pt1])
			continue;

		int v = Linear[pt1];

		for (int pt2 = NO_PIECE_TYPE; pt2 <= pt1; ++pt2)
			v += QuadraticOurs[pt1][pt2] * pieceCount[Us][pt2]
			+ QuadraticTheirs[pt1][pt2] * pieceCount[Them][pt2];

		bonus += pieceCount[Us][pt1] * v;
	}
	return bonus;
}

namespace Material
{
	// Material::probe() looks up the current position's material configuration in
	// the material hash table. It returns a pointer to the Entry if the position
	// is found. Otherwise a new Entry is computed and stored there, so we don't
	// have to recompute all when the same material configuration occurs again.
	Entry* probe(const Position& pos)
	{
		uint64_t key = pos.material_key();
		Entry* e = pos.this_thread()->materialTable[key];

		if (e->key == key)
			return e;

		std::memset(e, 0, sizeof(Entry));
		e->key = key;
		e->factor[WHITE] = e->factor[BLACK] = (uint8_t)SCALE_FACTOR_NORMAL;
		e->gamePhase = pos.game_phase();

		// Let's look if we have a specialized evaluation function for this particular
		// material configuration. Firstly we look for a fixed configuration one, then
		// for a generic one if the previous search failed.
		if ((e->evaluationFunction = pos.this_thread()->endgames.probe<Value>(key)) != nullptr)
			return e;

		// OK, we didn't find any special evaluation function for the current material
		// configuration. Is there a suitable specialized scaling function?
		EndgameBase<ScaleFactor>* sf;

		if ((sf = pos.this_thread()->endgames.probe<ScaleFactor>(key)) != nullptr)
		{
			e->scalingFunction[sf->strong_side()] = sf; // Only strong color assigned
			return e;
		}

		// Evaluate the material imbalance. We use PIECE_TYPE_NONE as a place holder
		// for the bishop pair "extended piece", which allows us to be more flexible
		// in defining bishop pair bonuses.
		const int PieceCount[COLOR_NB][PIECE_TYPE_NB] =
		{
				{ pos.count<BISHOP>(WHITE) > 1, pos.count<PAWN>(WHITE),  pos.count<ADVISOR>(WHITE),pos.count<BISHOP>(WHITE),
				  pos.count<KNIGHT>(WHITE),  pos.count<CANNON>(WHITE), pos.count<ROOK>(WHITE) },
				{ pos.count<BISHOP>(BLACK) > 1, pos.count<PAWN>(BLACK), pos.count<ADVISOR>(BLACK), pos.count<BISHOP>(BLACK),
				  pos.count<KNIGHT>(BLACK)    , pos.count<CANNON >(BLACK), pos.count<ROOK>(BLACK) } };

		e->value = int16_t((imbalance<WHITE>(PieceCount) - imbalance<BLACK>(PieceCount)) / 16);
		return e;
	}
} // namespace Material