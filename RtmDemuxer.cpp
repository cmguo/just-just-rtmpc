// RtmDemuxer.cpp

#include "just/rtmpc/Common.h"
#include "just/rtmpc/RtmDemuxer.h"
#include "just/rtmpc/RtmFilter.h"

#include <just/demux/basic/flv/FlvStream.h>
using namespace just::demux;

#include <just/avformat/flv/FlvTagType.h>
using namespace just::avformat;

#include <util/buffers/BuffersCopy.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.rtmpc.RtmDemuxer", framework::logger::Debug);

namespace just
{
    namespace rtmpc
    {

        RtmDemuxer::RtmDemuxer(
            boost::asio::io_service & io_svc, 
            just::data::PacketMedia & media)
            : just::demux::PacketDemuxer(io_svc, media)
            , filter_(NULL)
        {
        }

        RtmDemuxer::~RtmDemuxer()
        {
            if (filter_) {
                delete filter_;
            }
        }

        bool RtmDemuxer::check_open(
            boost::system::error_code & ec)
        {
            if (filter_ == NULL) {
                filter_ = new RtmFilter;
                add_filter(filter_);
            }

            Sample sample;
            while (peek_sample(sample, ec)) {
                FlvTag const * tag = (FlvTag const *)sample.context;
                if (sample.itrack == stream_infos_.size()) {
                    std::vector<boost::uint8_t> codec;
                    codec.resize(sample.size);
                    util::buffers::buffers_copy(boost::asio::buffer(codec), sample.data);
                    FlvStream info(*tag, codec, meta_);
                    info.index = sample.itrack;
                    info.start_time = 0;
                    stream_infos_.push_back(info);
                } else {
                    if (tag->Type == FlvTagType::DATA && tag->DataTag.Name == "onMetaData") {
                        meta_.from_data(tag->DataTag.Value);
                        if (meta_.duration != 0) {
                            media_info_.duration = meta_.duration;
                        }
                    }
                }
                if (!tag->is_sample) {
                    drop_sample(); // 清除没用的 sample
                }
                if (stream_infos_.size() == 2) {
                    return true;
                }
            }
            return false;
        }

    } // namespace rtmpc
} // namespace just
