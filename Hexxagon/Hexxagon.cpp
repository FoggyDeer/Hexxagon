#include "Hexxagon.h"
using namespace std;

namespace sf {
	RadioButton::RadioButton(int radius, bool square, auto value, RadioGroup* group) : value(value), group(group), Button(), CircleShape(radius, (square ? 4 : 0)) {
		setFillColor(Color::White);
		setOutlineColor(hoverColor);
		fillShape = CircleShape(radius / 2, (square ? 4 : 0));
		fillShape.setFillColor(checkFillColor);
		fillShape.setPosition({ getPosition().x - radius, getPosition().y - radius });
	};
}

/// Class for recording game info
/// into file.
/// 
class ScoreRec : public sf::Text {
private:
	string nickname;
	int score;
	int points;
	string time;

	void verify() {
		if (score < 0)
			score = 0;
		if (points < 0)
			points = 0;
	}
public:
	ScoreRec(string input, sf::Font& font) : sf::Text(font, "", 50) {
		auto str_input = stringstream(input);
		getline(str_input, nickname, '_');
		string buff;
		getline(str_input, buff, ';');
		score = stoi(buff);
		getline(str_input, buff, '|');
		points = stoi(buff);
		str_input >> time;
		verify();

		setString(nickname + " - " + std::to_string(score) + " (" + std::to_string(points) + " points) " + time);
	}

	ScoreRec(Hexxagon::Board::GameStatus* status, sf::Font& font) : sf::Text(font, "", 50)
	{
		bool red_won = status->getRedPoints() > status->getBluePoints();
		if (red_won) {
			nickname = "Red";
			score = status->getRedScore();
			points = status->getRedPoints();
		}
		else {
			nickname = "Blue";
			score = status->getBlueScore();
			points = status->getBluePoints();
		}
		time = status->getTime();

		verify();
		setString(nickname + " - " + std::to_string(score) + " (" + std::to_string(points) + ")");
	}

	/// Basic file read logic, which returns
	/// vector of saved scores info.
	///
	static vector<ScoreRec> read_file(std::fstream& stream, sf::Font& font) {
		vector<ScoreRec> v;
		string buff;
		while (std::getline(stream, buff)) {
			v.emplace_back(buff, font);
		}
		return v;
	};

	/// Basic file write logic, which writes info
	/// from vector of ScoreRec objects.
	///
	static void write_file(vector<ScoreRec> v, string file_path) {
		fstream stream(file_path, ios::out | ios::trunc);
		for (ScoreRec elem : v) {
			stream << elem << '\n';
		}
		stream.close();
	};

	/// Score table sorting.
	///
	static void sort(vector<ScoreRec>& v) {
		if (v.size() > 0) {
			for (int i = 0; i < v.size(); i++) {
				for (int j = i + 1; j < v.size(); j++) {
					if (v[i] < v[j]) {
						ScoreRec buff = v[i];
						v[i] = v[j];
						v[j] = buff;
					}
				}
			}
		}
	}

	/// Getters
	///
	string getNickname() const {
		return nickname;
	}

	int getScore() const {
		return score;
	}

	int getPoints() const {
		return points;
	}

	/// Operators
	///
	friend std::ostream& operator <<(std::fstream& fstream, const ScoreRec& score_obj) {
		return fstream << score_obj.nickname << "_" << score_obj.score << ';' << score_obj.points << "|" << score_obj.time;
	}
	friend std::ostream& operator <<(std::ostream& fstream, const ScoreRec& score_obj) {
		return fstream << score_obj.nickname << "_" << score_obj.score << ';' << score_obj.points << "|" << score_obj.time;
	}

	friend bool operator <(ScoreRec& score_1, ScoreRec& score_2) {
		return score_1.score < score_2.score;
	}

	friend bool operator >(ScoreRec& score_1, ScoreRec& score_2) {
		return score_1.score > score_2.score;
	}
};

///////////////////////////////////////////////////
/// High score panel rendering function.
///////////////////////////////////////////////////
void highScorePanelRender(sf::RenderWindow& window) {
	auto file_stream = std::fstream(
		"Saves\\scores.txt",
		std::ios::in
	);

	sf::Font font;
	font.loadFromFile("Assets\\BradBunR.ttf");
	vector<ScoreRec> scores = ScoreRec::read_file(file_stream, font);
	file_stream.close();

	ScoreRec::sort(scores);

	float y_shift = 80.f;
	for (ScoreRec& score : scores) {
		score.setPosition({ window.getSize().x / 2.f - score.getLocalBounds().getSize().x / 2.f, y_shift });
		y_shift += score.getLocalBounds().getSize().y + 20.f;
	}
	sf::Event event;

	while (window.isOpen()) {
		window.clear();
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
				return;
			}
		}
		for (ScoreRec& score : scores) {
			window.draw(score);
		}

		window.display();
	}
}

void menuRender(sf::RenderWindow& window);

