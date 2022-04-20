#include "SFML/Graphics.hpp"
#include "MiddleAverageFilter.h"

constexpr int WINDOW_X  = 1024;
constexpr int WINDOW_Y  = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 100;

Math::MiddleAverageFilter<float,100> fpscounter;

// Сконкатенируем все поля и методы в единый класс для порядка, унаследовав заодно свойства стандартного кружка. 
// Запретим дальнейшее наследование ключевым словом final, будем считать конечным дочерним классом.
class myBall final : public sf::CircleShape
{
// Зафрендим функцию-обрабочик коллизий.
    template <class T>
    friend void processCollision(std::vector<T>& vec);
// Будем считать, что пользователь будет получать эти данные через интерфейсные методы, инкапсулируем.
protected:
    sf::Vector2f p;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;
public:
    // Делигирующий конструктор
    myBall() : CircleShape()
    {
        // Зашьем всю инициализацию/рандомизацию прямиком в конструктор.
        dir.x = (-5 + (rand() % 10)) / 3.;
        dir.y = (-5 + (rand() % 10)) / 3.;
        r = 5 + rand() % 5;
        speed = 30 + rand() % 30;
        // Вычтем диаметр, чтобы исключить баг, когда кружки появляются прямо на нижней/правой кромке окна.
        p.x = rand() % (WINDOW_X - (int)r * 2 - 1);
        p.y = rand() % (WINDOW_Y - (int)r * 2 - 1);
        this->setRadius(r);
        // Нарандомим цветов ради развлечения.
        this->setFillColor(sf::Color( rand() % 256, rand() % 256, rand() % 256, 255) );
    }

    void draw(sf::RenderWindow& window)
    {
        this->setPosition(p.x, p.y);
        window.draw(*this);
    }

    void move(float deltaTime)
    {
        // Уберем создание лишних переменных и объединим все дело в одну операцию для каждой координаты.
        p.x += dir.x * speed * deltaTime;
        p.y += dir.y * speed * deltaTime;
    }
};

template <class T>
void processCollision(std::vector<T> &vec)
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
                return a.p.x > b.p.x;
            });

        // Создадим необходимые переменные для операций.
        auto   vecSize = vec.size();
        int    collision;
        double range;
        float  nx;
        float  ny;
        float  tx;
        float  ty;
        float  dpTan1;
        float  dpTan2;
        float  dpNorm1;
        float  dpNorm2;
        float  m1;
        float  m2;

        for (int i = 0; i < vecSize - 1; i++)
        {
            // Проверка на коллизии. Если расстояние между центрами окружностей в Декартовых координатах меньше суммы
            // радиусов, то столкновение.
            range = sqrt(pow(vec[i].p.x - vec[i + 1].p.x, 2) + pow(vec[i].p.y - vec[i + 1].p.y, 2));

            // Посредством векторной алгебры и физики необходимо изменить вектора траектории полета.
            // По-хорошему, необходимо проверять на коллизии еще на этапе спауна, ибо багаются друг в друге.
            if ((range) <= (vec[i].r + vec[i + 1].r))
            {
                // Взятая из интернета векторная арифметика, адаптированная под решение задачи.

                nx = (vec[i + 1].p.x - vec[i].p.x) / range;
                ny = (vec[i + 1].p.y - vec[i].p.y) / range;

                tx = -ny;
                ty = nx;

                dpTan1 = vec[i].dir.x * tx + vec[i].dir.y * ty;
                dpTan2 = vec[i + 1].dir.x * tx + vec[i + 1].dir.y * ty;

                dpNorm1 = vec[i].dir.x * nx + vec[i].dir.y * ny;
                dpNorm2 = vec[i + 1].dir.x * nx + vec[i + 1].dir.y * ny;

                // Вместо массы воткнем тут радиус по условию задачи.
                m1 = (dpNorm1 * (vec[i].r - vec[i + 1].r) + 2.0f * vec[i + 1].r * dpNorm2) / (vec[i].r + vec[i + 1].r);
                m2 = (dpNorm2 * (vec[i + 1].r - vec[i].r) + 2.0f * vec[i].r * dpNorm1) / (vec[i].r + vec[i + 1].r);

                // Итоговый пересчет скоростей.
                vec[i].dir.x = tx * dpTan1 + nx * m1;
                vec[i].dir.y = ty * dpTan1 + ny * m1;
                vec[i + 1].dir.x = tx * dpTan2 + nx * m2;
                vec[i + 1].dir.y = ty * dpTan2 + ny * m2;

                // Смена цветов объектов при коллизии. Заодно видно, когда метод работает криво.
                vec[i].setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
                vec[i + 1].setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
            }
        }

        // Программное представление кромок окна и отзеркаливание вектора для моделирования абсолютно упругого отскока.
        for (auto& obj : vec)
        {
            if (obj.p.x < 0)
                obj.dir.x = -obj.dir.x;

            if (obj.p.y < 0)
                obj.dir.y = -obj.dir.y;

            // Учитываем радиус шарика, ибо в противном случае заскакивает за кромку окна.
            if (obj.p.x > WINDOW_X - obj.r * 2)
                obj.dir.x = -obj.dir.x;

            if (obj.p.y > WINDOW_Y - obj.r * 2)
                obj.dir.y = -obj.dir.y;
        }
    }
}

void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");

    srand(time(NULL));
    
    // Генерация рандомного объема кружков.
    std::vector<myBall> balls;

    for (int i = 0; i < (rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS); i++)
    {
        myBall newBall;
        balls.push_back(newBall);
    }

   // window.setFramerateLimit(60);

    sf::Clock clock;
    float lastime = clock.restart().asSeconds();

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

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;
       
        window.clear();

        /// <summary>
        /// TODO: PLACE COLLISION CODE HERE 
        /// объекты создаются в случайном месте на плоскости со случайным вектором скорости, имеют радиус R
        /// Объекты движутся кинетически. Пространство ограниченно границами окна
        /// Напишите обработчик столкновений шаров между собой и краями окна. Как это сделать эффективно?
        /// Массы пропорцианальны площадям кругов, описывающих объекты 
        /// Как можно было-бы улучшить текущую архитектуру кода?
        /// Данный код является макетом, вы можете его модифицировать по своему усмотрению

        processCollision(balls);

        for (auto &ball : balls)
        {
            ball.move(deltaTime);

            ball.draw(window);
        }

        // Раскомментим, чтобы засветить частоту.
		draw_fps(window, fpscounter.getAverage());

		window.display();
    }
    return 0;
}
