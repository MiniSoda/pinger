#include <iostream>
#include "pinger_v6.hpp"

pinger_v6::pinger_v6(boost::asio::io_context& io_context, const char* destination)
    : pinger_base(io_context, destination, boost::asio::ip::icmp::icmp::v6())
    , m_resolver(io_context)
{

}

pinger_v6::~pinger_v6()
{

}

boost::asio::ip::icmp::endpoint pinger_v6::resolve(std::string const & destination)
{
  return *m_resolver.resolve(boost::asio::ip::icmp::v6(), destination, "").begin();
}

icmp_header pinger_v6::handle_payload(std::string const &body)
{
  // Create an ICMP header for an echo request.
  icmp_header echo_request;
  echo_request.type(icmp_header::echo_request_v6);
  echo_request.code(0);
  echo_request.identifier(get_identifier());
  echo_request.sequence_number(++m_sequence_number);
  compute_checksum(echo_request, body.begin(), body.end());
  return echo_request;
}

bool pinger_v6::handle_receive(std::size_t length)
{
    // The actual number of bytes received is committed to the buffer so that we
    // can extract it using a std::istream object.
    m_reply_buffer.commit(length);

    // Decode the reply packet.
    std::istream is(&m_reply_buffer);
    icmp_header icmp_hdr;
  
    //ipv6 raw socket does not support retrieving ipv6 header info
    //see http://manpages.ubuntu.com/manpages/hirsute/en/man4/icmp6.4freebsd.html
    is >> icmp_hdr;
    
    // We can receive all ICMP packets received by the host, so we need to
    // filter out only the echo replies that match the our identifier and
    // expected sequence number.
    if (is && icmp_hdr.type() == icmp_header::echo_reply_v6
          && icmp_hdr.identifier() == get_identifier()
          && icmp_hdr.sequence_number() == m_sequence_number)
    {
      // If this is the first reply, interrupt the five second timeout.
      if (m_num_replies++ == 0)
        m_timer.cancel();

      // Print out some information about the reply packet.
      chrono::steady_clock::time_point now = chrono::steady_clock::now();
      chrono::steady_clock::duration elapsed = now - m_time_sent;
      std::cout << "icmp_seq=" << icmp_hdr.sequence_number()
        << ", time="
        << chrono::duration_cast<chrono::milliseconds>(elapsed).count()
        << std::endl;
    }
  return true;
  }