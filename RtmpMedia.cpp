// RtmpMedia.cpp

#include "ppbox/rtmpc/Common.h"
#include "ppbox/rtmpc/RtmpMedia.h"

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/string/Algorithm.h>

#include <boost/bind.hpp>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.rtmpc.RtmpMedia", framework::logger::Debug);

namespace ppbox
{
    namespace rtmpc
    {

        RtmpMedia::RtmpMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PacketMedia(io_svc, url)
            , client_(io_svc)
            , source_(client_)
            , open_step_(0)
            , connect_url_(url_)
        {
            //connect_url_.param_clear();
            std::string::size_type pos = connect_url_.path().find('/', 1);
            path_ = connect_url_.path_all().substr(pos + 1);
            connect_url_.path(connect_url_.path().substr(0, pos));

            boost::system::error_code ec;
            PacketMedia::get_basic_info(info_, ec);
            info_.format = "rtm";
        }

        RtmpMedia::~RtmpMedia()
        {
        }

        void RtmpMedia::async_open(
            MediaBase::response_type const & resp)
        {
            resp_ = resp;
            handle_async(boost::system::error_code());
        }

        void RtmpMedia::cancel(
            boost::system::error_code & ec)
        {
            client_.cancel(ec);
        }

        void RtmpMedia::close(
            boost::system::error_code & ec)
        {
            client_.close(ec);
        }

        bool RtmpMedia::get_basic_info(
            ppbox::data::MediaBasicInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return true;
        }

        bool RtmpMedia::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = info_;
            ec.clear();
            return true;
        }

        void RtmpMedia::handle_async(
            boost::system::error_code const & ec)
        {
            if (ec) {
                response(ec);
                return;
            }

            switch (open_step_) {
                case 0:
                    open_step_ = 1;
                    client_.async_connect(connect_url_, 
                        boost::bind(&RtmpMedia::handle_async, this, _1));
                    break;
                case 1:
                    open_step_ = 2;
                    client_.async_play(path_, 
                        boost::bind(&RtmpMedia::handle_async, this, _1));
                    break;
                case 2:
                    open_step_ = 3;
                    client_.set_read_parallel(true);
                    response(ec);
                    break;
                case 4: // cancel
                    response(boost::asio::error::operation_aborted);
                    break;
                default:
                    assert(0);
            }
        }

        void RtmpMedia::response(
            boost::system::error_code const & ec)
        {
            MediaBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        bool RtmpMedia::get_packet_feature(
            ppbox::data::PacketFeature & feature,
            boost::system::error_code & ec) const
        {
            feature.piece_size = 1024;
            feature.packet_max_size = 1024 * 100;
            feature.buffer_size = 1024 * 1024 * 2; // 2M
            feature.prepare_size = 1024 * 10;
            ec.clear();
            return true;
        }

        ppbox::data::SourceBase & RtmpMedia::source()
        {
            return source_;
        }


    } // data
} // ppbox
