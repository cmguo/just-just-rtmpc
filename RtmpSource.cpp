#include "ppbox/rtmpc/Common.h"
#include "ppbox/rtmpc/RtmpSource.h"

#include <util/protocol/rtmp/RtmpError.h>
#include <util/protocol/rtmp/RtmpSocket.hpp>
#include <util/protocol/rtmp/RtmpMessageDataData.h>
using namespace util::protocol;

#include <boost/asio/buffer.hpp>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
#include <framework/logger/StringRecord.h>

namespace ppbox
{
    namespace rtmpc
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.rtmpc.RtmpSource", framework::logger::Debug);

        RtmpSource::RtmpSource(
            RtmpClient & rtmp)
            : SourceBase(rtmp.get_io_service())
            , rtmp_(rtmp)
        {
            RtmpDataMessage0 data0;
            RtmpDataMessage3 data3;
            RtmpVideoMessage video;
            RtmpAudioMessage audio;
            RtmpMessage msg;
            msg.reset(data0);
            msg.reset(data3);
            msg.reset(video);
            msg.reset(audio);
        }

        RtmpSource::~RtmpSource() {}

        boost::system::error_code RtmpSource::cancel(
            boost::system::error_code & ec)
        {
            return rtmp_.cancel_forever(ec);
        }

        size_t RtmpSource::private_read_some(
            util::stream::StreamMutableBuffers const & buffers, 
            boost::system::error_code & ec)
        {
            rtmp_.tick(ec);
            RtmpMessageHeaderEx & header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx *>(*buffers.begin());
            size_t n = rtmp_.read_raw_msg(header, util::buffers::sub_buffers(buffers, sizeof(header)), ec);
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
            util::stream::StreamMutableBuffers const & buffers, 
            util::stream::StreamHandler const & handler)
        {
            boost::system::error_code ec;
            rtmp_.tick(ec);
            RtmpMessageHeaderEx & header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx *>(*buffers.begin());
            rtmp_.async_read_raw_msg(header, 
                util::buffers::sub_buffers(buffers, sizeof(header)), 
                rtmp_source_read_handler(*this, buffers, handler));
        }

        void RtmpSource::handle_read_some(
            util::stream::StreamMutableBuffers const & buffers,
            util::stream::StreamHandler const & handler, 
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

        boost::system::error_code RtmpSource::set_non_block(
            bool non_block, 
            boost::system::error_code & ec)
        {
            return rtmp_.set_non_block(non_block, ec);
        }

        boost::system::error_code RtmpSource::set_time_out(
            boost::uint32_t time_out, 
            boost::system::error_code & ec)
        {
            return rtmp_.set_time_out(time_out, ec);
        }

        bool RtmpSource::continuable(
            boost::system::error_code const & ec)
        {
            return ec == boost::asio::error::would_block;
        }

    } // namespace data
} // namespace ppbox
