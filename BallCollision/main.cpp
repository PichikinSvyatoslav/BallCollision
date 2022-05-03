#include "SFML/Graphics.hpp"
#include "object.h"
#include "globals.h"
#include "MiddleAverageFilter.h"

// Попробуем передать этому объекту передать по смыслу весь функционал,
// отвечающий за частоту кадров, в надежде улучшить структуру кода и его читаемость.
Math::MiddleAverageFilter<float,100u> fpscounter;

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    window.setFramerateLimit(120);

    srand(time(NULL));
    
    // Генерация рандомного объема кружков.
    std::vector<myBall> balls;
    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    {
        myBall newBall;
        balls.push_back(newBall);
    } 
   
    fpscounter.init();

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }
        
        fpscounter.update(); 
       
        window.clear();

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению

        processCollision(std::move(balls));

        for (auto &ball : balls)
        {
            ball.move(fpscounter.deltaTime);

            ball.draw(window);
        }

        // draw_fps стала частью fpscounter
        fpscounter.draw_fps(window);

		window.display();
    }
    return 0;
}