///////////////////////////////////////////////////
/// Game panel rendering function.
///////////////////////////////////////////////////
void gameRender(sf::RenderWindow& window, bool playWithAI = false, string path = "") {
	sf::Event event;
	Hexxagon::Board* board;
	if(path.length() > 0)
		board = new Hexxagon::Board(35, path);
	else
		board = new Hexxagon::Board(35, playWithAI);

	board->getGameProgress()->calculateProgress();
	board->setLocation(window.getSize().x / 2, window.getSize().y / 2);

	int red_rect_width;
	int blue_rect_width;

	sf::RectangleShape red_rect({ 6.f, 50.f });
	red_rect.setPosition({ 0.f, window.getSize().y / 2.f - 75 });
	red_rect.setFillColor(sf::Color::Red);
	sf::RectangleShape blue_rect({ 6.f, 50.f });
	blue_rect.setPosition({ 0.f, window.getSize().y / 2.f + 25 });
	blue_rect.setFillColor(sf::Color::Blue);

	sf::Font font;
	font.loadFromFile("Assets\\BradBunR.ttf");
	sf::Text rp_count(font, "3", 30);
	rp_count.setPosition({ 20.f, red_rect.getPosition().y });
	sf::Text bp_count(font, "3", 30);
	bp_count.setPosition({ 20.f, blue_rect.getPosition().y });

	sf::Text red_score(font, "Score: 0", 50);
	red_score.setPosition({ 20.f, 0.f });
	sf::Text blue_score(font, "Score: 0", 50);
	blue_score.setPosition({ 20.f, window.getSize().y - 70.f });

	sf::Text final_text(font, "", 250);
	final_text.setOutlineColor(sf::Color(255, 103, 0));
	final_text.setOutlineThickness(5);

	auto file_stream = std::fstream(
		"Saves\\scores.txt",
		std::ios::in
	);
	vector<ScoreRec> scores = ScoreRec::read_file(file_stream, font);
	file_stream.close();

	ScoreRec::sort(scores);

	font.loadFromFile("Assets\\BradBunR.ttf");
	sf::TextField text_field({350.f, 50.f}, font, sf::Color::White, 40, 4, sf::Color::White);
	text_field.setFillColor(sf::Color::Black);
	text_field.setPosition({ window.getSize().x / 2.f - text_field.getSize().x / 2.f, window.getSize().y / 2.f - text_field.getSize().y / 2.f });

	sf::Text text_title(font, "Enter save name:", 40);
	text_title.setPosition({ window.getSize().x / 2.f - text_title.getGlobalBounds().getSize().x / 2.f, window.getSize().y / 4.f - text_field.getSize().y / 2.f });
	bool wrong = false;

	bool score_updated = false;
	bool text_field_opened = false;
	while (window.isOpen()) {
		window.clear();
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed || event.type == sf::Event::MouseEntered) {
				board->mousePressed(window);
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::Escape) {
					if (board->getGameProgress()->isRunning()) {
						if (text_field_opened) {
							text_field_opened = false;
							text_field.clear();
						}
						else if (board->wasLoaded()) {
							board->save(board->getSaveName());
							return;
						}
						else
							text_field_opened = true;
					}
					else
						return;
				}
				else if (text_field_opened && event.key.code == sf::Keyboard::Enter) {
					board->save(text_field.getText());
					return;
				}
			}

			if (board->getGameProgress()->isChanged()) {
				int rp = board->getGameProgress()->getRedPoints();
				int bp = board->getGameProgress()->getBluePoints();
				red_rect_width = rp * 2;
				blue_rect_width = bp * 2;

				red_rect.setSize({ (float)red_rect_width , 50.f });
				blue_rect.setSize({ (float)blue_rect_width , 50.f });

				rp_count.setString(std::to_string(rp));
				bp_count.setString(std::to_string(bp));

				red_score.setString("Score: " + std::to_string(board->getGameProgress()->getRedScore()));
				blue_score.setString("Score: " + std::to_string(board->getGameProgress()->getBlueScore()));

				if (!board->getGameProgress()->isRunning()) {
					if (!score_updated) {
						if (rp > bp) {
							final_text.setString("Reds Won!");
							final_text.setFillColor(sf::Color(227, 38, 54));
						}
						else if (rp < bp) {
							final_text.setString("Blue Won!");
							final_text.setFillColor(sf::Color(0, 71, 171));
						}
						else {
							final_text.setString("Draw!");
							final_text.setFillColor(sf::Color::White);
						}
						final_text.setPosition({
							window.getSize().x / 2.f - final_text.getLocalBounds().getSize().x / 2.f,
							window.getSize().y / 2.f - 180.f });
					}
				}
			}

			if(text_field_opened)
				text_field.handleEvent(window, event);
		}

		if (text_field_opened) {
			window.draw(text_field);
			window.draw(text_title);
		}
		else {
			window.draw(*board);
			window.draw(red_rect);
			window.draw(blue_rect);
			window.draw(rp_count);
			window.draw(bp_count);
			window.draw(red_score);
			window.draw(blue_score);
		}

		if (!board->getGameProgress()->isRunning()) {
			if (!score_updated) {
				score_updated = true;
				ScoreRec score(board->getGameProgress(), font);
				if (scores.size() > 4) {
					if (scores[scores.size() - 1] < score)
						scores[scores.size() - 1] = score;
				}
				else
					scores.push_back(score);
				ScoreRec::write_file(scores, "Saves\\scores.txt");
			}
			window.draw(final_text);
		}

		window.display();
	}
}

