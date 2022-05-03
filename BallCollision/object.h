#pragma once

#include "globals.h"

// Сконкатенируем все поля и методы в единый класс для порядка, унаследовав заодно свойства стандартного кружка. 
// Запретим дальнейшее наследование ключевым словом final, будем считать конечным дочерним классом.
class myBall final : public sf::CircleShape
{
    // Зафрендим функцию-обрабочик коллизий.
    template <class T>
    friend void processCollision(std::vector<T>&& vec);
  
public:
    // Делигирующий конструктор
    myBall() : CircleShape()
    {
        // Зашьем всю инициализацию/рандомизацию прямиком в конструктор.
        dir.x = (-5 + (rand() % 10)) / 3.;
        dir.y = (-5 + (rand() % 10)) / 3.;
        //r = 5 + rand() % 5;
        r = 5 + rand() % 5;
        speed = 30 + rand() % 30;
        // Вычтем диаметр, чтобы исключить баг, когда кружки появляются прямо на нижней/правой кромке окна.
        pos.x = rand() % (WINDOW_X - (int)r * 2 - 1);
        pos.y = rand() % (WINDOW_Y - (int)r * 2 - 1);
        this->setRadius(r);
        this->setFillColor(sf::Color::Blue);
    }

    void draw(sf::RenderWindow& window)
    {
        this->setPosition(pos.x, pos.y);
        window.draw(*this);
    }

    void move(const float& deltaTime)
    {
        // Уберем создание лишних переменных и объединим все дело в одну операцию для каждой координаты.
        pos.x += dir.x * speed * deltaTime;
        pos.y += dir.y * speed * deltaTime;
    }

    // Будем считать, что пользователь будет получать эти данные через интерфейсные методы, инкапсулируем.
protected:
    sf::Vector2f pos;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;
    bool isCollided = 0;
};

template <class T>
void processCollision(std::vector<T>&& vec) // Применим семантику перемещения для экономии ресурса.
{
    // Сделаем функцию полиморфной, полагая, что коллизии в будущем у нас будут не только для кружков.
    if constexpr (std::is_same_v < std::vector<T>, std::vector<myBall>>)
    {
        // Sweep and Prune метод, пробуем. Засунем в лямбду сравнение по координате одной оси и отсортируем все.
        // Сталкиваться могут только объекты, которые достаточно близки друг к другу по координатам.
        // Этот способ позволяет сузить количество проверок на коллизию, делая ее не для всех подряд кружков, а
        // лишь для тех, что наиболее вероятно могут столкнуться.
        std::sort(vec.begin(), vec.end(),
            [](const myBall& a, const myBall& b) -> bool
            {
                return a.pos.x > b.pos.x;
            });

        auto vecSize = vec.size();

        for (int i = 0; i < vecSize - OFFSET; i++)
        {
            if (vec[i].isCollided)
            {
                vec[i].isCollided = false;
                continue;
            }
            // Проверка на коллизии. 
            for (int j = i + 1; j < (i + OFFSET); j++)
            {
                if (vec[j].isCollided)
                {
                    vec[j].isCollided = false;
                    continue;
                }

                // Проверяем, есть ли пересечение у следующих отрезков по оси в количестве OFFSET
                if ((vec[i].pos.x + vec[i].r) > (vec[j].pos.x - vec[j].r))
                {
                    // Если расстояние между центрами окружностей в декартовых координатах меньше суммы
                    // радиусов, то столкновение.
                    double range = sqrt(pow(vec[i].pos.x - vec[j].pos.x, 2) + pow(vec[i].pos.y - vec[j].pos.y, 2));

                    // Посредством векторной алгебры и физики необходимо изменить вектора траектории полета.
                    // По-хорошему, необходимо проверять на коллизии еще на этапе спауна, ибо багаются друг в друге.
                    if (range <= (vec[i].r + vec[j].r))
                    {
                        // Взятая из интернета векторная арифметика, адаптированная под решение задачи.

                        float nx = (vec[j].pos.x - vec[i].pos.x) / range;
                        float ny = (vec[j].pos.y - vec[i].pos.y) / range;

                        float tx = -ny;
                        float ty = nx;

                        float dpTan1 = vec[i].dir.x * tx + vec[i].dir.y * ty;
                        float dpTan2 = vec[j].dir.x * tx + vec[j].dir.y * ty;

                        float dpNorm1 = vec[i].dir.x * nx + vec[i].dir.y * ny;
                        float dpNorm2 = vec[j].dir.x * nx + vec[j].dir.y * ny;

                        // Вместо массы воткнем тут радиус по условию задачи.
                        float m1 = (dpNorm1 * (vec[i].r - vec[j].r) + 2.0f * vec[j].r * dpNorm2) / (vec[i].r + vec[j].r);
                        float m2 = (dpNorm2 * (vec[j].r - vec[i].r) + 2.0f * vec[i].r * dpNorm1) / (vec[i].r + vec[j].r);

                        // Итоговый пересчет скоростей.
                        vec[i].dir.x = tx * dpTan1 + nx * m1;
                        vec[i].dir.y = ty * dpTan1 + ny * m1;
                        vec[j].dir.x = tx * dpTan2 + nx * m2;
                        vec[j].dir.y = ty * dpTan2 + ny * m2;

                        // Смена цветов объектов при коллизии. Заодно видно, когда метод работает криво.
                        (vec[i].getFillColor() == sf::Color::Red) ?
                            vec[i].setFillColor(sf::Color::Blue) : vec[i].setFillColor(sf::Color::Red);
                        (vec[j].getFillColor() == sf::Color::Red) ?
                            vec[j].setFillColor(sf::Color::Blue) : vec[j].setFillColor(sf::Color::Red);    

                        vec[i].isCollided = true;
                        vec[j].isCollided = true;
                    }
                }
            }          
        }

        // Программное представление кромок окна и отзеркаливание вектора для моделирования абсолютно упругого отскока.
        for (auto& obj : vec)
        {
            if (obj.pos.x < 0)
                obj.dir.x = -obj.dir.x;

            if (obj.pos.y < 0)
                obj.dir.y = -obj.dir.y;

            // Учитываем радиус шарика, ибо в противном случае заскакивает за кромку окна.
            if (obj.pos.x > WINDOW_X - obj.r * 2)
                obj.dir.x = -obj.dir.x;

            if (obj.pos.y > WINDOW_Y - obj.r * 2)
                obj.dir.y = -obj.dir.y;
        }
    }
}