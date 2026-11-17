#include "Connection.hpp"
#include <cstring>

namespace JE
{
    namespace Net
    {
        void Connection::Reset()
        {
            state = 0;
        }
    } // namespace Net
} // namespace JE
