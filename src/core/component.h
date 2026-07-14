#pragma once

class Component
{
public:
    // A virtual destructor is required: components are owned and destroyed
    // through base-class pointers (unique_ptr<Component>). Without it,
    // deleting a derived component via the base pointer is undefined
    // behavior.
    virtual ~Component() = default;
};
