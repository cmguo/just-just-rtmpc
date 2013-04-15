// RtmFilter.h

#ifndef _PPBOX_RTMPC_RTM_FILTER_H_
#define _PPBOX_RTMPC_RTM_FILTER_H_

#include <ppbox/demux/packet/Filter.h>

#include <ppbox/avformat/flv/FlvTagType.h>

#include <util/buffers/CycleBuffers.h>

namespace ppbox
{
    namespace rtmpc
    {

        class RtmFilter
            : public ppbox::demux::Filter
        {
        public:
            RtmFilter();

            ~RtmFilter();

        public:
            virtual bool get_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_next_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_last_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool before_seek(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            void parse_sub_types(
                std::basic_streambuf<boost::uint8_t> & buf,
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

            bool get_aggregate_sample(
                ppbox::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            ppbox::avformat::FlvTag tag_;
            boost::uint32_t video_track_;
            boost::uint32_t audio_track_;
            bool is_save_sample_;
            ppbox::demux::Sample sample_; // AggregateMessage
            typedef util::buffers::CycleBuffers<
                std::deque<boost::asio::const_buffer>, boost::uint8_t
            > cycle_buffer_t ;
            cycle_buffer_t buf_;
        };

    } // namespace mux
} // namespace ppbox

#endif // _PPBOX_RTMPC_RTM_FILTER_H_
