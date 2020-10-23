#pragma once
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/streambuf.hpp>
#include <chrono>
#include <iostream>

#include "icmp_header.hpp"

using boost::asio::ip::icmp;
namespace chrono = std::chrono;

namespace
{
  auto const PING_TIMEOUT = 5;
  auto const MAX_LENGTH = 2048;
}

class pinger_base
{

private:
  std::string m_destination_string;
  icmp::endpoint m_destination;

protected:
  std::size_t m_num_replies;
  boost::asio::streambuf m_reply_buffer;
  boost::asio::steady_timer m_timer;
  chrono::steady_clock::time_point m_time_sent;
  icmp::socket m_socket;
  unsigned short m_sequence_number;

public:
  pinger_base(boost::asio::io_context& io_context, const char* destination, boost::asio::ip::icmp protocol_family)
    : m_destination_string(destination)
    , m_num_replies(0)
    , m_timer(io_context)
    , m_socket(io_context, protocol_family)
    , m_sequence_number(0)
  {
  
  }

  virtual ~pinger_base()
  {
  }

  void ping()
  {
    m_destination = resolve(m_destination_string);
    start_send();
    start_receive();
  }

private:
  void start_send()
  {
    boost::asio::streambuf request_buffer;
    std::string body("ping from network-daemon");

    icmp_header echo_request = handle_payload(body);

    // Encode the request packet.
    std::ostream os(&request_buffer);
    os << echo_request << body;
    //std::cout << echo_request << std::endl;

    m_socket.async_send_to(request_buffer.data(), m_destination, 
    [this](const boost::system::error_code& error, 
            std::size_t bytes_transferred)
    {
      if(error.value() == 0)
      {
        m_time_sent = std::chrono::steady_clock::now();
        //handling timeout
        m_timer.expires_after(chrono::seconds(PING_TIMEOUT));
        m_timer.async_wait([this](const boost::system::error_code error)
        {
          if (m_num_replies == 0)
            std::cout << "Request timed out" << std::endl;

          // Requests must be sent no less than one second apart.
          m_timer.expires_at(m_time_sent + chrono::seconds(1));
          m_timer.async_wait([this](const boost::system::error_code error) 
          {
            if (error == boost::asio::error::operation_aborted)
            {
              return;
            }
            m_num_replies--;
            if (m_num_replies > 0)
            {
              start_send();
            }
            else
            {
              //timeout 4 times consider to be fail
              m_timer.cancel();
            }
          });
        });
        //receive incoming relpies
        std::cout<< "start_receive" << std::endl;
        start_receive();
      }
    });
  }

  void start_receive()
  {
    // Discard any data already in the buffer.
    m_reply_buffer.consume(m_reply_buffer.size());

    // Wait for a reply. We prepare the buffer to receive up to 64KB.
    m_socket.async_receive_from(m_reply_buffer.prepare(65536), m_destination,
        [this](const boost::system::error_code& error, 
            std::size_t bytes_transferred)
        {
          if(error)
            std::cout<<"receive error: "<< error.message() << std::endl;
          handle_receive(bytes_transferred);
        });
  }

  virtual icmp::endpoint resolve(std::string const & destination) = 0;
  virtual icmp_header handle_payload(std::string const &body) = 0;
  virtual bool handle_receive(std::size_t length) = 0;

protected:
  static unsigned short get_identifier()
  {
    return static_cast<unsigned short>(::getpid());
  }
};