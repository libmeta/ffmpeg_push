#pragma once 

#include <atomic>

#include "ffhead.hpp"

namespace xlab {

class IntCB final {
public:
    static int InterruptCallback(void *arg) {
        if (arg == nullptr) { return 1; }
        auto thiz = static_cast<IntCB *>(arg);
        if (thiz->quit) {
            return 1;
        }

        return 0;
    }

    explicit IntCB() : quit(false) {}

    ~IntCB() { quit = true; }

    AVIOInterruptCB getIntCB() {
        return {&IntCB::InterruptCallback, this};
    }

    void exit() { quit = true; }

    const bool isExit() const {
        return quit;
    }

    void reset() {
        quit = false;
    }

private:
    std::atomic<bool> quit;

};

} // namespace xlab