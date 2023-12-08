#pragma once

#include <SFML/Graphics.hpp>
#include <filesystem>
#include <string>
#include <iostream>
#include <algorithm>

namespace sf
{
	class Button {
	protected:

		enum Status { Default, Hovered, Pressed };

		Status status = Default;

		bool clicked = false;

		Button() {};

	public:
		/// Basic event handling.
		///
		virtual void handleEvent(RenderWindow& window, sf::Event event) = 0;

		/// Getters
		///
		bool wasClicked();		//!< returns 'true' if button was clicked recently
	};

	class G_Button : public RectangleShape, public Button {
	private:
		Texture regular;
		Texture pressed;
		Texture hover;

	protected:
		bool insideBounds(RenderWindow& window) const;		//!< returns 'true' if mouse is inside button bounds

	public:
		G_Button(std::string button_name, const Vector2f& size);

		/// Basic event handling.
		///
		void handleEvent(RenderWindow& window, sf::Event event) override;

		/// Setters
		///
		void setPosition(const Vector2f& position);
	};
	
	class RadioButton;

	class RadioGroup {
	private:
		RadioButton* selected;

	public:
		void enable(RadioButton* button);		//!< button activation
	};

	class RadioButton : public Button, public CircleShape {
	private:
		Color checkFillColor = Color::Blue;
		Color hoverColor = Color::Magenta;
		CircleShape fillShape;

		bool square = false;
		bool checked = false;
		bool hovered = false;

		RadioGroup* group;
		std::string value;

		bool insideBounds(RenderWindow& window) const;		//!< returns 'true' if mouse is inside button bounds

		void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;

	public:
		RadioButton(int radius, bool square, auto value, RadioGroup* group);

		RadioButton(int radius, auto value, RadioGroup* group) : RadioButton(radius, false, value, group) {};

		/// Basic event handling.
		///
		void handleEvent(RenderWindow& window, sf::Event event) override;

		void enable();		//!< button activation

		void disable();		//!< button deactivation

		/// Setters
		///
		void setCheckFillColor(Color color);

		void setBorder(float thickness);

		void setPosition(const Vector2f& position);

		/// Getters
		///
		bool isChecked() const;		//!< return 'true' if checked
	};

	class TextField : public Drawable, public Transformable {
	private:
		Text text;
		RectangleShape rectangle;
		RectangleShape caret;

		bool clicked = false;
		int caret_shift = 0;

		/// Basic text field logic, when
		/// text has been typed.
		///
		void type(Event event);

		/// Moving caret to prev/next character
		/// depending on direction.
		///
		void moveCaret(int direction);

		void draw(RenderTarget& target, const RenderStates& states) const;
	public:
		TextField(Vector2f size, Font& font, Color font_color, int character_size, int border_hickness, Color border_color);

		/// Basic event handling.
		///
		void handleEvent(sf::RenderWindow& window, const Event event);

		/// Reset all textfield data:
		/// caret position, entered text.
		/// 
		void clear();

		/// Setters
		///
		void setFillColor(Color color);

		void setPosition(const Vector2f position);

		/// Getters
		///
		std::string getText() const;

		bool wasClicked();

		Vector2f getSize();
	};
}