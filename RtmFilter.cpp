// RtmFilter.cpp

#include "ppbox/rtmpc/Common.h"
#include "ppbox/rtmpc/RtmFilter.h"

#include <ppbox/avformat/stream/SampleBuffers.h>
using namespace ppbox::avformat;

#include <util/archive/ArchiveBuffer.h>
#include <util/protocol/rtmp/RtmpMessage.hpp>
using namespace util::protocol;

namespace ppbox
{
    namespace rtmpc
    {

        RtmFilter::RtmFilter()
            : video_track_(boost::uint32_t(-1))
            , audio_track_(boost::uint32_t(-1))
        {
        }

        RtmFilter::~RtmFilter()
        {
        }

        bool RtmFilter::get_sample(
            Sample & sample,
            boost::system::error_code & ec)
        {
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

            while (header.length > header.chunk_size) {
                SampleBuffers::BuffersPosition pos(beg);
                beg.increment_bytes(end, header.chunk_size);
                sample.data.insert(sample.data.end(), SampleBuffers::range_buffers_begin(pos, beg), SampleBuffers::range_buffers_end());
                beg.increment_bytes(end, chunk.size());
                header.length -= header.chunk_size;
                sample.size -= chunk.size();
            }
            sample.data.insert(sample.data.end(), SampleBuffers::range_buffers_begin(beg, end), SampleBuffers::range_buffers_end());

            tag_.Type = header.type;
            tag_.DataSize = header.length;
            tag_.Timestamp = header.timestamp;
            tag_.StreamID = header.stream;

            sample.dts = tag_.Timestamp;
            sample.duration = 0;

            {
                util::archive::ArchiveBuffer<boost::uint8_t> buf(sample.data.front());
                RtmpMessageTraits::i_archive_t ia(buf);
                switch (tag_.Type) {
                    case FlvTagType::AUDIO:
                        ia >> tag_.AudioHeader;
                        if (audio_track_ == boost::uint32_t(-1))
                            audio_track_ = video_track_ + 1;
                        sample.itrack = audio_track_;
                        sample.flags = sample.sync;
                        if (tag_.AudioHeader.AACPacketType != 1)
                            sample.flags |= sample.config;
                        sample.cts_delta = 0;
                        break;
                    case FlvTagType::VIDEO:
                        ia >> tag_.VideoHeader;
                        if (video_track_ == boost::uint32_t(-1))
                            video_track_ = audio_track_ + 1;
                        sample.itrack = video_track_;
                        sample.flags = 0;
                        if (tag_.VideoHeader.FrameType == FlvFrameType::KEY)
                            sample.flags |= sample.sync;
                        if (tag_.VideoHeader.AVCPacketType != 1)
                            sample.flags |= sample.config;
                        sample.cts_delta = tag_.VideoHeader.CompositionTime;
                        //std::cout << "sample track = " << sample.itrack << ", dts = " << sample.dts << ", cts_delta = " << sample.cts_delta << std::endl;
                        break;
                    case FlvTagType::DATA:
                        ia >> tag_.DataTag;
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
                sample.data.front() = buf.data();
                sample.size -= ia.tellg();
                sample.context = &tag_;
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

    } // namespace mux
} // namespace ppbox
