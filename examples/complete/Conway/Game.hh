#ifndef GAME_HH
#define GAME_HH

#include <avr/sfr_defs.h>

//TODO Improve templating by defining the right type for each row (uint8_t, uint16_t, uint32_t...)
// Row Type could be deduced from COLUMNS value (or conversely)
//TODO Maybe make a class to hold one generation and access its members?
template<uint8_t ROWS, uint8_t COLUMNS>
class GameOfLife
{
public:
	GameOfLife(uint8_t game[ROWS]):_current_generation{game}, _empty{}, _still{} {}
	
	void progress_game_old()
	{
		uint8_t next_generation[ROWS];
		for (uint8_t row = 0; row < ROWS; ++row)
			for (uint8_t col = 0; col < COLUMNS; ++col)
			{
				uint8_t count_neighbours = neighbours(row, col);
				if (count_neighbours == 3 || (count_neighbours == 4 && (_current_generation[row] & _BV(col))))
					// cell is alive
					next_generation[row] |= _BV(col);
				else
					// cell is dead
					next_generation[row] &= ~_BV(col);
			}
		// Copy next generation to current one
		_still = true;
		_empty = true;
		for (uint8_t row = 0; row < ROWS; ++row)
		{
			if (_current_generation[row] != next_generation[row])
				_still = false;
			if ((_current_generation[row] = next_generation[row]))
				_empty = false;
		}
	}
	
	void progress_game()
	{
		uint8_t next_generation[ROWS];
		for (uint8_t row = 0; row < ROWS; ++row)
		{
			uint8_t previous = (row ? _current_generation[row - 1] : _current_generation[ROWS - 1]);
			uint8_t next = (row == ROWS - 1 ? _current_generation[0] : _current_generation[row + 1]);
			uint8_t current = _current_generation[row];
			uint8_t count_high, count_low;
			neighbours1(current, previous, next, count_high, count_low);
			uint8_t ok, code;
			neighbours2(count_high, count_low, code, ok);
			
			current = ok & ((code & ~current) | ~code);
			next_generation[row] = current; 
		}
		// Copy next generation to current one
		_still = true;
		_empty = true;
		for (uint8_t row = 0; row < ROWS; ++row)
		{
			if (_current_generation[row] != next_generation[row])
				_still = false;
			if ((_current_generation[row] = next_generation[row]))
				_empty = false;
		}
	}
	
	inline bool is_empty()
	{
		return _empty;
	}

	inline bool is_still()
	{
		return _still;
	}

private:
	static uint8_t rotate_left(uint8_t input)
	{
		return (input << 1) | (input >> 7);
	}
	static uint8_t rotate_right(uint8_t input)
	{
		return (input >> 1) | (input << 7);
	}
	static void adder(uint8_t input_a, uint8_t input_b, uint8_t input_carry, uint8_t& output_sum, uint8_t& output_carry)
	{
		// Perform bit-parallel calculation and update counts array
		// On return, counts contain the number of live cells over 3 rows, column per column (0-3)
		output_sum = input_a ^ input_b;
		output_carry = (output_sum | input_a) & input_carry;
		output_sum ^= input_carry;
	}
	static void adder(uint8_t input_a, uint8_t input_b, uint8_t input_carry, uint8_t& output_sum)
	{
		// Perform bit-parallel calculation and update counts array
		output_sum = input_a ^ input_b ^ input_carry;
	}
	
	static void neighbours1(uint8_t row1, uint8_t row2, uint8_t row3, uint8_t& count_high, uint8_t& count_low)
	{
		// Perform bit-parallel calculation and update counts array
		// On return, counts contain the number of live cells over 3 rows, column per column (0-3)
		adder(row1, row2, row3, count_low, count_high);
	}
	
	static void neighbours2(uint8_t count_high, uint8_t count_low, uint8_t& code, uint8_t& ok)
	{
		// Compute bit-parallel number of neighbours for each column
		// On return, for each column we return a code indicating:
		// - if the cell at this column has 3 neighbours (including itself)
		// - if the cell at this column has 4 neighbours (including itself)
		// - if the cell has less than 3 or more than 4 neighbours (including itself)

		// To perform bit-parallel computation we'll need to rotate copies of count bytes left and right
		// Add each column to its left and right columns into 4 bits [0-9]
		uint8_t total_0, total_1, total_2, total_3, carry_0, carry_1, carry_2;
		adder(count_low, rotate_left(count_low), rotate_right(count_low), total_0, carry_0);
		adder(count_high, rotate_left(count_high), rotate_right(count_high), total_1, carry_1);
		// Add carries now
		adder(total_0, total_1, carry_0, total_2, carry_2);
		adder(total_2, carry_1, carry_2, total_3);

		//TODO compute result bits
		// - OK = 0 => too few or too many neighbours
		// - OK = 1 / CODE = 0 -> 3 neighbours
		// - OK = 1 / CODE = 1 -> 4 neighbours
		ok = (~total_3) & (total_1 ^ total_2) & ~(total_0 ^ total_1);
		code = ok & total_2;
	}
	
	static uint8_t neighbours_in_row(uint8_t game_row, uint8_t col)
	{
		//TODO possibly optimize by:
		// - copy row to GPIOR0
		// - rotate GPIOR (col+1) times
		// check individual bits 0, 1 and 2
		uint8_t count = (game_row & _BV(col)) ? 1 : 0;
		if (game_row & _BV(col ? col - 1 : COLUMNS - 1)) ++count;
		if (game_row & _BV(col == COLUMNS - 1 ? 0 : col + 1)) ++count;
		return count;
	}
	
	uint8_t neighbours(uint8_t row, uint8_t col)
	{
		uint8_t count = neighbours_in_row(row ? _current_generation[row - 1] : _current_generation[ROWS - 1], col);
		count += neighbours_in_row(row == ROWS - 1 ? _current_generation[0] : _current_generation[row + 1], col);
		count += neighbours_in_row(_current_generation[row], col);
		return count;
	}

	uint8_t* _current_generation;
	bool _empty;
	bool _still;
};

#endif /* GAME_HH */