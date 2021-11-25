#pragma once

#include "ffhead.hpp"

namespace xlab
{
    class Packet : public AVPacket
    {
    public:
        static void getPacketDefaults(AVPacket *pkt)
        {
            if (pkt == nullptr)
            {
                return;
            }

            memset(pkt, 0, sizeof(*pkt));
            pkt->pts = AV_NOPTS_VALUE;
            pkt->dts = AV_NOPTS_VALUE;
            pkt->pos = -1;
        }

        explicit Packet()
        {
            getPacketDefaults(this);
        }

        Packet(size_t size)
        {
            auto ret = av_new_packet(this, size);
        }

        Packet(const AVPacket &packet)
        {
            auto ret = av_packet_ref(this, &packet);
            //     av_packet_make_refcounted(this);
        }

        virtual ~Packet()
        {
            reset();
        }

        bool addSideData(const uint8_t *data, size_t size, AVPacketSideDataType type = AVPacketSideDataType::AV_PKT_DATA_NEW_EXTRADATA)
        {
            auto sideData = (uint8_t *)(av_memdup(data, size));
            auto ret = av_packet_add_side_data(this, type, sideData, size);
            return ret == 0;
        }

        const uint8_t *getSideData(size_t *size, AVPacketSideDataType type = AVPacketSideDataType::AV_PKT_DATA_NEW_EXTRADATA)
        {
            return av_packet_get_side_data(this, type, reinterpret_cast<int *>(size));
        }

        void freeSideData()
        {
            av_packet_free_side_data(this);
        }

        bool addData(const uint8_t *data, int size)
        {
            auto Data = (uint8_t *)(av_memdup(data, size));
            auto ret = av_packet_from_data(this, Data, size);
            return ret == 0;
        }

        void freeData()
        {
            av_buffer_unref(&this->buf);
            this->data = nullptr;
            this->size = 0;
        }

        void reset()
        {
            av_packet_unref(this);
        }

    };

} // namesxlabe

