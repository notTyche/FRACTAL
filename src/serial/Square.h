//
// Created by not_tyche on 30/06/20.
//

#ifndef FRACTAL_SQUARE_H
#define FRACTAL_SQUARE_H

class Square{

    int value{};
    int x{};
    int y{};
    int actualStatus{};
    int formerStatus{};

public:

    Square() = default;
    Square(int value, int x, int y) : x(x), y(y), value(value) {}
    Square(int value, int x, int y, int actualStatus) : x(x), y(y), actualStatus(actualStatus), value(value) {}

    int getX() const {
        return x;
    }

    void setX(int x) {
        Square::x = x;
    }

    int getY() const {
        return y;
    }

    void setY(int y) {
        Square::y = y;
    }

    int getActualStatus() const {
        return actualStatus;
    }

    void setActualStatus(int actualStatus) {
        Square::actualStatus = actualStatus;
    }

    int getFormerStatus() const {
        return formerStatus;
    }

    void setFormerStatus(int formerStatus) {
        Square::formerStatus = formerStatus;
    }

    bool operator==(const Square &rhs) const {
        return x == rhs.x &&
               y == rhs.y;
    }

    bool operator!=(const Square &rhs) const {
        return !(rhs == *this);
    }

    Square& operator=(const int& other)
    {
        if (this->value != other) {
            value = other;
        }
        return *this;
    }


};

#endif //FRACTAL_SQUARE_H
