#ifndef RENDER_MODE_H
#define RENDER_MODE_H

#include "camera.h"
#include "renderer.h"

class Sim;

class RenderMode {
public:
    RenderMode(Sim& sim) : sim_(sim) {}
    virtual Camera& get_camera() = 0;
    virtual void render() const = 0;
    virtual void step() = 0;
    virtual void init() = 0;
    virtual void end() = 0;
protected:
    Sim& sim_;
};

#endif