#pragma once

#include "globals.h"

// �������������� ��� ���� � ������ � ������ ����� ��� �������, ����������� ������ �������� ������������ ������. 
// �������� ���������� ������������ �������� ������ final, ����� ������� �������� �������� �������.
class myBall final : public sf::CircleShape
{
    // ��������� �������-��������� ��������.
    template <class T>
    friend void processCollision(std::vector<T>&& vec);
  
public:
    // ������������ �����������
    myBall() : CircleShape()
    {
        // ������ ��� �������������/������������ �������� � �����������.
        dir.x = (-5 + (rand() % 10)) / 3.;
        dir.y = (-5 + (rand() % 10)) / 3.;
        //r = 5 + rand() % 5;
        r = 5 + rand() % 5;
        speed = 30 + rand() % 30;
        // ������ �������, ����� ��������� ���, ����� ������ ���������� ����� �� ������/������ ������ ����.
        pos.x = rand() % (WINDOW_X - (int)r * 2 - 1);
        pos.y = rand() % (WINDOW_Y - (int)r * 2 - 1);
        this->setRadius(r);
        // ���������� ������ ���� �����������.
        this->setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
    }

    void draw(sf::RenderWindow& window)
    {
        this->setPosition(pos.x, pos.y);
        window.draw(*this);
    }

    void move(const float& deltaTime)
    {
        // ������ �������� ������ ���������� � ��������� ��� ���� � ���� �������� ��� ������ ����������.
        pos.x += dir.x * speed * deltaTime;
        pos.y += dir.y * speed * deltaTime;
    }

    // ����� �������, ��� ������������ ����� �������� ��� ������ ����� ������������ ������, �������������.
protected:
    sf::Vector2f pos;
    sf::Vector2f dir;
    float r = 0;
    float speed = 0;
};

template <class T>
void processCollision(std::vector<T>&& vec) // �������� ��������� ����������� ��� �������� �������.
{
    // ������� ������� �����������, �������, ��� �������� � ������� � ��� ����� �� ������ ��� �������.
    if constexpr (std::is_same_v < std::vector<T>, std::vector<myBall>>)
    {
        // Sweep and Prune �����, �������. ������� � ������ ��������� �� ���������� ����� ��� � ����������� ���.
        // ������������ ����� ������ �������, ������� ���������� ������ ���� � ����� �� �����������.
        // ���� ������ ��������� ������ ���������� �������� �� ��������, ����� �� �� ��� ���� ������ �������, �
        // ���� ��� ���, ��� �������� �������� ����� �����������.
        std::sort(vec.begin(), vec.end(),
            [](const myBall& a, const myBall& b) -> bool
            {
                return a.pos.x > b.pos.x;
            });

        auto vecSize = vec.size();

        for (int i = 0; i < vecSize - OFFSET; i++)
        {
            // �������� �� ��������. 

            for (int j = i; j < (i + OFFSET); j++)
            {
                // ���������, ���� �� ����������� � ��������� �������� �� ��� � ���������� OFFSET
                if ((vec[j].pos.x + vec[j].r) > (vec[j + 1].pos.x - vec[j + 1].r))
                {

                    // ���� ���������� ����� �������� ����������� � ���������� ����������� ������ �����
                    // ��������, �� ������������.
                    double range = sqrt(pow(vec[i].pos.x - vec[i + 1].pos.x, 2) + pow(vec[i].pos.y - vec[i + 1].pos.y, 2));

                    // ����������� ��������� ������� � ������ ���������� �������� ������� ���������� ������.
                    // ��-��������, ���������� ��������� �� �������� ��� �� ����� ������, ��� �������� ���� � �����.
                    if ((range) <= (vec[i].r + vec[i + 1].r))
                    {
                        // ������ �� ��������� ��������� ����������, �������������� ��� ������� ������.

                        float nx = (vec[i + 1].pos.x - vec[i].pos.x) / range;
                        float ny = (vec[i + 1].pos.y - vec[i].pos.y) / range;

                        float tx = -ny;
                        float ty = nx;

                        float dpTan1 = vec[i].dir.x * tx + vec[i].dir.y * ty;
                        float dpTan2 = vec[i + 1].dir.x * tx + vec[i + 1].dir.y * ty;

                        float dpNorm1 = vec[i].dir.x * nx + vec[i].dir.y * ny;
                        float dpNorm2 = vec[i + 1].dir.x * nx + vec[i + 1].dir.y * ny;

                        // ������ ����� ������� ��� ������ �� ������� ������.
                        float m1 = (dpNorm1 * (vec[i].r - vec[i + 1].r) + 2.0f * vec[i + 1].r * dpNorm2) / (vec[i].r + vec[i + 1].r);
                        float m2 = (dpNorm2 * (vec[i + 1].r - vec[i].r) + 2.0f * vec[i].r * dpNorm1) / (vec[i].r + vec[i + 1].r);

                        // �������� �������� ���������.
                        vec[i].dir.x = tx * dpTan1 + nx * m1;
                        vec[i].dir.y = ty * dpTan1 + ny * m1;
                        vec[i + 1].dir.x = tx * dpTan2 + nx * m2;
                        vec[i + 1].dir.y = ty * dpTan2 + ny * m2;

                        // ����� ������ �������� ��� ��������. ������ �����, ����� ����� �������� �����.
                        vec[i].setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
                        vec[i + 1].setFillColor(sf::Color(rand() % 256, rand() % 256, rand() % 256, 255));
                    }
                }
            }          
        }

        // ����������� ������������� ������ ���� � �������������� ������� ��� ������������� ��������� �������� �������.
        for (auto& obj : vec)
        {
            if (obj.pos.x < 0)
                obj.dir.x = -obj.dir.x;

            if (obj.pos.y < 0)
                obj.dir.y = -obj.dir.y;

            // ��������� ������ ������, ��� � ��������� ������ ����������� �� ������ ����.
            if (obj.pos.x > WINDOW_X - obj.r * 2)
                obj.dir.x = -obj.dir.x;

            if (obj.pos.y > WINDOW_Y - obj.r * 2)
                obj.dir.y = -obj.dir.y;
        }
    }
}