// RtmDemuxer.h

#ifndef _PPBOX_RTMPC_RTM_DEMUXER_H_
#define _PPBOX_RTMPC_RTM_DEMUXER_H_

#include "ppbox/demux/packet/PacketDemuxer.h"

#include <ppbox/avformat/flv/FlvMetaData.h>

namespace ppbox
{
    namespace rtmpc
    {

        class RtmFilter;

        class RtmDemuxer
            : public ppbox::demux::PacketDemuxer
        {
        public:
            RtmDemuxer(
                boost::asio::io_service & io_svc, 
                ppbox::data::PacketMedia & media);

            virtual ~RtmDemuxer();

        protected:
            virtual bool check_open(
                boost::system::error_code & ec);

        private:
            ppbox::avformat::FlvMetaData meta_;
            RtmFilter * filter_;
        };

        PPBOX_REGISTER_PACKET_DEMUXER("rtm", RtmDemuxer);

    } // namespace rtmpc
} // namespace ppbox

#endif // _PPBOX_RTMPC_RTM_DEMUXER_H_
