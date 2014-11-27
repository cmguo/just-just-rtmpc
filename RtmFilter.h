// RtmFilter.h

#ifndef _JUST_RTMPC_RTM_FILTER_H_
#define _JUST_RTMPC_RTM_FILTER_H_

#include <just/demux/packet/Filter.h>

#include <just/avformat/flv/FlvTagType.h>

#include <util/buffers/CycleBuffers.h>

namespace just
{
    namespace rtmpc
    {

        class RtmFilter
            : public just::demux::Filter
        {
        public:
            RtmFilter();

            ~RtmFilter();

        public:
            virtual bool get_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_next_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool get_last_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            virtual bool before_seek(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            void parse_sub_types(
                std::basic_streambuf<boost::uint8_t> & buf,
                just::demux::Sample & sample,
                boost::system::error_code & ec);

            bool get_aggregate_sample(
                just::demux::Sample & sample,
                boost::system::error_code & ec);

        private:
            just::avformat::FlvTag tag_;
            boost::uint32_t video_track_;
            boost::uint32_t audio_track_;
            bool is_save_sample_;
            just::demux::Sample sample_; // AggregateMessage
            typedef util::buffers::CycleBuffers<
                std::deque<boost::asio::const_buffer>, boost::uint8_t
            > cycle_buffer_t ;
            cycle_buffer_t buf_;
        };

    } // namespace rtmpc
} // namespace just

#endif // _JUST_RTMPC_RTM_FILTER_H_
