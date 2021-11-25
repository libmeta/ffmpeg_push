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

        void onStats();

        void onNetBand();

        void onNetStatus();

        void onSuccess();

        void onError();

        bool buildVideoStream();

        bool buildAudioStream();

        bool pushPacket(std::shared_ptr<Packet> packet);

        const std::shared_ptr<Packet> popPacket();

        void cleanPacket();

    public:
        static inline std::shared_ptr<IEvent> event = nullptr;

    private:
        std::thread thread_;
        std::mutex packet_mutex_;
        std::atomic<bool> is_quit_{false};

        std::list<std::shared_ptr<Packet>> packet_list_;
        std::vector<unsigned char> video_head_;
        std::vector<unsigned char> audio_head_;
        Semaphore packet_sem_;
        Semaphore video_head_sem_;
        Semaphore audio_head_sem_;
        Semaphore nap_;
        IntCB int_cb_{};

        std::string url_;
        int64_t bitrate_;
        int width_;
        int height_;
        int channels_;

        AVFormatContext *fmt_ctx_ = nullptr;
        AVCodecContext *video_codec_ctx_ = nullptr;
        AVCodecContext *audio_codec_ctx_ = nullptr;
        AVDictionary *options_ = nullptr;
        int io_open_result_ = -1;
        int io_write_head_result_ = -1;

        bool run_success_flag_ = false;
        int64_t send_packet_size_ = 0;
        std::optional<Time::Point> get_net_band_start_time_{};
        std::optional<Time::Point> get_net_status_start_time_{};

        std::atomic<bool> need_key_frame_{true};
        int64_t post_packet_bytes_ = 0;
        int64_t lost_packet_bytes_ = 0;
    };

}