/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: Position.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_POSITION_H
#define	GAMEENGINE_POSITION_H

class Position {
public:
    Position();
    Position(const Position&);
    Position(int, int);
    virtual ~Position();
    void setX(int);
    const int& getX() const;
    void setY(int);
    const int& getY() const;
private:
    int x_ = 0;
    int y_ = 0;
};

#endif //GAMEENGINE_POSITION_H