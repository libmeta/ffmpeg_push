#pragma once

#include <functional>

namespace xlab
{

    struct auto_defer final
    {
        using DeferFunction = std::function<void(void)>;

    private:
        DeferFunction defer_function;

    private:
        auto_defer(const auto_defer &) = delete;

        auto_defer(auto_defer &&) = delete;

        auto_defer &operator=(const auto_defer &) = delete;

        auto_defer &operator=(auto_defer &&) = delete;

    public:
        auto_defer(DeferFunction defer_function) : defer_function(defer_function) {}

        ~auto_defer()
        {
            if (!defer_function)
            {
                return;
            }
            defer_function();
            defer_function = nullptr;
        }

    public:
        DeferFunction cancel_defer()
        {
            DeferFunction old_defer_function = defer_function;
            defer_function = nullptr;
            return old_defer_function;
        }

        void early_execute(bool clean = true)
        {
            if (defer_function)
            {
                defer_function();
            }
            if (clean)
            {
                defer_function = nullptr;
            }
        }
    }; // struct auto_defer

}