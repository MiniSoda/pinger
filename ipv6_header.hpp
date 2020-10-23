#pragma once

#include <algorithm>
#include <boost/asio/ip/address_v6.hpp>

// Packet header for ipv6.
//
// The wire format of an ipv6 header is:
// 
// 0      3            11          15           24                31
// +------+-------------+-----------------------------------------+      ---
// |versio| Traffic     |               Flow   label              |       ^
// |n     | Class       |                                         |       |
// +------+-------------+----------+-------------+----------------+       |
// |        Payload length         | Next header |   Hop Limit    |       |
// +-------------------------------+-------------+----------------+      40
// |                                                              |     bytes
// |                      source ipv6 address                     |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination ipv6 address                   |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
// |                                                              |       ^
// |                                                              |       |
// /                        options (if any)                      /    0 - 40
// /                                                              /     bytes
// |                                                              |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---

struct ipv6_frame
{
  uint32_t flow_label : 20;
  uint32_t traffic_class : 8;
  uint32_t version : 4;
  uint32_t hop_limit : 8;
  uint32_t next_header : 8;
  uint32_t payload_length : 16;
  uint8_t source_address[8];
  uint8_t destination_address[8];
  uint8_t payloads[40];
};

class ipv6_header
{
public:
  ipv6_header() : rep_({}){ }

  unsigned char version() const { return rep_.version; }
  unsigned short traffic_class() const { return rep_.traffic_class; }
  unsigned char flow_label() const { return rep_.flow_label; }
  unsigned short payload_length() const { return rep_.payload_length; }
  unsigned short next_header() const { return rep_.next_header; }
  unsigned short hop_limit() const { return rep_.hop_limit; }

  boost::asio::ip::address_v6 source_address() const
  {
    boost::asio::ip::address_v6::bytes_type bytes
      = { 
          {rep_.source_address[0], rep_.source_address[1], rep_.source_address[2], rep_.source_address[3]
          ,rep_.source_address[4], rep_.source_address[5], rep_.source_address[6], rep_.source_address[7]} 
        };
    return boost::asio::ip::address_v6(bytes);
  }

  boost::asio::ip::address_v6 destination_address() const
  {
    boost::asio::ip::address_v6::bytes_type bytes
      = { 
          {rep_.destination_address[0], rep_.destination_address[1], rep_.destination_address[2], rep_.destination_address[3]
          ,rep_.destination_address[4], rep_.destination_address[5], rep_.destination_address[6], rep_.destination_address[7]} 
        };
    return boost::asio::ip::address_v6(bytes);
  }

  friend std::istream& operator>>(std::istream& is, ipv6_header& header)
  {
    is.read(reinterpret_cast<char*>(&header.rep_), 40);
    if (header.version() != 4)
      is.setstate(std::ios::failbit);
    std::streamsize options_length = header.payload_length();
    if (options_length < 0 || options_length > 40)
      is.setstate(std::ios::failbit);
    else
      is.read(reinterpret_cast<char*>(&header.rep_) + 40, options_length);
    return is;
  }

private:
  ipv6_frame rep_;
};