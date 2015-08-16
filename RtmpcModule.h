// RtmpcModule.h

#ifndef _JUST_RTMPC_RTMPC_MODULE_H_
#define _JUST_RTMPC_RTMPC_MODULE_H_

namespace just
{
    namespace rtmpc
    {

        class RtmpcModule 
            : public just::common::CommonModuleBase<RtmpcModule>
        {
        public:
            RtmpcModule(
                util::daemon::Daemon & daemon);

            virtual ~RtmpcModule();

        public:
            virtual bool startup(
                boost::system::error_code & ec);

            virtual bool shutdown(
                boost::system::error_code & ec);
        };

    } // namespace rtmpc
} // namespace just

#endif // _JUST_RTMPC_RTMPC_MODULE_H_
