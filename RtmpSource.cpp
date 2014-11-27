// RtmpSource.cpp

#include "just/rtmpc/Common.h"
#include "just/rtmpc/RtmpSource.h"

#include <util/protocol/rtmp/RtmpError.h>
#include <util/protocol/rtmp/RtmpSocket.hpp>
#include <util/protocol/rtmp/RtmpMessageDataData.h>
using namespace util::protocol;

#include <boost/asio/buffer.hpp>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>

namespace just
{
    namespace rtmpc
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.rtmpc.RtmpSource", framework::logger::Debug);

        RtmpSource::RtmpSource(
            boost::asio::io_service & io_svc)
            : util::stream::UrlSource(io_svc)
            , client_(io_svc)
            , open_step_(0)
        {
            RtmpDataMessage0 data0;
            RtmpDataMessage3 data3;
            RtmpVideoMessage video;
            RtmpAudioMessage audio;
            RtmpAggregateMessage aggregate;
            RtmpMessage msg;
            msg.reset(data0);
            msg.reset(data3);
            msg.reset(video);
            msg.reset(audio);
            msg.reset(aggregate);
        }

        RtmpSource::~RtmpSource()
        {
        }

        bool RtmpSource::open(
            framework::string::Url const & url, 
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            url_ = url;
            open_step_ = 1;
            is_open(ec);
            return !ec;
        }

        void RtmpSource::async_open(
            framework::string::Url const & url, 
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            url_ = url;
            resp_ = resp;
            handle_open(boost::system::error_code());
        }

        void RtmpSource::handle_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                response(ec);
                return;
            }

            switch (open_step_) {
                case 0:
                    open_step_ = 1;
                    client_.async_connect(url_, 
                        boost::bind(&RtmpSource::handle_open, this, _1));
                    break;
                case 1:
                    open_step_ = 2;
                    client_.async_play(
                        boost::bind(&RtmpSource::handle_open, this, _1));
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

        void RtmpSource::response(
            boost::system::error_code const & ec)
        {
            response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        bool RtmpSource::is_open(
            boost::system::error_code & ec)
        {
            if (open_step_ == 3) {
                ec.clear();
                return true;
            }

            switch (open_step_) {
                case 1:
                    if (client_.connect(url_, ec))
                        break;
                    open_step_ = 2;
                case 2:
                    if (client_.play(ec))
                        break;
                    open_step_ = 3;
                    client_.set_read_parallel(true);
                default:
                    assert(0);
            }

            return !ec;
        }

        bool RtmpSource::close(
            boost::system::error_code & ec)
        {
            return !client_.close(ec);
        }

        bool RtmpSource::cancel(
            boost::system::error_code & ec)
        {
            return !client_.cancel_forever(ec);
        }

        size_t RtmpSource::private_read_some(
            buffers_t const & buffers,
            boost::system::error_code & ec)
        {
            client_.tick(ec);
            RtmpMessageHeaderEx & header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx *>(*buffers.begin());
            size_t n = client_.read_raw_msg(header, util::buffers::sub_buffers(buffers, sizeof(header)), ec);
            return n ? n + sizeof(header) : 0;
        }

        struct rtmp_source_read_handler
        {
            rtmp_source_read_handler(
                RtmpSource & source, 
                util::stream::StreamMutableBuffers const & buffers, 
                util::stream::StreamHandler const & handler)
                : source_(source)
                , buffers_(buffers)
                , handler_(handler)
            {
            }

            void operator()(
                boost::system::error_code const & ec, 
                size_t bytes_transferred) const
            {
                source_.handle_read_some(buffers_, handler_, ec, bytes_transferred);
            }

        private:
            RtmpSource & source_;
            util::stream::StreamMutableBuffers buffers_;
            util::stream::StreamHandler handler_;
        };

        void RtmpSource::private_async_read_some(
            buffers_t const & buffers,
            handler_t const & handler)
        {
            boost::system::error_code ec;
            client_.tick(ec);
            RtmpMessageHeaderEx & header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx *>(*buffers.begin());
            client_.async_read_raw_msg(header, 
                util::buffers::sub_buffers(buffers, sizeof(header)), 
                rtmp_source_read_handler(*this, buffers, handler));
        }

        void RtmpSource::handle_read_some(
            buffers_t const & buffers,
            handler_t const & handler, 
            boost::system::error_code const & ec, 
            size_t bytes_transferred)
        {
            if (ec == boost::asio::error::would_block) {
                private_async_read_some(buffers, handler);
                return;
            }
            if (bytes_transferred)
                bytes_transferred += sizeof(RtmpMessageHeaderEx);
            handler(ec, bytes_transferred);
        }

        bool RtmpSource::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            return !client_.set_non_block(non_block, ec);
        }

        bool RtmpSource::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            return !client_.set_time_out(time_out, ec);
        }

        bool RtmpSource::continuable(
            boost::system::error_code const & ec)
        {
            return ec == boost::asio::error::would_block;
        }

        bool RtmpSource::is_record() const
        {
            return client_.context().read.stream_is_record(1);
        }

    } // namespace rtmpc
} // namespace just
