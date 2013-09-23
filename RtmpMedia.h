// RtmpMedia.h

#ifndef _PPBOX_RTMPC_RTMP_MEDIA_H_
#define _PPBOX_RTMPC_RTMP_MEDIA_H_

#include "ppbox/rtmpc/RtmpSource.h"

#include <ppbox/data/packet/PacketMedia.h>

#include <util/protocol/rtmp/RtmpClient.h>

namespace ppbox
{
    namespace rtmpc
    {

        class RtmpMedia
            : public ppbox::data::PacketMedia
        {
        public:
            RtmpMedia(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            virtual ~RtmpMedia();

        public:
            virtual void async_open(
                response_type const & resp);

            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

        public:
            virtual bool get_basic_info(
                ppbox::data::MediaBasicInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_info(
                ppbox::data::MediaInfo & info,
                boost::system::error_code & ec) const;

        public:
            virtual bool get_packet_feature(
                ppbox::data::PacketFeature & feature,
                boost::system::error_code & ec) const;

            virtual util::stream::Source & source();

        private:
            void handle_open(
                boost::system::error_code const & ec, 
                MediaBase::response_type const & resp);

        private:
            RtmpSource source_;
            ppbox::data::MediaInfo info_;
        };

        PPBOX_REGISTER_MEDIA_BY_PROTOCOL("rtmp", RtmpMedia);

    } // data
} // ppbox

#endif // _PPBOX_RTMPC_RTMP_MEDIA_H_
