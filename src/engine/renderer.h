#ifndef RENDERER_H
#define RENDERER_H

#include "renderer_exception.h"

class Renderer
{
protected:
    int winWidth = 1280;
    int winHeight = 720;
    const char *winName = "Window";

    virtual void start() throw() = 0;
    virtual void update() = 0;
    virtual void render() = 0;
    virtual void destroy() = 0;

public:
    Renderer() = default;
    virtual int init(int width, int height, const char *name) = 0;
};

#endif