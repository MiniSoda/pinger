#pragma once
#include <string>
#include <memory>

#include "pinger.hpp"
#include "icmp_header.hpp"

class pinger_v6 : public pinger_base
{
public:
  pinger_v6(boost::asio::io_context& io_context, const char* destination);
  ~pinger_v6();

private:
  boost::asio::ip::icmp::resolver m_resolver;
  boost::asio::ip::icmp::endpoint resolve(std::string const & destination) override;
  icmp_header handle_payload(std::string const &body) override;
  bool handle_receive(std::size_t length) override;
};