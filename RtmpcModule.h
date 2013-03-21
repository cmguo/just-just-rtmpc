// RtmpcModule.h

#ifndef _PPBOX_RTMPC_RTMPC_MODULE_H_
#define _PPBOX_RTMPC_RTMPC_MODULE_H_

namespace ppbox
{
    namespace rtmpc
    {

        class RtmpSession;
        class RtmpDispatcher;

        class RtmpcModule 
            : public ppbox::common::CommonModuleBase<RtmpcModule>
        {
        public:
            RtmpcModule(
                util::daemon::Daemon & daemon);

            virtual ~RtmpcModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();
        };

    } // namespace rtmpc
} // namespace ppbox

#endif // _PPBOX_RTMPC_RTMPC_MODULE_H_