#include "GameBoard.h"
namespace Hexxagon
{
	////////////////////////////////////////////////////////////
	HexxagonAI::HexxagonAI(Board* board) : board(board), fields(board->getFields()) {
		std::srand(std::time(nullptr));
	};

	////////////////////////////////////////////////////////////
	std::vector<StepField*> HexxagonAI::getOccupiedFields() const
	{
		std::vector<StepField*> v{};
		for (StepField* field : board->getFields()) {
			if (field != nullptr && field->isOccupied() && field->getGameChip()->getColor() == sf::Color::Blue) {
				v.push_back(field);
			}
		}
		return v;
	}

	////////////////////////////////////////////////////////////
	std::vector<StepField*> HexxagonAI::getAvailableFields() const
	{
		std::set<StepField*> s{};
		for (StepField* field : getOccupiedFields()) {
			if (field->getGameChip()->canMakeStep())
				s.insert(field);
		}
		return std::vector<StepField*>(s.begin(), s.end());
	}

	////////////////////////////////////////////////////////////
	void HexxagonAI::makeStep() const
	{
		std::vector<StepField*> v = getAvailableFields();
		auto range = std::ranges::partition(v, [](StepField* f) -> bool {return f->isBorder(); });

		StepField* selectedField = nullptr;
		StepField* nextStepField = nullptr;
		int buff_count = 0;
		int count = 0;

		if (v.size() > 0) {
			/////////////////////////////////////////////////////////
			/// Checking the edge gamechips for the near move
			/////////////////////////////////////////////////////////
			for (auto iter_i = v.begin(); iter_i < range.begin() && buff_count != 5; iter_i++) {
				std::vector<StepField*> v1 = (*iter_i)->getFreeCloseNeighbours();
				if(v1.size() > 0)
					for (auto iter_j = v1.begin(); iter_j < v1.end() && buff_count != 5; iter_j++) {
						buff_count = (*iter_j)->getNearestGamechipCount(sf::Color::Red) > count;
						if (buff_count > count) {
							selectedField = *iter_i;
							nextStepField = (*iter_j);
							count = buff_count;
						}
					}
			}

			if (count != 5) {
				/////////////////////////////////////////////////////////
				/// Checking the edge gamechips for the distant move
				/////////////////////////////////////////////////////////
				for (auto iter_i = v.begin(); iter_i < range.begin() && buff_count != 5; iter_i++) {
					std::vector<StepField*> v1 = (*iter_i)->getFreeDistantNeighbours();
					if (v1.size() > 0)
						for (auto iter_j = v1.begin(); iter_j < v1.end() && buff_count != 5; iter_j++) {
							buff_count = (*iter_j)->getNearestGamechipCount(sf::Color::Red);
							if (buff_count > 1 && buff_count > count) {
								selectedField = *iter_i;
								nextStepField = (*iter_j);
								count = buff_count;
							}
						}
				}

				/////////////////////////////////////////////////////////
				/// Checking the edge gamechips for the distant move
				/////////////////////////////////////////////////////////
				for (auto iter_i = range.begin(); iter_i < range.end() && buff_count != 5; iter_i++) {
					std::vector<StepField*> v1 = (*iter_i)->getFreeDistantNeighbours();
					if (v1.size() > 0)
						for (auto iter_j = v1.begin(); iter_j < v1.end() && buff_count != 5; iter_j++) {
							buff_count = (*iter_j)->getNearestGamechipCount(sf::Color::Red);
							if (buff_count > 1 && buff_count > count) {
								selectedField = *iter_i;
								nextStepField = (*iter_j);
								count = buff_count;
							}
						}
				}
			}

			if (selectedField == nullptr || nextStepField == nullptr) {
				if (range.begin() - v.begin() > 0) {
					selectedField = v.at(std::rand() % (range.begin() - v.begin()));
					std::vector<StepField*> v1 = selectedField->getFreeCloseNeighbours();
					if (v1.size() > 0)
						nextStepField = *v1.begin();
					else
						nextStepField = *selectedField->getFreeDistantNeighbours().begin();
				}
			}
		}

		board->clearSelected();
		board->selected_f = selectedField;
		board->makeStep(nextStepField);
	}
}