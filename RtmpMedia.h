// RtmpMedia.h

#ifndef _JUST_RTMPC_RTMP_MEDIA_H_
#define _JUST_RTMPC_RTMP_MEDIA_H_

#include "just/rtmpc/RtmpSource.h"

#include <just/data/packet/PacketMedia.h>

#include <util/protocol/rtmp/RtmpClient.h>

namespace just
{
    namespace rtmpc
    {

        class RtmpMedia
            : public just::data::PacketMedia
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
                just::data::MediaBasicInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_info(
                just::data::MediaInfo & info,
                boost::system::error_code & ec) const;

        public:
            virtual bool get_packet_feature(
                just::data::PacketFeature & feature,
                boost::system::error_code & ec) const;

            virtual util::stream::Source & source();

        private:
            void handle_open(
                boost::system::error_code const & ec, 
                MediaBase::response_type const & resp);

        private:
            RtmpSource source_;
            just::data::MediaInfo info_;
        };

        JUST_REGISTER_MEDIA_BY_PROTOCOL("rtmp", RtmpMedia);

    } // data
} // just

#endif // _JUST_RTMPC_RTMP_MEDIA_H_
