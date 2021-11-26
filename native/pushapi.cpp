#include "push.hpp"

static std::mutex gMutex;
static std::shared_ptr<xlab::Push> gPush = nullptr;

namespace PushApi
{
    static inline void set_event(std::shared_ptr<xlab::IEvent> evt)
    {
        xlab::Push::event = evt;
    }

    static inline void start_up(const char *url, int64_t bitrate, int width, int height, int channels)
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush = nullptr;
        gPush = std::make_shared<xlab::Push>(url, bitrate, width, height, channels);
    }

    static inline void clean_up()
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush = nullptr;
    }

    static inline void set_video_head(const unsigned char *data, int size)
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush->setVideoHead(data, size);
    }

    static inline void set_audio_head(const unsigned char *data, int size)
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush->setAudioHead(data, size);
    }

    static inline void post_video_body(const unsigned char *data, int size, int64_t timestamp, bool is_key)
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush->postVideoBody(data, size, timestamp, is_key);
    }

    static inline void post_audio_body(const unsigned char *data, int size, int64_t timestamp)
    {
        std::lock_guard<std::mutex> lock{gMutex};
        gPush->postAudioBody(data, size, timestamp);
    }

}
