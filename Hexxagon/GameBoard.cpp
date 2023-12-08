#include "GameBoard.h"
#include "HexxagonAI.h"

namespace Hexxagon
{

    /***********************************************************/
    /// GameChip class methods initialisation.
    /***********************************************************/
    GameChip::GameChip(sf::Color color, StepField* currentField) : color(color), currentField(currentField) {};

    ////////////////////////////////////////////////////////////
    sf::Color GameChip::getColor() const { return color; }

    ////////////////////////////////////////////////////////////
    void GameChip::setColor(sf::Color c) { color = c; }

    ////////////////////////////////////////////////////////////
    StepField* GameChip::getField() const { return currentField; }

    ////////////////////////////////////////////////////////////
    void GameChip::setField(StepField* field) { currentField = field; }

    ////////////////////////////////////////////////////////////
    bool GameChip::canMakeStep() const{
        return currentField->getFreeCloseNeighbours().size() + currentField->getFreeDistantNeighbours().size() > 0;
    }


    /***********************************************************/
    /// StepField class methods initialisation.
    /***********************************************************/
    StepField::StepField(float Radius, int pointCount) : Radius(Radius), sf::CircleShape(Radius, pointCount)
    {
        this->ID     = ++count;
        this->radius = sqrt(3) / 2.f * Radius;
        isSelected   = false;
        gameChip     = nullptr;

        setFillColor(sf::Color::White);
        setOutlineColor(sf::Color::Transparent);
        setOutlineThickness(5.f);
    }

    ////////////////////////////////////////////////////////////
    bool StepField::isOccupied() const { return gameChip != nullptr; }

    ////////////////////////////////////////////////////////////
    GameChip* StepField::getGameChip() const { return gameChip; }

    ////////////////////////////////////////////////////////////
    void StepField::occupy(GameChip* chip)
    {
        gameChip = chip;
        chip->setField(this);
        setFillColor(chip->getColor());
    }

    ////////////////////////////////////////////////////////////
    void StepField::makeFree()
    {
        this->gameChip = nullptr;
        setFillColor(sf::Color::White);
    }

