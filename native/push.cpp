#include "push.hpp"
#include "auto_defer.hpp"
#include "json.hpp"
#include "xlog.hpp"
namespace xlab
{

    static inline std::string fferr(int ret)
    {
        char errStr[256] = {0};
        av_strerror(ret, errStr, sizeof(errStr));

        return std::string(errStr) + "(" + std::to_string(ret) + ")";
    }

    Push::Push(const std::string &url, int64_t bitrate, int width, int height, int channels)
        : url_(url), bitrate_(bitrate), width_(width), height_(height), channels_(channels)

    {
        avformat_network_init();
        xlog("run push ...");
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

        xlog("wait close push ...");
        if (thread_.joinable())
        {
            thread_.join();
        }
        xlog("close push over !!!");
        avformat_network_deinit();
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

        pushPacket(pkt);
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

        pushPacket(pkt);
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
                                {
                                    stop();
                                    cleanPacket();
                                    if (!is_quit_)
                                    {
                                        //onError();
                                        onStats();
                                    }
                                });

            if (!start_result)
            {
                nap_.TimedWait(2s);
                continue;
            }

            while (!is_quit_)
            {
                if (!doSend())
                {
                    nap_.TimedWait(300ms);
                    break;
                }

                onSuccess();
                onStats();
            }
        }
    }

    bool Push::start()
    {
        fmt_ctx_ = nullptr;
        video_codec_ctx_ = nullptr;
        audio_codec_ctx_ = nullptr;
        options_ = nullptr;
        io_open_result_ = -1;
        io_write_head_result_ = -1;
        send_packet_size_ = 0;

        const auto ret = avformat_alloc_output_context2(&fmt_ctx_, nullptr, "mpegts", url_.c_str());
        if (fmt_ctx_ == nullptr || ret < 0)
        {
            xlog("%d:%s \n", __LINE__, fferr(ret).c_str());
            return false;
        }

        fmt_ctx_->interrupt_callback = int_cb_.getIntCB();
        fmt_ctx_->max_interleave_delta = 1000;

        if (!buildVideoStream())
        {
            return false;
        }

        if (!buildAudioStream())
        {
            return false;
        }

        if (!(fmt_ctx_->flags & AVFMT_NOFILE))
        {
            io_open_result_ = avio_open2(&fmt_ctx_->pb, url_.c_str(), AVIO_FLAG_WRITE, &fmt_ctx_->interrupt_callback, &options_);
            if (io_open_result_ < 0)
            {
                xlog("%d:%s \n", __LINE__, fferr(io_open_result_).c_str());
                return false;
            }
        }

        io_write_head_result_ = avformat_write_header(fmt_ctx_, nullptr);
        if (io_write_head_result_ < 0)
        {
            xlog("%d:%s \n", __LINE__, fferr(io_write_head_result_).c_str());
            return false;
        }

        return true;
    }

    void Push::stop()
    {
        if (fmt_ctx_ != nullptr)
        {
            if (io_write_head_result_ >= 0)
            {
                av_write_trailer(fmt_ctx_);
            }

            if ((fmt_ctx_->pb != nullptr) && !(fmt_ctx_->flags & AVFMT_NOFILE) && (io_open_result_ >= 0))
            {
                avio_closep(&fmt_ctx_->pb);
            }

            avcodec_free_context(&audio_codec_ctx_);
            avcodec_free_context(&video_codec_ctx_);
            avformat_free_context(fmt_ctx_);
            fmt_ctx_ = nullptr;
        }

        av_dict_free(&options_);
    }

    bool Push::doSend()
    {
        std::shared_ptr<Packet> packet = nullptr;
        while (!is_quit_)
        {
            packet = popPacket();
            if (packet != nullptr)
            {
                break;
            }
        }

        av_packet_rescale_ts(packet.get(), AV_TIME_BASE_Q, fmt_ctx_->streams[packet->stream_index]->time_base);
        const int write_result = av_write_frame(fmt_ctx_, packet.get());
        if (write_result < 0)
        {
            xlog("%d:%s \n", __LINE__, fferr(write_result).c_str());
            return false;
        }

        send_packet_size_ += packet->size;

        return true;
    }

    void Push::onStats()
    {
        onNetBand();
        onNetStatus();
    }

    void Push::onNetBand()
    {
        if (!get_net_band_start_time_.has_value())
        {
            get_net_band_start_time_ = Time::Point::Now();
            return;
        }

        const auto get_netband_time_diff = Time::Point::Now() - get_net_band_start_time_.value();
        if (get_netband_time_diff >= 2000ms)
        {
            const int64_t netband = send_packet_size_ * 8 / get_netband_time_diff.ToChrono<s>().count();
            get_net_band_start_time_ = Time::Point::Now();
            send_packet_size_ = 0;
            if (event != nullptr)
            {
                nlohmann::json json;
                json["event"] = "onPushNetBand";
                json["netband"] = netband;
                event->onMessage(json.dump());
            }
        }
    }

    void Push::onNetStatus()
    {
        if (!get_net_status_start_time_.has_value())
        {
            get_net_status_start_time_ = Time::Point::Now();
            return;
        }

        const auto get_net_status_time_diff = Time::Point::Now() - get_net_status_start_time_.value();
        if (get_net_status_time_diff >= 10000ms)
        {
            int lost_percent = 0;
            int64_t remain_bytes = 0;
            std::lock_guard<decltype(packet_mutex_)> lock(packet_mutex_);
            for (const auto &it : packet_list_)
            {
                remain_bytes += it->size;
            }

            if (post_packet_bytes_ > remain_bytes)
            {
                const int take_bytes = post_packet_bytes_ - remain_bytes;
                lost_percent = static_cast<int>((lost_packet_bytes_ * 1.0 / take_bytes) * 100);
            }

            post_packet_bytes_ = remain_bytes;
            get_net_status_start_time_ = Time::Point::Now();
            const int net_status = lost_percent <= 9 ? 0 : -1;
            if (event != nullptr)
            {
                nlohmann::json json;
                json["event"] = "onPushNetStatus";
                json["netstatus"] = net_status;
                event->onMessage(json.dump());
            }
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

        xlog("push ok!!!");
    }

    void Push::onError()
    {
        xlog("push error!!!");
        if (event != nullptr)
        {
            nlohmann::json json;
            json["event"] = "onPushError";
            event->onMessage(json.dump());
        }
    }

    bool Push::buildVideoStream()
    {
        const auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (codec == nullptr)
        {
            return false;
        }

        auto codecCtx = avcodec_alloc_context3(codec);
        if (codecCtx == nullptr)
        {
            return false;
        }

        codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
        codecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
        codecCtx->codec_id = codec->id;
        codecCtx->bit_rate = bitrate_;
        codecCtx->width = width_;
        codecCtx->height = height_;
        codecCtx->coded_width = codecCtx->width;
        codecCtx->coded_height = codecCtx->height;
        codecCtx->time_base = {1, 30};
        codecCtx->gop_size = 30;
        codecCtx->codec_tag = 0;
        codecCtx->extradata = static_cast<uint8_t *>(av_memdup(video_head_.data(), video_head_.size() + AV_INPUT_BUFFER_PADDING_SIZE));
        codecCtx->extradata_size = video_head_.size();

        fmt_ctx_->oformat->video_codec = codecCtx->codec_id;
        if (fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        {
            codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        auto stream = avformat_new_stream(fmt_ctx_, nullptr);
        if (stream == nullptr)
        {
            return false;
        }

        avcodec_parameters_from_context(stream->codecpar, codecCtx);
        stream->id = 0;
        stream->time_base = codecCtx->time_base;
        stream->avg_frame_rate = av_inv_q(codecCtx->time_base);
        stream->r_frame_rate = stream->avg_frame_rate;

        video_codec_ctx_ = codecCtx;

        return true;
    }

    bool Push::buildAudioStream()
    {
        const auto codec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        auto codecCtx = avcodec_alloc_context3(codec);
        if (codecCtx == nullptr)
        {
            return false;
        }

        codecCtx->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_S16;
        codecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
        codecCtx->codec_id = codec->id;
        codecCtx->bit_rate = 128000;
        codecCtx->channels = channels_;
        codecCtx->channel_layout = channels_ == 1 ? AV_CH_LAYOUT_MONO : AV_CH_LAYOUT_STEREO;
        codecCtx->sample_rate = 48000;
        codecCtx->time_base = {1, codecCtx->sample_rate};
        codecCtx->codec_tag = 0;
        codecCtx->extradata = static_cast<uint8_t *>(av_memdup(audio_head_.data(), audio_head_.size() + AV_INPUT_BUFFER_PADDING_SIZE));
        codecCtx->extradata_size = audio_head_.size();

        fmt_ctx_->oformat->audio_codec = codecCtx->codec_id;
        if (fmt_ctx_->oformat->flags & AVFMT_GLOBALHEADER)
        {
            codecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        auto stream = avformat_new_stream(fmt_ctx_, nullptr);
        if (stream == nullptr)
        {
            return false;
        }

        avcodec_parameters_from_context(stream->codecpar, codecCtx);
        stream->id = 1;
        stream->time_base = codecCtx->time_base;

        audio_codec_ctx_ = codecCtx;

        return true;
    }

    bool Push::pushPacket(std::shared_ptr<Packet> packet)
    {
        std::lock_guard<decltype(packet_mutex_)> lock(packet_mutex_);
        if (packet->stream_index == 0)
        {
            if (((packet->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY) && need_key_frame_)
            {
                need_key_frame_ = false;
            }
        }

        if (!need_key_frame_)
        {
            packet_list_.push_back(packet);
            post_packet_bytes_ += packet->size;
            packet_sem_.Post();
        }

        int64_t key_frame_count = 0;
        for (const auto &it : packet_list_)
        {
            if (it->stream_index == 0 && ((it->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY))
            {
                ++key_frame_count;
            }
        }

        if (key_frame_count == 0)
        {
            bool need_clean_packet = packet_list_.size() >= 2000;
            if (!need_clean_packet && packet_list_.size() >= 1000)
            {
                need_clean_packet = packet_list_.end() == std::find_if(packet_list_.begin(), packet_list_.end(), [](const std::shared_ptr<Packet> &it)
                                                                { return it->stream_index == 0; });
            }

            if (need_clean_packet)
            {
                packet_list_.remove_if([this](const std::shared_ptr<Packet> &it)
                                       {
                                           lost_packet_bytes_ += it->size;
                                           need_key_frame_ = true;
                                           return true;
                                       });
            }
        }
        else if ((key_frame_count > 0) && (key_frame_count < 3))
        {
        }
        else if ((key_frame_count >= 3) && (key_frame_count < 10))
        {
            int key_cnt = 0;
            packet_list_.remove_if([&](const std::shared_ptr<Packet> &it)
                                   {
                                       if (it->stream_index == 0 && ((it->flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY))
                                       {
                                           ++key_cnt;
                                           return false;
                                       }

                                       if (key_cnt == 1)
                                       {
                                           lost_packet_bytes_ += it->size;
                                           return true;
                                       }

                                       return false;
                                   });
        }
        else
        {
            packet_list_.remove_if([this](const std::shared_ptr<Packet> &it)
                                   {
                                       lost_packet_bytes_ += it->size;
                                       need_key_frame_ = true;
                                       return true;
                                   });
        }

        return true;
    }

    const std::shared_ptr<Packet> Push::popPacket()
    {
        packet_sem_.TimedWait(10ms);
        std::shared_ptr<Packet> packet = nullptr;
        {
            std::lock_guard<decltype(packet_mutex_)> lock(packet_mutex_);
            if (!packet_list_.empty())
            {
                packet = packet_list_.front();
                packet_list_.pop_front();
            }
        }

        return packet;
    }

    void Push::cleanPacket()
    {
        std::lock_guard<decltype(packet_mutex_)> lock(packet_mutex_);
        packet_list_.remove_if([this](const std::shared_ptr<Packet> &it)
                               {
                                   lost_packet_bytes_ += it->size;
                                   need_key_frame_ = true;
                                   return true;
                               });
    }

}