void gameRender(sf::RenderWindow& window, string path) {
	gameRender(window, false, path);
}

///////////////////////////////////////////////////
/// Menu panel rendering function.
///////////////////////////////////////////////////
void menuRender(sf::RenderWindow& window) {
	if (window.isOpen()) {
		sf::Event event;

		sf::G_Button new_game_btn("New Game", { 300, 120 });
		new_game_btn.setPosition({ window.getSize().x / 2.f - 150.f, window.getSize().y / 2.f - 250.f });
		sf::G_Button continue_btn("Continue", { 300, 120 });
		continue_btn.setPosition({ window.getSize().x / 2.f - 150.f, window.getSize().y / 2.f - 125.f });
		sf::G_Button high_score_btn("High Score", { 300, 120 });
		high_score_btn.setPosition({ window.getSize().x / 2.f - 150.f, window.getSize().y / 2.f });

		sf::RadioGroup group = sf::RadioGroup();
		sf::RadioButton two_players_rbtn(30, true, "two players", &group);
		two_players_rbtn.setPosition({ window.getSize().x / 2.f - 120.f - 30, window.getSize().y / 2.f + 170.f });
		group.enable(&two_players_rbtn);
		sf::RadioButton one_players_rbtn(30, true, "one player", &group);
		one_players_rbtn.setPosition({ window.getSize().x / 2.f + 120.f - 30, window.getSize().y / 2.f + 170.f });
		one_players_rbtn.setCheckFillColor(sf::Color::Red);

		sf::Font font;
		font.loadFromFile("Assets\\Chase Dreams.ttf");
		sf::Text two_players_rbtn_label(font, "Play with Friend", 40);
		two_players_rbtn_label.setPosition({ window.getSize().x / 2.f - 400, window.getSize().y / 2.f + 175.f });
		sf::Text one_players_rbtn_label(font, "Play with Computer", 40);
		one_players_rbtn_label.setPosition({ window.getSize().x / 2.f + 180, window.getSize().y / 2.f + 175.f });

		bool text_field_opened = false;
		sf::Font font1;
		font1.loadFromFile("Assets\\BradBunR.ttf");
		sf::TextField text_field = sf::TextField({300.f, 50.f}, font1, sf::Color::White, 30, 3, sf::Color::White);
		text_field.setFillColor(sf::Color::Black);
		text_field.setPosition({ window.getSize().x / 2.f - text_field.getSize().x / 2.f, window.getSize().y / 2.f - text_field.getSize().y / 2.f});

		sf::Text text_title(font1, "Enter save name:", 40);
		text_title.setPosition({ window.getSize().x / 2.f - text_title.getGlobalBounds().getSize().x / 2.f, window.getSize().y / 4.f - text_field.getSize().y / 2.f });
		bool wrong = false;
		while (window.isOpen()) {
			window.clear();
			while (window.pollEvent(event)) {
				if (event.type == sf::Event::Closed) {
					window.close();
					return;
				}
				else if (event.type == sf::Event::KeyPressed) {
					if (wrong) {
						wrong = false;
						text_title.setString("Enter save name:");
						text_title.setFillColor(sf::Color::White);
					}
					if (event.key.code == sf::Keyboard::Escape) {
						if (text_field_opened) {
							text_field_opened = false;
							text_field.clear();
						}
						else {
							window.close();
							return;
						}
					}
					else if (text_field_opened && event.key.code == sf::Keyboard::Enter) {
						string path = "Saves\\" + text_field.getText() + (text_field.getText().ends_with(".bin") ? "" : ".bin");
						if (std::filesystem::exists(path)) {
							text_field_opened = false;
							gameRender(window, text_field.getText());
							text_field.clear();
						}
						else {
							text_title.setString("Wrong filename!");
							text_title.setFillColor(sf::Color::Red);
							wrong = true;
						}
					}
				}

				if (text_field_opened) {
					text_field.handleEvent(window, event);
				}
				else {
					new_game_btn.handleEvent(window, event);
					continue_btn.handleEvent(window, event);
					high_score_btn.handleEvent(window, event);
					one_players_rbtn.handleEvent(window, event);
					two_players_rbtn.handleEvent(window, event);
				}
			}

			if (text_field_opened) {
				window.draw(text_field);
				window.draw(text_title);
			}
			else {
				if (new_game_btn.wasClicked())
					gameRender(window, one_players_rbtn.isChecked());
				else if (high_score_btn.wasClicked())
					highScorePanelRender(window);
				else if (continue_btn.wasClicked())
					text_field_opened = true;

				window.draw(new_game_btn);
				window.draw(continue_btn);
				window.draw(high_score_btn);
				window.draw(one_players_rbtn);
				window.draw(two_players_rbtn);
				window.draw(one_players_rbtn_label);
				window.draw(two_players_rbtn_label);
			}

			window.display();
		}
	}
}

int main() {
	sf::RenderWindow window(
		sf::VideoMode({ 1250, 700 }),
		"TITILE");

	menuRender(window);

	return 0;
}