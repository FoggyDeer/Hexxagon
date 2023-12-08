#pragma once

#include <algorithm>
#include <vector>
#include <set>
#include <string>
#include <fstream>
#include <iostream>
#include <SFML/Graphics.hpp>
#include "HexxagonAI.h"

namespace Hexxagon
{

    class HexxagonAI;

    ////////////////////////////////////////////////////////////
    /// GameChip objects will be placed into board cells.
    ////////////////////////////////////////////////////////////
    class GameChip 
    {
    private:
        sf::Color color;            //!< affects the player's number
        StepField* currentField;    //!< pointer of current game board cell

    public:
        GameChip(sf::Color color, StepField* currentField);

        /// Setters
        ///
        void setColor(sf::Color c);     //!< sets gamechip color

        void setField(StepField* field);     //!< sets current game field

        /// Getters
        /// 
        sf::Color getColor() const;      //!< returns color

        StepField* getField() const;      //!< returns current field

        bool canMakeStep() const;      //!< returns 'true' if surrounded from all sides.
    };

    ////////////////////////////////////////////////////////////
    /// Game board cell class, whose objects will
    /// contain GameChip objects and will be rendered
    /// on display.
    ////////////////////////////////////////////////////////////
    class StepField : public sf::CircleShape 
    {
        static int count;

    private:
        int ID;
        bool isSelected = false;

        GameChip* gameChip;

        float Radius;       //!< The radius of the circumscribed circle
        float radius;       //!< The radius of the inscribed circle

        std::vector<StepField*> neighbours;     //!< vector of neighbour game board cells

    public:

        StepField(float Radius, int pointCount);

        bool isOccupied() const;      //!< returns 'true' if contains gamechip

        void occupy(GameChip* chip);    //!< sets gamechip to gameChip variable

        void makeFree();        //!< removes gamechip from gameChip variable

        void addNeighbour(StepField* stepField);

        bool isCloseNeighbourOf(StepField* field) const;      //!< returns 'true' if neighbours vector contains 'field' pointer

        bool isDistantNeighbourOf(StepField* field) const;      //!< returns 'true' if any neighbour contains neighbour with 'field' pointer

        bool isBorder() const{
            return std::ranges::find_if(neighbours, [](StepField* f) -> bool { return f != nullptr && !f->isOccupied(); }) != neighbours.end();
        }

        bool mouseEvent(sf::RenderWindow& window) const;      //!< returns 'true' if mouse pressed on field area, selects it and it's empty neighbours

        GameChip* getGameChip() const;

        int getID() const;

        bool getSelected() const;

        void setSelected(bool selected);

        int getNearestGamechipCount(sf::Color color) const;     //!< returns count of nearest gamechips with provided color

        std::vector<StepField*> getCloseNeighbours() const;

        std::vector<StepField*> getDistantNeighbours() const;

        std::vector<StepField*> getFreeCloseNeighbours() const;

        std::vector<StepField*> getFreeDistantNeighbours() const;

        friend class HexxagonAI;
        friend class Board;
    };

    ////////////////////////////////////////////////////////////
    /// Game board class, which implements basic
    /// game logic and functionality.
    ////////////////////////////////////////////////////////////
    class Board : public sf::Drawable, public sf::Transformable
    {
    public:
        ////////////////////////////////////////////////////////////
        /// Game status class, which contains information
        /// about game progress.
        ////////////////////////////////////////////////////////////
        class GameStatus
        {
        private:
            int points_r = 0;
            int points_b = 0;
            int red_score = 0;
            int blue_score = 0;
            bool is_running = true;
            bool changed = false;
            tm start_time;
            tm end_time;
            const Board* board;

        public:
            GameStatus(const Board* board);

            void calculateProgress();     //!< recalculate game information based on game board

            int getRedPoints() const;       //!< returns red points count

            int getBluePoints() const;      //!< returns blue points count

            bool isRunning() const;      //!< returns game status

            void addRedScore(int score);

            void addBlueScore(int score);

            int getRedScore() const;

            int getBlueScore() const;

            bool isChanged() const;

            std::string getTime();

            friend class Board;
        };

    private:
        float yDistance;        //!< length of the Y vector between centers of two hexagons

        float size;             //!< width and height of game board

        float fieldRadius = 35;      //!< radius of 1 game board's cell

        int player = 0;

        StepField* selected_f = nullptr;    //!< current selected game board cell

        GameStatus* progress = nullptr;    //!< containts game progress info

        bool AI_game = false;       //!< 'true' if user is playing with computer

        std::string save_name = "";       //!< file name of save

        bool loaded = false;       //!< 'true' if game was loaded from save file

        HexxagonAI AI;

        std::vector<std::vector<StepField*>> fields;

        void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;

        /// Basic steps logic, where is invoking
        /// doubleCheap() or moveCheap() methods.
        ///
        void makeStep(StepField* field);

        /// Initialization of every game board field 
        /// to default value.
        ///
        void generateField();

        /// Inits location of every game board cell on display 
        /// relatively to game board location.
        ///
        void initFieldsLocation();

        /// Creates new gamechip and moves in to provided
        /// field cell.
        ///
        void doubleCheap(GameChip& chip, StepField* field);

        /// Moves provided gamechip from current field cell
        /// to another
        ///
        void moveCheap(GameChip* chip, StepField* field);

        /// Checks if gamechip touches any opponent's
        /// game chip, and occupies it
        ///
        void checkNeighbours(GameChip* chip);

        /// Deactivates selected game cell.
        ///
        void clearSelected();

    public:
        Board(float fieldRadius, bool AI_game);

        Board(float fieldRadius, std::string path);

        ~Board();

        /// Checks if mouse hover on any game board
        /// cell and invokes main functionality.
        ///
        void mousePressed(sf::RenderWindow& window);

        /// Changes current player's number
        ///
        void nextPlayer();

        /// Saving game board to provided file.
        ///
        void save(std::string file_name);

        /// return a save file path
        ///
        std::string getSaveName() const;

        void setLocation(int x, int y);

        /// Getters
        /// 
        GameStatus* getGameProgress() const;

        std::vector<StepField*> getFields() const;

        bool wasLoaded() const;

        friend class HexxagonAI;
    };
}