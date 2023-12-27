#include <SFML/Graphics.hpp>
#include "imgui.h"
#include "imgui-SFML.h"
#include <cmath>

void RenderGui(bool& linearGradient, bool& radialGradient)
{
    ImGui::Begin("Gradient Options");

    if (ImGui::Button("Linear Gradient Fill"))
    {
        linearGradient = true;
        radialGradient = false;
    }

    if (ImGui::Button("Radial Gradient Fill"))
    {
        radialGradient = true;
        linearGradient = false;
    }

    ImGui::End();
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Vakhaev A.R. IDB-20-11");
    window.setFramerateLimit(60);
    ImGui::SFML::Init(window);

    bool linearGradient = false;
    bool radialGradient = false;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            if (event.type == sf::Event::Closed)
                window.close();
        }

        ImGui::SFML::Update(window, sf::seconds(1.0f / 60.0f));

        window.clear(sf::Color::White);

        RenderGui(linearGradient, radialGradient);

        if (linearGradient)
        {
            sf::VertexArray vertices(sf::TrianglesStrip, 4);
            vertices[0].position = sf::Vector2f(0, 0);
            vertices[1].position = sf::Vector2f(window.getSize().x, 0);
            vertices[2].position = sf::Vector2f(0, window.getSize().y);
            vertices[3].position = sf::Vector2f(window.getSize().x, window.getSize().y);

            vertices[0].color = sf::Color::Red;
            vertices[1].color = sf::Color::Green;
            vertices[2].color = sf::Color::Blue;
            vertices[3].color = sf::Color::Yellow;

            window.draw(vertices);
        }
        else if (radialGradient)
        {
            sf::RenderTexture renderTexture;
            renderTexture.create(window.getSize().x, window.getSize().y);
            renderTexture.clear(sf::Color::Transparent);

            float maxRadius = std::sqrt(window.getSize().x * window.getSize().x + window.getSize().y * window.getSize().y) / 2.f;

            for (unsigned int y = 0; y < window.getSize().y; ++y)
            {
                for (unsigned int x = 0; x < window.getSize().x; ++x)
                {
                    float distance = std::sqrt((x - window.getSize().x / 2.f) * (x - window.getSize().x / 2.f) +
                        (y - window.getSize().y / 2.f) * (y - window.getSize().y / 2.f));

                    float ratio = distance / maxRadius;
                    sf::Uint8 alpha = static_cast<sf::Uint8>((1.f - ratio) * 255);

                    sf::RectangleShape pixel(sf::Vector2f(1.f, 1.f));
                    pixel.setPosition(static_cast<float>(x), static_cast<float>(y));
                    pixel.setFillColor(sf::Color(255, 255, 255, alpha));
                    renderTexture.draw(pixel);
                }
            }

            renderTexture.display();
            sf::Sprite sprite(renderTexture.getTexture());
            window.draw(sprite);
        }

        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}