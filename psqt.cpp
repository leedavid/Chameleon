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

#include "types.h"

namespace PSQT
{
#define S(mg, eg) make_score(mg, eg)

	// Bonus[PieceType][Square / 2] contains Piece-Square scores. For each piece
	// type on a given square a (middlegame, endgame) score pair is assigned. Table
	// is defined for files A..D and white side: it is symmetric for black side and
	// second half of the files.
	const Score Bonus[][RANK_NB][int(FILE_NB + 1) / 2] =
	{
		{
		},
		{	// Pawn
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// Advisor
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// Bishop
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// Knight
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// Cannon
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// Rook
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		},
		{	// King
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			//---------------------------------
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) },
			{ S(0,0), S(0,0), S(0,0), S(0,0), S(0,0) }
		}
	};

#undef S
	Score psq[COLOR_NB][PIECE_TYPE_NB][SQUARE_NB];

	// init() initializes piece square tables: the white halves of the tables are
	// copied from Bonus[] adding the piece value, then the black halves of the
	// tables are initialized by flipping and changing the sign of the white scores.
	void init()
	{
		for (PieceType pt = PAWN; pt <= KING; ++pt)
		{
			PieceValue[MG][make_piece(BLACK, pt)] = PieceValue[MG][pt];
			PieceValue[EG][make_piece(BLACK, pt)] = PieceValue[EG][pt];

			Score v = make_score(PieceValue[MG][pt], PieceValue[EG][pt]);

			for (Square s = SQ_A0; s <= SQ_I9; ++s)
			{
				int edgeDistance = file_of(s) <= FILE_E ? file_of(s) : FILE_I - file_of(s);
				psq[BLACK][pt][~s] = -(psq[WHITE][pt][s] = v + Bonus[pt][rank_of(s)][edgeDistance]);
			}
		}
	}

} // namespace PSQT