#include "ExtendedAssets.h"

namespace sf
{
	bool Button::wasClicked() {
		if (clicked) {
			return !(clicked = false);
		}
		return clicked;
	}

	G_Button::G_Button(std::string button_name, const Vector2f& size) : RectangleShape(size) {
		if (regular.loadFromFile("Assets\\" + button_name + "\\Regular.png")) {
			setTexture(&regular);
		}
		else {
			std::cout << "Regular texture could not load";
		}

		if (!pressed.loadFromFile("Assets\\" + button_name + "\\Pressed.png")) {
			std::cout << "Pressed texture could not load";
		}

		if (!hover.loadFromFile("Assets\\" + button_name + "\\Hover.png")) {
			std::cout << "Hover texture could not load";
		}
	}
	
	bool G_Button::insideBounds(RenderWindow& window) const {
		sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		return worldPos.x >= getPosition().x && worldPos.x <= getPosition().x + getSize().x &&
			worldPos.y >= getPosition().y && worldPos.y <= getPosition().y + getSize().y;
	}

	void G_Button::setPosition(const Vector2f& position) {
		RectangleShape::setPosition(position);
	}

	void G_Button::handleEvent(RenderWindow& window, sf::Event event) {
		if (insideBounds(window)) {
			if ((status == Pressed && event.type == sf::Event::MouseButtonReleased))
				clicked = true;

			if (status != Pressed && event.type == sf::Event::MouseButtonPressed) {
				status = Pressed;
				setTexture(&pressed);
			}
			else if ((status == Default && event.type == sf::Event::MouseMoved) || (status == Pressed && event.type == sf::Event::MouseButtonReleased)) {
				sf::Cursor cursor;
				if (cursor.loadFromSystem(sf::Cursor::Hand))
					window.setMouseCursor(cursor);
				status = Hovered;
				setTexture(&hover);
			}
		}
		else if (status != Default) {
			sf::Cursor cursor;
			if (cursor.loadFromSystem(sf::Cursor::Arrow))
				window.setMouseCursor(cursor);
			status = Default;
			setTexture(&regular);
		}
	}

	void RadioGroup::enable(RadioButton* button) {
		if (selected != button) {
			if (selected != nullptr)
				selected->disable();
			selected = button;
			selected->enable();
		}
	}

