#include <SFML/Graphics.hpp>
#include <functional>
#include <iostream>
#include <vector>
#include "imgui-SFML.h"
#include "imgui.h"
float RAnd(float w1, float w2) { return w1 + w2 + std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }
float ROr(float w1, float w2) { return w1 + w2 - std::sqrt((w1 * w1 + w2 * w2) - 2 * w1 * w2); }
std::vector<sf::VertexArray> gradientPaths;
float** NxMatrix;
float** NyMatrix;
enum Shapes
{
	Nx,
	Ny,
	Nz,
	Nw
};
float figure(const sf::Vector2f& point)
{
	std::function<float(const sf::Vector2f&)> rFuncs[6];
	rFuncs[0] = [](const sf::Vector2f& point) -> float { return std::pow(point.x, 2) + std::pow(point.y - 12, 2) + 12; };
	rFuncs[1] = [](const sf::Vector2f& point) -> float { return std::pow(point.x - 15, 2) + std::pow(point.y, 2) + 12; };
	rFuncs[2] = [](const sf::Vector2f& point) -> float { return std::pow(point.x + 4, 2) + std::pow(point.y, 2) + 18; };
	rFuncs[3] = [](const sf::Vector2f& point) -> float { return std::pow(point.x - 3, 2) + std::pow(point.y + 3, 2) + 3; };
	rFuncs[4] = [](const sf::Vector2f& point) -> float { return std::pow(point.x + 5, 2) + std::pow(point.y, 2) + 27; };
	rFuncs[5] = [](const sf::Vector2f& point) -> float { return std::pow(point.x - 45, 2) + std::pow(point.y, 2) + 27; };
	float res1 = ROr(rFuncs[0](point), rFuncs[1](point));
	float res2 = ROr(res1, rFuncs[2](point));
	float res3 = ROr(res2, rFuncs[3](point));
	float res4 = RAnd(rFuncs[4](point), rFuncs[5](point));
	float res = ROr(res3, res4);
	return res;
}
sf::Color interpolateColors(const sf::Color& colorFirst, const sf::Color& colorSec, float k)
{
	sf::Color val;
	val.r = static_cast<sf::Uint8>(colorFirst.r + (colorSec.r - colorFirst.r) * k);
	val.g = static_cast<sf::Uint8>(colorFirst.g + (colorSec.g - colorFirst.g) * k);
	val.b = static_cast<sf::Uint8>(colorFirst.b + (colorSec.b - colorFirst.b) * k);
	val.a = static_cast<sf::Uint8>(colorFirst.a + (colorSec.a - colorFirst.a) * k);

	return val;
}

