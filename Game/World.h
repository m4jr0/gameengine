/*******************************************************************************
 * Licence: http://creativecommons.org/licenses/by-nc-sa/3.0/
 * File: World.h
 * Author: m4jr0
 * Description:
 ******************************************************************************/

#ifndef GAMEENGINE_WORLD_H
#define	GAMEENGINE_WORLD_H

#include <ctime>

class World {
public:
    World();
    World(double);
    World(double, unsigned int);
    World(const World&);
    virtual ~World();
    const double getMsPerUpdate() const;
    void setMsPerUpdate(double);
    const unsigned int getFpsCap() const;
    void setFpsCap(unsigned int);
    void loop();
    static constexpr double DEFAULT_MS_PER_UPDATE = 1000 / double(60);
    static const unsigned int DEFAULT_FPS_CAP = 60;
private:
    double msPerUpdate_;
    unsigned int fpsCap_;
    bool running_;
};

#endif //GAMEENGINE_WORLD_H