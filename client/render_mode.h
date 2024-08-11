#ifndef RENDER_MODE_H
#define RENDER_MODE_H

#include "camera.h"
#include "renderer.h"

class RenderMode {
public:
    virtual Camera& get_camera() = 0;
    virtual void render(Renderer& renderer) const = 0;
private:
};

#endif