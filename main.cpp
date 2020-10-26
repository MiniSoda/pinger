#include "pinger_v4.hpp"
#include "pinger_v6.hpp"

int main(int argc, char* argv[])
{
  try
  {
    if (argc != 2)
    {
    std::cerr << "Usage: ping <host>" << std::endl;
#if !defined(BOOST_ASIO_WINDOWS)
    std::cerr << "(You may need to run this program as root.)" << std::endl;
#endif
    return 1;
    }

    boost::asio::io_context io_context;
    pinger_v4 p(io_context, argv[1]);
    //pinger_v6 p(io_context, argv[1]);
    p.ping();
    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
}