class RFuncSprites : public sf::Sprite
{
public:
	const sf::Texture& getTexture() const { return _texture; }
	void create(const sf::Vector2u& size, const int getElem, Shapes shapeType)
	{
		_image.create(size.x, size.y, sf::Color::Cyan);
		_texture.loadFromImage(_image);
		setTexture(_texture);
		_firstColor = sf::Color::Black;
		_secondColor = sf::Color::White;
		_getNorm = getElem;
		_shapeType = shapeType;
	}
	sf::Color getPixelColor(float Av, float Bv, float Cv, float Dv, unsigned x, unsigned y)
	{
		float nx, ny, nz, nw;

		const float det = std::sqrt(Av * Av + Bv * Bv + Cv * Cv + Dv * Dv);
		nx = Av / det;
		ny = Bv / det;
		nz = Cv / det;
		nw = Dv / det;

		NxMatrix[y][x] = nx;
		NyMatrix[y][x] = ny;

		float shapeValue;

		switch (_shapeType)
		{
		case Nx:
			shapeValue = nx;
			break;
		case Ny:
			shapeValue = ny;
			break;
		case Nz:
			shapeValue = nz;
			break;
		case Nw:
			shapeValue = nw;
			break;
		default:
			throw std::runtime_error("Wrong shape");
		}

		return interpolateColors(_firstColor, _secondColor, (1.f + shapeValue) / 2);
	}
	void DrawRFunc(const std::function<float(const sf::Vector2f&)>& rfunc, const sf::FloatRect& subSpace)
	{
		NxMatrix = new float* [_image.getSize().y];
		NyMatrix = new float* [_image.getSize().y];
		for (int i = 0; i < _image.getSize().y; ++i)
		{
			NxMatrix[i] = new float[_image.getSize().x];
			NyMatrix[i] = new float[_image.getSize().x];
		}
		const sf::Texture& texture = getTexture();
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
				_image.setPixel(x, y, getPixelColor(Av, Bv, Cv, Dv, x, y));
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
	void saveToFile(const std::string& filename) { _image.saveToFile(filename); }
	sf::Vector2f getGradient(const std::function<float(const sf::Vector2f&)>& rfunc, const sf::Vector2f& point)
	{
		unsigned x = point.x;
		unsigned y = point.y;

		return sf::Vector2f(NxMatrix[y][x], NyMatrix[y][x]);
	}
	sf::VertexArray GradientDescent(const std::function<float(const sf::Vector2f&)>& rfunc, const sf::Vector2f& startPoint, float stepSize = 10)
	{
		sf::VertexArray descentPath(sf::LineStrip);
		std::vector<sf::Vertex> vertexList;
		auto vertex = sf::Vertex(startPoint, sf::Color::Blue);
		descentPath.append(vertex);
		vertexList.push_back(vertex);
		sf::Vector2f currentPoint = startPoint;
		while (true)
		{
			sf::Vector2f gradient = getGradient(rfunc, currentPoint);
			currentPoint += stepSize * gradient;
			if (currentPoint.x >= _image.getSize().x || currentPoint.y >= _image.getSize().y || currentPoint.x <= 0 ||
				currentPoint.y <= 0)
			{
				return descentPath;
			}
			for (int i = 0; i < vertexList.size(); i++)
			{
				unsigned x1 = currentPoint.x;
				unsigned y1 = currentPoint.y;
				unsigned x2 = vertexList[i].position.x;
				unsigned y2 = vertexList[i].position.y;
				if (x1 == x2 && y1 == y2)
				{
					return descentPath;
				}
			}
			auto tempVertex = sf::Vertex(currentPoint, sf::Color::Blue);
			descentPath.append(tempVertex);
			vertexList.push_back(tempVertex);
		}
		return descentPath;
	}
	void ClearGradients()
	{
		gradientPaths.clear();
	}
private:
	Shapes _shapeType;
	sf::Color _firstColor;
	sf::Color _secondColor;
	sf::Texture _texture;
	sf::Image _image;
	int _getNorm;
};
void HandleUserInput(sf::RenderWindow& window, const sf::Event& event, RFuncSprites& rFunc)
{
	switch (event.type)
	{
	case sf::Event::Closed:
		window.close();
		break;
	case sf::Event::MouseButtonPressed:
	{
		sf::Vector2f mouseClickPoint;
		sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
		mouseClickPoint = window.mapPixelToCoords(mousePosition);

		sf::VertexArray descentPathNx =
			rFunc.GradientDescent([](const sf::Vector2f& point) -> float { return figure(point); }, mouseClickPoint);

		gradientPaths.push_back(descentPathNx);
		break;
	}
	default:
		break;
	}
}
void renderGui(sf::RenderWindow& window, RFuncSprites& rFuncSpriteNx)
{
	ImGui::Begin("Settings");
	if (ImGui::Button("Save nx"))
	{
		rFuncSpriteNx.saveToFile("Nx.png");
	}
	if (ImGui::Button("Clear image"))
	{
		rFuncSpriteNx.ClearGradients();
	}
	ImGui::End();
}
void drawAllSprites(sf::RenderWindow& window, RFuncSprites arr[], int arrSize)
{
	window.clear();
	for (int i = 0; i < arrSize; i++)
	{
		window.draw(arr[i]);
	}
}

int main()
{
	const int width = 400;
	const int height = 400;
	sf::RenderWindow window(sf::VideoMode(width, height), "Vakhaev A.R. lab 3");
	window.setFramerateLimit(60);
	if (!ImGui::SFML::Init(window))
	{
		std::cout << "ImGui initialization failed\n";
		return -1;
	}
	sf::Vector2u spriteSize = window.getSize();
	RFuncSprites RFuncSpritesNx;
	RFuncSpritesNx.create(spriteSize, 0, Shapes::Nx);
	sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);
	RFuncSprites arr[1] = { RFuncSpritesNx};
	RFuncSpritesNx.DrawRFunc(&figure, subSpace);
	sf::Clock deltaClock;
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(window, event);
			HandleUserInput(window, event, RFuncSpritesNx);
		}
		ImGui::SFML::Update(window, deltaClock.restart());
		renderGui(window, RFuncSpritesNx);
		drawAllSprites(window, arr, 1);
		for (const auto& gradientPath : gradientPaths)
		{
			window.draw(gradientPath);
		}
		ImGui::SFML::Render(window);
		window.display();
	}
	ImGui::SFML::Shutdown();
	return 0;
}