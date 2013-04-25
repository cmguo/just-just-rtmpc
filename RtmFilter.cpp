// RtmFilter.cpp

#include "ppbox/rtmpc/Common.h"
#include "ppbox/rtmpc/RtmFilter.h"

#include <ppbox/avformat/stream/SampleBuffers.h>
using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>
#include <util/protocol/rtmp/RtmpMessage.hpp>
#include <util/protocol/rtmp/RtmpMessageData.h>
using namespace util::protocol;

namespace ppbox
{
    namespace rtmpc
    {

        RtmFilter::RtmFilter()
            : video_track_(boost::uint32_t(-1))
            , audio_track_(boost::uint32_t(-1))
            , is_save_sample_(false)
        {
        }

        RtmFilter::~RtmFilter()
        {
        }

        bool RtmFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (is_save_sample_) {
                ec.clear();
                return get_aggregate_sample(sample, ec);
            }

            if (!Filter::get_sample(sample, ec))
                return false;

            RtmpMessageHeaderEx header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx const *>(sample.data.front());
            sample.data.front() = sample.data.front() + sizeof(header);
            sample.size -= sizeof(header);

            RtmpChunkHeader chunk;
            {
                util::archive::ArchiveBuffer<boost::uint8_t> buf(sample.data.front());
                RtmpMessageTraits::i_archive_t ia(buf);
                ia >> chunk;
                sample.data.front() = buf.data();
                sample.size -= ia.tellg();
            }

            SampleBuffers::ConstBuffers data;
            sample.data.swap(data);
            SampleBuffers::BuffersPosition beg(data.begin(), data.end());
            SampleBuffers::BuffersPosition end(data.end());

            boost::uint32_t message_length = header.length;
            while (message_length > header.chunk_size) {
                SampleBuffers::BuffersPosition pos(beg);
                beg.increment_bytes(end, header.chunk_size);
                sample.data.insert(sample.data.end(), SampleBuffers::range_buffers_begin(pos, beg), SampleBuffers::range_buffers_end());
                beg.increment_bytes(end, chunk.size());
                message_length -= header.chunk_size;
                sample.size -= chunk.size();
            }
            sample.data.insert(sample.data.end(), SampleBuffers::range_buffers_begin(beg, end), SampleBuffers::range_buffers_end());
            assert(sample.size == header.length);

            tag_.Type = header.type;
            tag_.DataSize = header.length;
            tag_.Timestamp = header.timestamp;
            tag_.StreamID = header.stream;

            if (sample.data.empty()) {

            } else {
                util::archive::ArchiveBuffer<boost::uint8_t> buf(sample.data.front());
                parse_sub_types(buf, sample, ec);
                sample.data.front() = buf.data();
            }

            if (is_save_sample_) {
                return get_aggregate_sample(sample, ec);
            }

            sample.dts = tag_.Timestamp;
            sample.duration = 0;
            sample.size = tag_.DataSize;
            sample.context = &tag_;

            return true;
        }

        bool RtmFilter::get_aggregate_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            RtmpMessageTraits::i_archive_t ia(buf_);
            ia >> (FlvTagHeader &)tag_;

            //sample.slice(sample_);
            parse_sub_types(buf_, sample, ec);

            sample.dts =  sample_.dts + tag_.Timestamp;
            sample.duration = 0;
            sample.size = tag_.DataSize;
            sample.context = &tag_;
            sample.data.clear();
            sample.data.insert(sample.data.end(), buf_.rbegin(sample.size), buf_.rend());
            buf_.consume(sample.size + 4); // 4 is the PreTagSize

            if (buf_.in_avail() == 0) {
                sample.memory = sample_.memory;
                sample_.memory = NULL;
                is_save_sample_ = false;
            }

            return true;
        }

        bool RtmFilter::get_next_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_next_sample(sample, ec))
                return false;

            RtmpMessageHeaderEx header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx const *>(sample.data.front());
            sample.dts = header.timestamp;
            return true;
        }

        bool RtmFilter::get_last_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
            if (!Filter::get_last_sample(sample, ec))
                return false;

            RtmpMessageHeaderEx header = 
                *boost::asio::buffer_cast<RtmpMessageHeaderEx const *>(sample.data.front());
            sample.dts = header.timestamp;
            return true;
        }

        bool RtmFilter::before_seek(
            Sample & sample,
            boost::system::error_code & ec)
        {
            is_save_sample_ = false;
            sample.append(sample_);
            return Filter::before_seek(sample, ec);
        }

        void RtmFilter::parse_sub_types(
            std::basic_streambuf<boost::uint8_t> & buf,
            Sample & sample,
            boost::system::error_code & ec)
        {
            RtmpMessageTraits::i_archive_t ia(buf);
            boost::uint64_t data_offset = ia.tellg();

            switch (tag_.Type) {
                case FlvTagType::AUDIO:
                    ia >> tag_.AudioHeader;
                    tag_.is_sample = tag_.AudioHeader.AACPacketType == 1;
                    tag_.is_sync = true;
                    tag_.cts_delta = 0;

                    if (audio_track_ == boost::uint32_t(-1))
                        audio_track_ = video_track_ + 1;
                    sample.itrack = audio_track_;
                    sample.flags = sample.f_sync;
                    if (tag_.AudioHeader.AACPacketType != 1)
                        sample.flags |= sample.f_config;
                    sample.cts_delta = 0;
                    break;
                case FlvTagType::VIDEO:
                    ia >> tag_.VideoHeader;
                    tag_.is_sample = tag_.VideoHeader.AVCPacketType == 1;
                    tag_.is_sync = tag_.VideoHeader.FrameType == FlvFrameType::KEY;
                    tag_.cts_delta = tag_.VideoHeader.CompositionTime;

                    if (video_track_ == boost::uint32_t(-1))
                        video_track_ = audio_track_ + 1;
                    sample.itrack = video_track_;
                    sample.flags = 0;
                    if (tag_.VideoHeader.FrameType == FlvFrameType::KEY)
                        sample.flags |= sample.f_sync;
                    if (tag_.VideoHeader.AVCPacketType != 1)
                        sample.flags |= sample.f_config;
                    sample.cts_delta = tag_.VideoHeader.CompositionTime;
                    //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
                    break;
                case RCMT_AggregateMessage:
                    sample_ = sample;
                    sample_.dts = tag_.Timestamp;
                    {
                        FlvTagHeader tag_header;
                        ia >> tag_header;
                        ia.seekg(data_offset, std::ios::beg);
                        sample_.dts -= tag_header.Timestamp;
                    }
                    buf_ = cycle_buffer_t(sample_.data);
                    buf_.commit(sample.size);
                    is_save_sample_ = true;
                    break;
                case FlvTagType::DATA:
                    ia >> tag_.DataTag;
                    tag_.is_sample = false;
                    tag_.is_sync = false;
                    tag_.cts_delta = 0;

                    sample.itrack = boost::uint32_t(-1);
                    sample.flags = 0;
                    sample.cts_delta = 0;
                    break;
                default:
                    sample.itrack = boost::uint32_t(-1);
                    sample.flags = 0;
                    sample.cts_delta = 0;
                    break;
            }

            if (ia) {
                tag_.data_offset = ia.tellg();
                tag_.DataSize = data_offset + tag_.DataSize - tag_.data_offset;
            } else {
                tag_.is_sample = false;
                sample.flags |= sample.f_config; // drop this sample
            }
        }

    } // namespace rtmpc
} // namespace ppbox
