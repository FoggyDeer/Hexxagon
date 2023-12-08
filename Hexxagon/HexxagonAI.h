#include <set>
#include <vector>
#include <algorithm>

#pragma once

namespace Hexxagon
{
	class Board;
	class StepField;

	/////////////////////////////////////////////////////////
	/// Basic Hexxagon AI class which implements
	/// algorithms for game with computer.
	/////////////////////////////////////////////////////////
	class HexxagonAI
	{
	private:
		Board* board;

		std::vector<StepField*> fields;

		std::vector<StepField*> getOccupiedFields() const;		//!< returns all occupied fields

		std::vector<StepField*> getAvailableFields() const;		//!< returns fields which contains gamechips available for step

	public:
		HexxagonAI(Board* board);

		void makeStep() const;		//!< basic AI logic
	};
}