	void RadioButton::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
		target.draw(((CircleShape)*this), states);
		if (checked)
			target.draw(fillShape, states);

	}

	bool RadioButton::insideBounds(RenderWindow& window) const {
		sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
		return sqrt(pow((int)(worldPos.x - (CircleShape::getPosition().x + getRadius())), 2) + pow((int)(worldPos.y - (CircleShape::getPosition().y + getRadius())), 2)) <= getRadius();
	}

	void RadioButton::setCheckFillColor(Color color) {
		fillShape.setFillColor(color);
	}

	void RadioButton::setBorder(float thickness) {
		CircleShape::setOutlineThickness(thickness);
		CircleShape::setRadius(getRadius() - thickness);
	}

	void RadioButton::setPosition(const Vector2f& position) {
		CircleShape::setPosition(position);
		fillShape.setPosition({ CircleShape::getPosition().x + fillShape.getRadius(), CircleShape::getPosition().y + fillShape.getRadius() });
	}

	void RadioButton::enable() {
		checked = true;
	}

	void RadioButton::disable() {
		checked = false;
	}

	bool RadioButton::isChecked() const {
		return checked;
	}

	void RadioButton::handleEvent(RenderWindow& window, sf::Event event) {
		if (insideBounds(window)) {
			if (!hovered) {
				hovered = true;
				sf::Cursor cursor;
				if (cursor.loadFromSystem(sf::Cursor::Hand))
					window.setMouseCursor(cursor);
			}
			setOutlineThickness(4);
			if(event.type == Event::MouseButtonPressed)
				group->enable(this);
		}
		else {
			if (hovered) {
				hovered = false;
				sf::Cursor cursor;
				if (cursor.loadFromSystem(sf::Cursor::Arrow))
					window.setMouseCursor(cursor);
			}
			setOutlineThickness(0);
		}
	}

	TextField::TextField(Vector2f size, Font& font, Color font_color, int character_size, int border_thickness, Color border_color) : 
		text(font, "", character_size), 
		rectangle(size), 
		caret({ 2.f, size.y/3.f*2}) 
	{
		rectangle.setOutlineThickness(border_thickness);
		rectangle.setOutlineColor(border_color);
		text.setFillColor(font_color);
		text.setPosition({ 10.f, (size.y - text.getLocalBounds().getSize().y) / 2.f });
		caret.setFillColor(sf::Color::White);
		caret.setPosition({ 10.f, (size.y - caret.getLocalBounds().getSize().y) / 2.f});
	}

	void TextField::draw(RenderTarget& target, const RenderStates& states) const {
		target.draw(rectangle, states);
		target.draw(text, states);
		target.draw(caret, states);
	}

	void TextField::handleEvent(sf::RenderWindow& window, const sf::Event event) {
		if (event.type == Event::MouseButtonPressed){
			sf::Vector2f worldPos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

			if(	worldPos.x >= rectangle.getPosition().x &&
				worldPos.y >= rectangle.getPosition().y &&
				worldPos.x <= rectangle.getPosition().x + rectangle.getSize().x &&
				worldPos.y <= rectangle.getPosition().y + rectangle.getSize().y	)
				clicked = true;
		}
		else if (event.type == Event::TextEntered || (event.type == Event::KeyPressed && event.key.code == Keyboard::Delete))
			type(event);
		else if (event.type == Event::KeyPressed){
			if (caret_shift > 0 && event.key.code == Keyboard::Left) {
				moveCaret(-1);
			}
			else if (caret_shift < text.getString().getSize() && event.key.code == Keyboard::Right) {
				moveCaret(1);
			}
		}
	}

	void TextField::type(Event event) {
		std::string str = text.getString();
		int key = event.key.code;

		if (key == Keyboard::Delete) {
			if (caret_shift < text.getString().getSize()) {
				text.setString(str.erase(caret_shift, 1));
			}
		}
		else if (key == '\b') {
			if (caret_shift > 0) {
				text.setString(str.erase(caret_shift - 1, 1));
				moveCaret( - 1);
			}
		}
		else if (event.text.unicode > ' ' && text.getGlobalBounds().getSize().x < rectangle.getSize().x - 35.f) {
			text.setString(str.insert(caret_shift, 1, key));
			moveCaret(1);
		}
	}

	void TextField::moveCaret(int direction) {
		if (direction == 1 && caret_shift <= text.getString().getSize()) {
			if (caret_shift < text.getString().getSize() - 1) {
				caret.setPosition({ text.findCharacterPos(caret_shift+1).x, caret.getPosition().y});
			}
			else
				caret.setPosition({ text.getPosition().x + text.getGlobalBounds().getSize().x, caret.getPosition().y});
			caret_shift++;
		} else if (direction == -1 && caret_shift >= 0) {
			if (caret_shift > 1) {
				caret.setPosition({ text.findCharacterPos(caret_shift - 1).x, caret.getPosition().y });
			}
			else
				caret.setPosition({ text.getPosition().x, caret.getPosition().y });
			caret_shift--;
		}
	}

	void TextField::clear() {
		text.setString("");
		caret_shift = 0;
		caret.setPosition({rectangle.getPosition().x + 10.f, caret.getPosition().y});
	}

	void TextField::setFillColor(Color color) {
		rectangle.setFillColor(color);
	}

	bool TextField::wasClicked() {
		return clicked;
	}

	std::string TextField::getText() const {
		return text.getString();
	}

	Vector2f TextField::getSize() {
		return rectangle.getSize();
	}

	void TextField::setPosition(const Vector2f position) {
		rectangle.setPosition(position);
		text.setPosition({text.getGlobalBounds().getPosition().x + position.x , position.y});
		caret.setPosition({caret.getPosition().x + position.x, caret.getPosition().y + position.y});
	}
}