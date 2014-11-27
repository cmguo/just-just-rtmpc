// RtmpcModule.cpp

#include "just/rtmpc/Common.h"
#include "just/rtmpc/RtmpcModule.h"
#include "just/rtmpc/ClassRegister.h"

namespace just
{
    namespace rtmpc
    {

        RtmpcModule::RtmpcModule(
            util::daemon::Daemon & daemon)
            : just::common::CommonModuleBase<RtmpcModule>(daemon, "RtmpcModule")
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
} // namespace just
