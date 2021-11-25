#pragma once

#include <string>

namespace xlab
{
    class IEvent
    {
    public:
        virtual ~IEvent() = default;

        virtual void onMessage(std::string json) = 0;
    };
} // namespace xlab
