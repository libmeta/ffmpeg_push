#pragma once

#include <string>
#include <list>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <optional>

#include "event.hpp"
#include "semaphore.hpp"
#include "packet.hpp"
#include "intcb.hpp"
#include "time.hpp"

namespace xlab
{
    class Push
    {
    public:
        explicit Push(const std::string &url, int64_t bitrate, int width, int height, int channels);

        ~Push();

        void setVideoHead(const unsigned char *data, int size);

        void setAudioHead(const unsigned char *data, int size);

        void postVideoBody(const unsigned char *data, int size, int64_t timestamp, bool is_key);

        void postAudioBody(const unsigned char *data, int size, int64_t timestamp);

    protected:
        void run();

    private:
        bool start();

        void stop();

        bool doSend();

        void onNetBand();

        void onSuccess();

        void onError();
        
    public:
        static inline std::shared_ptr<IEvent> event = nullptr;

    private:
        std::thread thread_;
        std::mutex mutex_;
        std::atomic<bool> is_quit_;

        std::list<std::shared_ptr<Packet>> packet_list_;
        std::vector<unsigned char> video_head_;
        std::vector<unsigned char> audio_head_;
        Semaphore packet_sem_;
        Semaphore video_head_sem_;
        Semaphore audio_head_sem_;
        Semaphore nap_;
        IntCB int_cb_;

        std::string url_;
        int64_t bitrate_;
        int width_;
        int height_;
        int channels_;

        bool run_success_flag_;
        int64_t send_packet_size_;

        std::optional<Time::Point> start_time_{};
        
    };

}