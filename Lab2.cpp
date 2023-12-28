#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include "imgui-SFML.h"
#include "imgui.h"
float RAnd(float w1, float w2) { return w1 + w2 + std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }
float ROr(float w1, float w2) { return w1 + w2 - std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }
float figure(const sf::Vector2f &point)
{
	std::function<float(const sf::Vector2f &)> rFuncs[6];

	rFuncs[0] = [](const sf::Vector2f &point) -> float { return std::pow(point.x, 2) + std::pow(point.y - 12, 2) + 12; };
	rFuncs[1] = [](const sf::Vector2f &point) -> float { return std::pow(point.x - 15, 2) + std::pow(point.y, 2) + 12; };
	rFuncs[2] = [](const sf::Vector2f &point) -> float { return std::pow(point.x + 4, 2) + std::pow(point.y, 2) + 18; };
	rFuncs[3] = [](const sf::Vector2f &point) -> float { return std::pow(point.x - 3, 2) + std::pow(point.y + 3, 2) + 3; };
	rFuncs[4] = [](const sf::Vector2f &point) -> float { return std::pow(point.x + 5, 2) + std::pow(point.y, 2) + 27; };
	rFuncs[5] = [](const sf::Vector2f &point) -> float { return std::pow(point.x + 45, 2) + std::pow(point.y, 2) + 27; };


	float res1 = ROr(rFuncs[0](point), rFuncs[1](point));
	float res2 = ROr(res1, rFuncs[2](point));
	float res3 = ROr(res2, rFuncs[3](point));
	float res4 = RAnd(rFuncs[4](point), rFuncs[5](point));
	float res = ROr(res3, res4);
	return res;
}
// Определение функции для интерполяции цветов
sf::Color interpolateColors(const sf::Color& colorFirst, const sf::Color& colorSec, float k)
{
	sf::Color val;
	val.r = static_cast<sf::Uint8>(colorFirst.r + (colorSec.r - colorFirst.r) * k);
	val.g = static_cast<sf::Uint8>(colorFirst.g + (colorSec.g - colorFirst.g) * k);
	val.b = static_cast<sf::Uint8>(colorFirst.b + (colorSec.b - colorFirst.b) * k);
	val.a = static_cast<sf::Uint8>(colorFirst.a + (colorSec.a - colorFirst.a) * k);

	return val;
}

// Класс для отображения спрайтов с использованием функций векторного поля
class RFuncSprites : public sf::Sprite
{
public:
	void create(const sf::Vector2u& size, const int getElem)
	{
		_image.create(size.x, size.y, sf::Color::Cyan);
		_texture.loadFromImage(_image);
		setTexture(_texture);
		_firstColor = sf::Color::Black;
		_secondColor = sf::Color::White;
		_getNorm = getElem;
	}
	void DrawRFunc(const std::function<float(const sf::Vector2f&)>& rfunc, const sf::FloatRect& subSpace)
	{
		sf::Vector2f spaceStep = { subSpace.width / static_cast<float>(_image.getSize().x),
								  subSpace.height / static_cast<float>(_image.getSize().y) };

		for (unsigned int x = 0; x < _image.getSize().x; ++x)
		{
			for (unsigned int y = 0; y < _image.getSize().y; ++y)
			{
				sf::Vector2f spacePoint1 = { subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y };

				sf::Vector2f spacePoint2 = { subSpace.left + static_cast<float>(x + 1) * spaceStep.x,
											subSpace.top + static_cast<float>(y) * spaceStep.y };
				sf::Vector2f spacePoint3 = { subSpace.left + static_cast<float>(x) * spaceStep.x,
											subSpace.top + static_cast<float>(y + 1) * spaceStep.y };
				const float z2 = rfunc(spacePoint2);
				const float z1 = rfunc(spacePoint1);
				const float z3 = rfunc(spacePoint3);
				const float Av = calculateDeterminant({
					{spacePoint1.y, z1, 1},
					{spacePoint2.y, z2, 1},
					{spacePoint3.y, z3, 1},
					});
				const float Bv = calculateDeterminant({
					{spacePoint1.x, z1, 1},
					{spacePoint2.x, z2, 1},
					{spacePoint3.x, z3, 1},
					});

				const float Cv = calculateDeterminant({
					{spacePoint1.x, spacePoint1.y, 1},
					{spacePoint2.x, spacePoint2.y, 1},
					{spacePoint3.x, spacePoint3.y, 1},
					});
				const float Dv = calculateDeterminant({
					{spacePoint1.x, spacePoint1.y, z1},
					{spacePoint2.x, spacePoint2.y, z2},
					{spacePoint3.x, spacePoint3.y, z3},
					});
				const float det = std::sqrt(Av * Av + Bv * Bv + Cv * Cv + Dv * Dv);
				float nx = Av / det;
				float ny = Bv / det;
				float nz = Cv / det;
				float nw = Dv / det;
				float getNorm = nx;
				switch (_getNorm)
				{
				case 0:
					break;
				case 1:
					getNorm = ny;
					break;
				case 2:
					getNorm = nz;
					break;
				case 3:
					getNorm = nw;
					break;
				}
				auto pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + getNorm) / 2);
				_image.setPixel(x, y, pixelColor);
			}
		}
		_texture.loadFromImage(_image);
	}
	float calculateDeterminant(const std::vector<std::vector<float>>& matrix)
	{
		return matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
			matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
			matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);
	}
	void saveToFile(const std::string &filename) { _image.saveToFile(filename); }
