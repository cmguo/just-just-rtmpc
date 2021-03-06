// RtmpMedia.cpp

#include "just/rtmpc/Common.h"
#include "just/rtmpc/RtmpMedia.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Algorithm.h>

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.rtmpc.RtmpMedia", framework::logger::Debug);

namespace just
{
    namespace rtmpc
    {

        RtmpMedia::RtmpMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PacketMedia(io_svc, url)
            , source_(io_svc)
        {
            boost::system::error_code ec;
            PacketMedia::get_basic_info(info_, ec);
            info_.type = info_.live;
            //info_.flags |= just::data::PacketMediaFlags::f_non_ordered;
            info_.format_type = "rtm";
        }

        RtmpMedia::~RtmpMedia()
        {
        }

        void RtmpMedia::async_open(
            MediaBase::response_type const & resp)
        {
            source_.async_open(url_, 
                boost::bind(&RtmpMedia::handle_open, this, _1, resp));
        }

        void RtmpMedia::handle_open(
            boost::system::error_code const & ec, 
            MediaBase::response_type const & resp)
        {
            if (!ec && source_.is_record()) {
                info_.type = info_.vod;
                info_.flags |= info_.f_seekable;
                info_.flags |= info_.f_pauseable;
            }
            resp(ec);
        }

        void RtmpMedia::cancel(
            boost::system::error_code & ec)
        {
            source_.cancel(ec);
        }

        void RtmpMedia::close(
            boost::system::error_code & ec)
        {
            source_.close(ec);
        }

        bool RtmpMedia::get_basic_info(
            just::avbase::MediaBasicInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return true;
        }

        bool RtmpMedia::get_info(
            just::avbase::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return PacketMedia::get_info(info, ec);
        }

        bool RtmpMedia::get_packet_feature(
            just::data::PacketFeature & feature,
            boost::system::error_code & ec) const
        {
            feature.piece_size = 1024;
            feature.packet_max_size = 1024 * 100;
            feature.buffer_size = 1024 * 1024 * 2; // 2M
            feature.prepare_size = 1024 * 10;
            ec.clear();
            return true;
        }

        util::stream::Source & RtmpMedia::source()
        {
            return source_;
        }


    } // rtmpc
} // just
