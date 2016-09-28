/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: Position.cpp
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#include "Position.h"

Position::Position(): x_(0), y_(0) {

}

Position::Position(const Position& orig) {
    this->x_ = orig.getX();
    this->y_ = orig.getY();
}

Position::Position(int x, int y) {
    this->x_ = x;
    this->y_ = y;
}

Position::~Position() {
}

void Position::setX(int x) {
    this->x_ = x;
}

const int& Position::getX() const {
    return this->x_;
}

void Position::setY(int y) {
    this->y_ = y;
}

const int& Position::getY() const {
    return this->y_;
}