    ////////////////////////////////////////////////////////////
    void StepField::setSelected(bool selected)
    {
        if (selected && isOccupied())
        {
            isSelected = true;
            setOutlineColor(sf::Color::Yellow);
            for (StepField* neighbour : neighbours)
            {
                if (!neighbour->isOccupied())
                    neighbour->setOutlineColor(sf::Color::Green);
            }
            for (StepField* neighbour : neighbours)
            {
                for (StepField* stepField : neighbour->getCloseNeighbours())
                {
                    if (stepField->getOutlineColor() == sf::Color::Transparent && !stepField->isOccupied())
                        stepField->setOutlineColor(sf::Color::Yellow);
                }
            }
        }
        else {
            isSelected = false;
            setOutlineColor(sf::Color::Transparent);
            for (StepField* neighbour : neighbours)
            {
                neighbour->setOutlineColor(sf::Color::Transparent);
            }
            for (StepField* neighbour : neighbours) {
                for (StepField* stepField : neighbour->getCloseNeighbours())
                {
                    if (stepField->getOutlineColor() != sf::Color::Transparent)
                        stepField->setOutlineColor(sf::Color::Transparent);
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////
    void StepField::addNeighbour(StepField* stepField)
    {
        neighbours.push_back(stepField);
        stepField->neighbours.push_back(this);
    }

    ////////////////////////////////////////////////////////////
    bool StepField::isCloseNeighbourOf(StepField* field) const
    {
        return std::ranges::find(neighbours, field) != neighbours.end();
    }

    ////////////////////////////////////////////////////////////
    bool StepField::isDistantNeighbourOf(StepField* field) const 
    {
        for (StepField* f : field->getCloseNeighbours())
            if (isCloseNeighbourOf(f))
                return true;
        return false;
    }

    ////////////////////////////////////////////////////////////
    bool StepField::mouseEvent(sf::RenderWindow& window) const 
    {
        sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        return sqrt(pow((int)(worldPos.x - (getPosition().x + Radius)), 2) + pow((int)(worldPos.y - (getPosition().y + Radius)), 2)) <= radius;
    }

    ////////////////////////////////////////////////////////////
    int StepField::getID() const { return ID; }

    ////////////////////////////////////////////////////////////
    bool StepField::getSelected() const { return isSelected; }

    ////////////////////////////////////////////////////////////
    int StepField::getNearestGamechipCount(sf::Color color) const
    {
        return std::ranges::count_if(neighbours, [color](StepField* f) -> bool {return f != nullptr && f->isOccupied() && f->getGameChip()->getColor() == color; });
    }

    ////////////////////////////////////////////////////////////
    std::vector<StepField*> StepField::getCloseNeighbours() const { return neighbours; }

    ////////////////////////////////////////////////////////////
    std::vector<StepField*> StepField::getDistantNeighbours() const
    {
        std::set<StepField*> s{};
        for (StepField* field : neighbours) {
            s.insert_range(field->getCloseNeighbours());
        }
        for (StepField* field : neighbours) {
            std::erase_if(s, [field](StepField* f)->bool {return f == field; });
        }
        return std::vector<StepField*>(s.begin(), s.end());
    };

    ////////////////////////////////////////////////////////////
    std::vector<StepField*> StepField::getFreeCloseNeighbours() const
    {
        std::vector<StepField*> v(neighbours);
        auto range = std::ranges::remove_if(v, [](StepField* f) -> bool {return f == nullptr || f->isOccupied(); });
        v.erase(range.begin(), range.end());
        return v;
    }

    ////////////////////////////////////////////////////////////
    std::vector<StepField*> StepField::getFreeDistantNeighbours() const
    {
        std::vector<StepField*> v = getDistantNeighbours();
        auto range = std::ranges::remove_if(v, [](StepField* f) -> bool {return f == nullptr || f->isOccupied(); });
        v.erase(range.begin(), range.end());
        return v;
    }

    ////////////////////////////////////////////////////////////
    int StepField::count = 0;


    /***********************************************************/
    /// GameStatus class methods initialisation.
    /***********************************************************/
    Board::GameStatus::GameStatus(const Board* board) : board(board) {
        points_b = 0;
        points_r = 0;
        time_t t = std::time(nullptr);
        start_time = *std::localtime(&t);
    };

    ////////////////////////////////////////////////////////////
    void Board::GameStatus::calculateProgress()
    {
        if (is_running) {
            points_r = 0;
            points_b = 0;
            bool r_step = false;
            bool b_step = false;
            int fields_count = 0;

            for (std::vector<StepField*> list : board->fields) {
                auto ra_1 = std::ranges::partition(list.begin(), list.end(), [](StepField* f)->bool {return f != nullptr && f->isOccupied(); });
                auto ra_2 = std::ranges::partition(list.begin(), ra_1.begin(), [](StepField* f)->bool {return f->getGameChip()->getColor() == sf::Color::Red; });
                for (auto i = list.begin(); i < ra_2.begin() && !r_step; i++) {
                    if ((*i)->getGameChip()->canMakeStep())
                        r_step = true;
                }
                for (auto i = ra_2.begin(); i < ra_1.begin() && !b_step; i++) {
                    if ((*i)->getGameChip()->canMakeStep())
                        b_step = true;
                }
                points_r += ra_2.begin() - list.begin();
                points_b += ra_1.begin() - ra_2.begin();
                fields_count += list.size();
            }
            if (points_r == 0 || points_b == 0 || points_r + points_b >= fields_count || !r_step || !b_step) {
                time_t t = std::time(nullptr);
                end_time = *std::localtime(&t);

                is_running = false;
            }
            changed = true;
        }
    }

    ////////////////////////////////////////////////////////////
    int Board::GameStatus::getRedPoints() const { return points_r; }

    ////////////////////////////////////////////////////////////
    int Board::GameStatus::getBluePoints() const { return points_b; }

    ////////////////////////////////////////////////////////////
    bool Board::GameStatus::isRunning() const { return is_running; }

    ////////////////////////////////////////////////////////////
    void Board::GameStatus::addRedScore(int score) { red_score += score; }

    ////////////////////////////////////////////////////////////
    void Board::GameStatus::addBlueScore(int score) { blue_score += score; }

    ////////////////////////////////////////////////////////////
    int Board::GameStatus::getRedScore() const { return red_score; }

    ////////////////////////////////////////////////////////////
    int Board::GameStatus::getBlueScore() const { return blue_score; }

    bool Board::GameStatus::isChanged() const { return changed; }

    ////////////////////////////////////////////////////////////
     std::string Board::GameStatus::getTime() {
         end_time.tm_hour = end_time.tm_hour - start_time.tm_hour;
         end_time.tm_min = end_time.tm_min - start_time.tm_min;
         end_time.tm_sec = end_time.tm_sec - start_time.tm_sec;
         if (end_time.tm_sec < 0) {
             end_time.tm_min--;
             end_time.tm_sec = 60 + end_time.tm_sec;
         }
         if (end_time.tm_min < 0) {
             end_time.tm_hour--;
             end_time.tm_min = 60 + end_time.tm_min;
         }
        return  (end_time.tm_hour < 10 ? "0" : "") + std::to_string(end_time.tm_hour) + ":" +
                (end_time.tm_min < 10 ? "0" : "") + std::to_string(end_time.tm_min) + ":" + 
                (end_time.tm_sec < 10 ? "0" : "") + std::to_string(end_time.tm_sec);
    }


    /***********************************************************/
    /// Board class methods initialisation.
    /***********************************************************/
     Board::Board(float fieldRadius, std::string file_name) : 
         save_name(file_name),
         loaded(true),
         fieldRadius(fieldRadius), 
         AI(HexxagonAI(this)), 
         progress(new GameStatus(this)) 
     {
         generateField();
         std::fstream stream = std::fstream("Saves\\" + file_name + (file_name.ends_with(".bin") ? "" : ".bin"), std::ios::in | std::ios::binary);
         int buff;
         for(int i = 0; i < 9; i++)
         {
             stream >> buff;
             switch (i)
             {
             case 0: progress->points_r = buff; break;
             case 1: progress->points_b = buff; break;
             case 2: progress->red_score = buff; break;
             case 3: progress->blue_score = buff; break;
             case 4: progress->start_time.tm_sec = buff; break;
             case 5: progress->start_time.tm_min = buff; break;
             case 6: progress->start_time.tm_hour = buff; break;
             case 7: player = buff; break;
             case 8: AI_game = buff; break;
             default:
                 break;
             }
         }
         unsigned int field_status = 0;
         for (const std::vector<StepField*> list : fields) {
             for(StepField* field : list){
                 stream >> field_status;
                 if (field != nullptr && field_status & 0b100) {
                     field->occupy(new GameChip((field_status & 0b10 ? sf::Color::Blue : sf::Color::Red), field));
                     field->setSelected(field_status & 1);
                 }
             }
         }
     }

    ////////////////////////////////////////////////////////////
    Board::Board(float fieldRadius, bool AI_game) : fieldRadius(fieldRadius), AI_game(AI_game), AI(HexxagonAI(this)), progress(new GameStatus(this))
    {
        generateField();
    }

    ////////////////////////////////////////////////////////////
    Board::~Board(){
        delete selected_f;
        delete progress;
    }

    ////////////////////////////////////////////////////////////
    void Board::generateField() {
        for (int i = 0; i < 9; i++)
        {
            fields.push_back(std::vector<StepField*>{});
            for (int j = 0; j < 9 - abs(i - 4); j++)
            {
                if (!(((i == 3 || i == 5) && j == 4) || (i == 4 && j == 3)))
                {
                    fields[i].push_back(new StepField(fieldRadius, 6));
                    if (i - 1 >= 0) {
                        if (i < 5 && j > 0 && fields[i - 1][j - 1] != nullptr)
                            fields[i - 1][j - 1]->addNeighbour(fields[i][j]);
                        else if (i >= 5 && j < fields[i - 1].size() && fields[i - 1][j + 1] != nullptr)
                            fields[i - 1][j + 1]->addNeighbour(fields[i][j]);

                        if (j < fields[i - 1].size() && fields[i - 1][j] != nullptr)
                            fields[i - 1][j]->addNeighbour(fields[i][j]);
                    }

                    if (j > 0 && fields[i][j - 1] != nullptr)
                        fields[i][j - 1]->addNeighbour(fields[i][j]);
                }
                else
                    fields[i].push_back(nullptr);
            }

            if (fields[i].size() == 5)
            {
                fields[i][0]->occupy(new GameChip(sf::Color::Blue, fields[i][0]));
                fields[i][fields[i].size() - 1]->occupy(new GameChip(sf::Color::Red, fields[i][0]));
            }
            if (fields[i].size() == 9)
            {
                fields[i][0]->occupy(new GameChip(sf::Color::Red, fields[i][0]));
                fields[i][fields[i].size() - 1]->occupy(new GameChip(sf::Color::Blue, fields[i][0]));
            }
        }
        yDistance = fieldRadius * 0.86602540378443864676372317075294f; //sqrt(3)/2
        size = 9.f * (yDistance * 2 + 4);
        initFieldsLocation();
    }

    ////////////////////////////////////////////////////////////
    void Board::draw(sf::RenderTarget& target, const sf::RenderStates& states) const
    {
        for (const std::vector<StepField*>& list : fields)
            for (StepField* field : list) {
                if (field != nullptr)
                    target.draw(*field, states);
            }
    }

    ////////////////////////////////////////////////////////////
    void Board::initFieldsLocation()
    {
        float y_shift = getPosition().y;

        for (int i = 0; i < fields.size(); i++)
        {
            float x_shift = getPosition().x;

            for (int j = abs(i - 4); j > 0; j--)
                x_shift += fieldRadius;

            for (int j = 0; j < fields[i].size(); j++)
            {
                if (fields[i][j] != nullptr)
                    fields[i][j]->setPosition({ x_shift, y_shift });
                x_shift += fieldRadius * 2;
            }
            y_shift += yDistance * 2 + 2;
        }
    }

    ////////////////////////////////////////////////////////////
    void Board::setLocation(int x, int y)
    {
        setPosition({ (float)x - (size / 2.f), (float)y - (size / 2.f) });
        initFieldsLocation();
    }

    void Board::clearSelected() {
        if (selected_f != nullptr) {
            selected_f->setSelected(false);
            selected_f = nullptr;
        }
    }

    ////////////////////////////////////////////////////////////
    void Board::mousePressed(sf::RenderWindow& window)
    {
        bool pressed = false;

        for (int i = 0; progress->isRunning() && i < fields.size() && !pressed; i++)
        {
            for (int j = 0; j < fields[i].size() && !pressed; j++)
            {
                if (fields[i][j] != nullptr && fields[i][j]->mouseEvent(window))
                {
                    if (fields[i][j] != selected_f && fields[i][j]->isOccupied() &&
                        ((player == 0 && fields[i][j]->getGameChip()->getColor() == sf::Color::Red) ||
                            (player == 1 && fields[i][j]->getGameChip()->getColor() == sf::Color::Blue))) {
                        clearSelected();
                        fields[i][j]->setSelected(true);
                        selected_f = fields[i][j];
                    } else
                        makeStep(fields[i][j]);
                }
            }
        }
    }

    ////////////////////////////////////////////////////////////
    void Board::makeStep(StepField* field)
    {
        if (selected_f != nullptr && field != nullptr && !field->isOccupied())
        {
            if (field->isCloseNeighbourOf(selected_f))
                doubleCheap(*selected_f->getGameChip(), field);
            else if (field->isDistantNeighbourOf(selected_f))
                moveCheap(selected_f->getGameChip(), field);
            progress->calculateProgress();

        if (AI_game && player == 1)
            AI.makeStep();
        }

        clearSelected();
    }

    ////////////////////////////////////////////////////////////
    void Board::doubleCheap(GameChip& chip, StepField* field)
    {
        if (chip.getColor() == sf::Color::Red)
            progress->addRedScore(10);
        else
            progress->addBlueScore(10);
        field->occupy(new GameChip(chip.getColor(), field));
        checkNeighbours(field->getGameChip());
    }

    ////////////////////////////////////////////////////////////
    void Board::moveCheap(GameChip* chip, StepField* field)
    {
        chip->getField()->makeFree();
        field->occupy(chip);
        checkNeighbours(field->getGameChip());
    }

    ////////////////////////////////////////////////////////////
    void Board::checkNeighbours(GameChip* chip)
    {
        for (StepField* neighbour : chip->getField()->getCloseNeighbours())
            if (neighbour->isOccupied() && neighbour->getGameChip() != nullptr && neighbour->getGameChip()->getColor() != chip->getColor()) {
                if (chip->getColor() == sf::Color::Red)
                    progress->addRedScore(30);
                else
                    progress->addBlueScore(30);
                neighbour->occupy(chip);
            }
        nextPlayer();
    }

    ////////////////////////////////////////////////////////////
    Board::GameStatus* Board::getGameProgress() const { return progress; }

    ////////////////////////////////////////////////////////////
    std::vector<StepField*> Board::getFields() const {
        std::vector<StepField*> v{};
        for (std::vector<StepField*> list : fields) {
            v.insert(v.begin(), list.begin(), list.end());
        }
        return v;
    }

    ////////////////////////////////////////////////////////////
    void Board::nextPlayer() { player = abs(player - 1); }

    ////////////////////////////////////////////////////////////
    void Board::save(std::string file_name) {
        std::fstream stream = std::fstream("Saves\\" + file_name + (file_name.ends_with(".bin") ? "" : ".bin"), std::ios::out | std::ios::trunc);
        stream << progress->points_r << '\n';
        stream << progress->points_b << '\n';
        stream << progress->red_score << '\n';
        stream << progress->blue_score << '\n';
        stream << progress->start_time.tm_sec << '\n';
        stream << progress->start_time.tm_min << '\n';
        stream << progress->start_time.tm_hour << '\n';
        stream << player << '\n';
        stream << AI_game << '\n';
        unsigned int field_status = 0;
        for (const std::vector<StepField*> list : fields) {
            for (StepField* field : list) {
                if (field != nullptr && field->getGameChip() != nullptr) {
                    field_status |= (field->isOccupied() << 2);
                    if (field_status) {
                        field_status |= ((field->getGameChip()->getColor() == sf::Color::Blue ? 1 : 0) << 1);
                        field_status |= field->isSelected;
                    }
                    stream << field_status << "\n";
                    stream;
                }
                else {
                    field_status &= 0;
                    stream << field_status << "\n";
                }
                field_status = 0;
            }
        }
        stream.close();
    }

    ////////////////////////////////////////////////////////////
    std::string Board::getSaveName() const { return save_name; }

    ////////////////////////////////////////////////////////////
    bool Board::wasLoaded() const { return loaded; }
};