private:
	sf::Color _firstColor;
	sf::Color _secondColor;
	sf::Texture _texture;
	sf::Image _image;
	int _getNorm;
};


int main()
{
	sf::RenderWindow window(sf::VideoMode(400, 400), "Vakhaev A.R. lab 2");
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "ImGui initialization failed\n";
		return -1;
	}
	sf::Vector2u spriteSize = sf::Vector2u{window.getSize().x / 2, window.getSize().y / 2};
	RFuncSprites RFuncSpritesNx;
	RFuncSpritesNx.create(spriteSize, 0);
	RFuncSprites RFuncSpritesNy;
	RFuncSpritesNy.create(spriteSize, 1);
	RFuncSpritesNy.setPosition(spriteSize.x, 0);
	RFuncSprites RFuncSpritesNz;
	RFuncSpritesNz.create(spriteSize, 2);
	RFuncSpritesNz.setPosition(0, spriteSize.y);
	RFuncSprites RFuncSpritesNw;
	RFuncSpritesNw.create(spriteSize, 3);
	RFuncSpritesNw.setPosition(spriteSize.x, spriteSize.y);
	sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);
	RFuncSprites arr[4] = { RFuncSpritesNx, RFuncSpritesNy, RFuncSpritesNz, RFuncSpritesNw };
	RFuncSpritesNx.DrawRFunc(&figure, subSpace);
	RFuncSpritesNy.DrawRFunc(&figure, subSpace);
	RFuncSpritesNz.DrawRFunc(&figure, subSpace);
	RFuncSpritesNw.DrawRFunc(&figure, subSpace);
	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
		}
		ImGui::SFML::Update(window, deltaClock.restart());
		ImGui::Begin("Menu");
		ImVec4 firstColor = ImVec4(0, 0, 0, 1);
		ImVec4 secondColor = ImVec4(1, 1, 1, 1);
		auto sfFirstColor = sf::Color(static_cast<sf::Uint8>(firstColor.x * 255), static_cast<sf::Uint8>(firstColor.y * 255),
			static_cast<sf::Uint8>(firstColor.z * 255), static_cast<sf::Uint8>(firstColor.w * 255));
		auto sfSecondColor =
			sf::Color(static_cast<sf::Uint8>(secondColor.x * 255), static_cast<sf::Uint8>(secondColor.y * 255),
				static_cast<sf::Uint8>(secondColor.z * 255), static_cast<sf::Uint8>(secondColor.w * 255));
		if (ImGui::Button("Save Images"))
		{
			RFuncSpritesNx.saveToFile("Nx.png");
			RFuncSpritesNy.saveToFile("Ny.png");
			RFuncSpritesNz.saveToFile("Nz.png");
			RFuncSpritesNw.saveToFile("Nw.png");
		}
		ImGui::End();
		window.clear();
		window.draw(RFuncSpritesNx);
		window.draw(RFuncSpritesNy);
		window.draw(RFuncSpritesNz);
		window.draw(RFuncSpritesNw);
		ImGui::SFML::Render(window);
		window.display();
	}
	ImGui::SFML::Shutdown();
	return 0;
}