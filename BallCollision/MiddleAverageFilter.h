#pragma once

namespace Math
{
    template<typename T, unsigned size>
    class MiddleAverageFilter
    {
        T data[size]{};

        sf::Clock clock{};

        unsigned id           = 0u;
        float    lastime      = 0.f;
        float    current_time = 0.f;
        
    public:
        void init()
        {
            lastime = clock.restart().asSeconds();
        }

        void update()
        {
            current_time = clock.getElapsedTime().asSeconds();
            deltaTime    = current_time - lastime;

            data[id] = 1.0f / (current_time - lastime);
            id = (id + 1) % size;

            lastime = current_time;
        }

        T getAverage() const
        {
            T sum = 0;
            for (auto& i : data) sum += i;
            return sum / size;
        }

        void draw_fps(sf::RenderWindow& window)
        {
            char c[32];
            snprintf(c, 32, "FPS: %f", getAverage());
            sf::String  str(c);
            window.setTitle(str);
        }

        float deltaTime = 0.f;
    };
}


