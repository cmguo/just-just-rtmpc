// RtmDemuxer.h

#ifndef _JUST_RTMPC_RTM_DEMUXER_H_
#define _JUST_RTMPC_RTM_DEMUXER_H_

#include "just/demux/packet/PacketDemuxer.h"

#include <just/avformat/flv/FlvMetaData.h>

namespace just
{
    namespace rtmpc
    {

        class RtmFilter;

        class RtmDemuxer
            : public just::demux::PacketDemuxer
        {
        public:
            RtmDemuxer(
                boost::asio::io_service & io_svc, 
                just::data::PacketMedia & media);

            virtual ~RtmDemuxer();

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            just::avformat::FlvMetaData meta_;
            RtmFilter * filter_;
        };

        JUST_REGISTER_PACKET_DEMUXER("rtm", RtmDemuxer);

    } // namespace rtmpc
} // namespace just

#endif // _JUST_RTMPC_RTM_DEMUXER_H_
