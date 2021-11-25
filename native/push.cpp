#include "push.hpp"
#include "auto_defer.hpp"
#include "time.hpp"
#include "json.hpp"
namespace xlab
{

    Push::Push(const std::string &url, int64_t bitrate, int width, int height, int channels)
        : is_quit_(false), run_success_flag_(false), url_(url), bitrate_(bitrate), width_(width), height_(height), channels_(channels)

    {
        thread_ = std::thread(&Push::run, this);
    }

    Push::~Push()
    {
        is_quit_ = true;
        int_cb_.exit();
        packet_sem_.Post();
        video_head_sem_.Post();
        audio_head_sem_.Post();
        nap_.Post();

        if (thread_.joinable())
        {
            thread_.join();
        }
    }

    void Push::setVideoHead(const unsigned char *data, int size)
    {
        if (is_quit_)
        {
            return;
        }

        video_head_ = std::vector<unsigned char>(data, data + size);
        video_head_sem_.Post();
    }

    void Push::setAudioHead(const unsigned char *data, int size)
    {
        if (is_quit_)
        {
            return;
        }

        audio_head_ = std::vector<unsigned char>(data, data + size);
        audio_head_sem_.Post();
    }

    void Push::postVideoBody(const unsigned char *data, int size, int64_t timestamp, bool is_key)
    {
        if (is_quit_)
        {
            return;
        }

        auto pkt = std::make_shared<Packet>();
        pkt->addData(data, size);
        pkt->stream_index = 0;
        pkt->pts = timestamp;
        pkt->flags = is_key ? AV_PKT_FLAG_KEY : 0;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            packet_list_.push_back(pkt);
            packet_sem_.Post();
        }
    }

    void Push::postAudioBody(const unsigned char *data, int size, int64_t timestamp)
    {
        if (is_quit_)
        {
            return;
        }

        auto pkt = std::make_shared<Packet>();
        pkt->addData(data, size);
        pkt->stream_index = 1;
        pkt->pts = timestamp;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            packet_list_.push_back(pkt);
            packet_sem_.Post();
        }
    }

    void Push::run()
    {
        auto_defer run_over([this]
                            { is_quit_ = true; });
        audio_head_sem_.Wait();
        video_head_sem_.Wait();
        if (is_quit_)
        {
            return;
        }

        while (!is_quit_)
        {
            const auto start_result = start();
            auto_defer clean_up([this]
                                { stop(); });
            if (!start_result)
            {
                nap_.TimedWait(1s);
                continue;
            }

            while (!is_quit_)
            {
                if (doSend())
                {
                    onSuccess();
                }

                onNetBand();
            }
        }

        if (!is_quit_)
        {
            onError();
        }
    }

    bool Push::start()
    {
        

        return true;
    }

    void Push::stop()
    {
    }

    bool Push::doSend()
    {
        return true;
    }

    void Push::onNetBand()
    {
        
        
        if (event != nullptr)
        {
            nlohmann::json json;
            json["event"] = "onPushNetBand";
            json["bitrate"] = 0;
            event->onMessage(json.dump());
        }
    }

    void Push::onSuccess()
    {
        if (run_success_flag_)
        {
            return;
        }

        run_success_flag_ = true;
        if (event != nullptr)
        {
            nlohmann::json json;
            json["event"] = "onPushSuccess";
            event->onMessage(json.dump());
        }
    }

    void Push::onError()
    {
        if (event != nullptr)
        {
            nlohmann::json json;
            json["event"] = "onPushError";
            event->onMessage(json.dump());
        }
    }
}