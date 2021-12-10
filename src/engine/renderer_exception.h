#ifndef RENDERER_EXCEPTION_H
#define RENDERER_EXCEPTION_H

#include <stdexcept>

class RendererException : public std::exception
{
private:
    const char *msg;

public:
    RendererException(const char *msg)
    {
        this->msg = msg;
    }
    const char *what() const throw() { return msg; }
};

#endif