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

#ifndef BITBOARD_H
#define BITBOARD_H
#include <cstdint>
#include <cstdio>

class Bitboard
{
public:
	Bitboard();
	Bitboard(uint64_t low, uint64_t hig);
	operator bool() const;
	int operator == (const Bitboard& board) const;
	int operator != (const Bitboard& board) const;
	Bitboard operator ~() const;
	Bitboard operator &(const Bitboard& board) const;
	Bitboard operator |(const Bitboard& board) const;
	Bitboard operator ^(const Bitboard &board) const;
	Bitboard &operator &=(const Bitboard &board);
	Bitboard &operator |=(const Bitboard &board);
	Bitboard &operator ^=(const Bitboard &board);
	Bitboard operator <<(int bit);
	Bitboard operator >> (int bit);
	Bitboard &operator <<=(int bit);
	Bitboard &operator >>=(int bit);
	void pop_lsb();
	bool more_than_one() const;
	bool equal_to_two() const;
public:
	uint64_t bb[2];
	static const uint64_t BIT_MASK;
};

#endif