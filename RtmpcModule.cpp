// RtmpcModule.cpp

#include "ppbox/rtmpc/Common.h"
#include "ppbox/rtmpc/RtmpcModule.h"
#define PPBOX_ENABLE_REGISTER_CLASS
#include "ppbox/rtmpc/RtmpMedia.h"
#include "ppbox/rtmpc/RtmDemuxer.h"

namespace ppbox
{
    namespace rtmpc
    {

        RtmpcModule::RtmpcModule(
            util::daemon::Daemon & daemon)
            : ppbox::common::CommonModuleBase<RtmpcModule>(daemon, "RtmpcModule")
        {
        }

        RtmpcModule::~RtmpcModule()
        {
        }

        boost::system::error_code RtmpcModule::startup()
        {
            boost::system::error_code ec;
            return ec;
        }

        void RtmpcModule::shutdown()
        {
        }

    } // namespace rtmpc
} // namespace ppbox
