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

        bool RtmpcModule::startup(
           boost::system::error_code & ec)
        {
            return true;
        }

        bool RtmpcModule::shutdown(
           boost::system::error_code & ec)
        {
            return true;
        }

    } // namespace rtmpc
} // namespace just
