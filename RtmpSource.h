// RtmpSource.h

#ifndef _PPBOX_RTMPC_RTMP_SOURCE_H_
#define _PPBOX_RTMPC_RTMP_SOURCE_H_

#include <ppbox/data/base/SourceBase.h>

#include <util/protocol/rtmp/RtmpClient.h>

namespace ppbox
{
    namespace rtmpc
    {

        struct rtmp_source_read_handler;

        class RtmpSource
            : public ppbox::data::SourceBase
        {
        public:
            RtmpSource(
                util::protocol::RtmpClient & rtmp);

            ~RtmpSource();

        public:
            virtual boost::system::error_code cancel(
                boost::system::error_code & ec);

        public:
            virtual boost::system::error_code set_non_block(
                bool non_block, 
                boost::system::error_code & ec);

            virtual boost::system::error_code set_time_out(
                boost::uint32_t time_out, 
                boost::system::error_code & ec);

            virtual bool continuable(
                boost::system::error_code const & ec);

        private:
            // implement util::stream::Source
            virtual std::size_t private_read_some(
                util::stream::StreamMutableBuffers const & buffers,
                boost::system::error_code & ec);

            virtual void private_async_read_some(
                util::stream::StreamMutableBuffers const & buffers,
                util::stream::StreamHandler const & handler);

        private:
            friend struct rtmp_source_read_handler;

            void handle_read_some(
                util::stream::StreamMutableBuffers const & buffers,
                util::stream::StreamHandler const & handler, 
                boost::system::error_code const & ec, 
                size_t bytes_transferred);

        protected:
            bool flag_;
            util::protocol::RtmpClient & rtmp_;
        };

    } // namespace data
} // namespace ppbox

#endif // _PPBOX_RTMPC_RTMP_SOURCE_H_
