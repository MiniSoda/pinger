#pragma once

#include <algorithm>
#include <boost/asio/ip/address_v4.hpp>

// Packet header for IPv4.
//
// The wire format of an IPv4 header is:
// 
// 0               8               16                             31
// +-------+-------+---------------+------------------------------+      ---
// |       |       |               |                              |       ^
// |version|header |    type of    |    total length in bytes     |       |
// |  (4)  | length|    service    |                              |       |
// +-------+-------+---------------+-+-+-+------------------------+       |
// |                               | | | |                        |       |
// |        identification         |0|D|M|    fragment offset     |       |
// |                               | |F|F|                        |       |
// +---------------+---------------+-+-+-+------------------------+       |
// |               |               |                              |       |
// | time to live  |   protocol    |       header checksum        |   20 bytes
// |               |               |                              |       |
// +---------------+---------------+------------------------------+       |
// |                                                              |       |
// |                      source IPv4 address                     |       |
// |                                                              |       |
// +--------------------------------------------------------------+       |
// |                                                              |       |
// |                   destination IPv4 address                   |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---
// |                                                              |       ^
// |                                                              |       |
// /                        options (if any)                      /    0 - 40
// /                                                              /     bytes
// |                                                              |       |
// |                                                              |       v
// +--------------------------------------------------------------+      ---

struct ipv4_frame
{
  uint8_t header_length : 4;
  uint8_t version : 4;
  uint8_t type_of_service;
  uint16_t total_length;
  uint16_t identification;
  uint16_t flags_and_offset;
  uint8_t time_to_live;
  uint8_t protocol;
  uint16_t header_checksum;
  uint8_t source_address[4];
  uint8_t destination_address[4];
  uint8_t payloads[40];
};

class ipv4_header
{
public:
  ipv4_header() : rep_({}){ }

  unsigned char version() const { return rep_.version; }
  unsigned short header_length() const { return rep_.header_length * 4; }
  unsigned char type_of_service() const { return rep_.type_of_service; }
  unsigned short total_length() const { return rep_.type_of_service; }
  unsigned short identification() const { return rep_.identification; }
  unsigned short fragment_offset() const { return rep_.flags_and_offset; }
  unsigned int time_to_live() const { return rep_.time_to_live; }
  unsigned char protocol() const { return rep_.protocol; }
  unsigned short header_checksum() const { return rep_.header_checksum; }

  boost::asio::ip::address_v4 source_address() const
  {
    boost::asio::ip::address_v4::bytes_type bytes
      = { {rep_.source_address[0], rep_.source_address[1], rep_.source_address[2], rep_.source_address[3]} };
    return boost::asio::ip::address_v4(bytes);
  }

  boost::asio::ip::address_v4 destination_address() const
  {
    boost::asio::ip::address_v4::bytes_type bytes
      = { {rep_.destination_address[0], rep_.destination_address[1], rep_.destination_address[2], rep_.destination_address[3]} };
    return boost::asio::ip::address_v4(bytes);
  }

  friend std::istream& operator>>(std::istream& is, ipv4_header& header)
  {
    is.read(reinterpret_cast<char*>(&header.rep_), 20);
    if (header.version() != 4)
      is.setstate(std::ios::failbit);
    std::streamsize options_length = header.header_length() - 20;
    if (options_length < 0 || options_length > 40)
      is.setstate(std::ios::failbit);
    else
      is.read(reinterpret_cast<char*>(&header.rep_) + 20, options_length);
    return is;
  }

  friend std::ostream& operator<<(std::ostream& os, ipv4_header& header)
  {
    os << "ipv4_header: version " << int(header.version()) 
        <<" ,header length " << int(header.header_length())
        <<" ,type of service " << int(header.type_of_service())
        <<" ,total length in bytes " << int(header.total_length())
        <<" ,identification " << int(header.identification())
        <<" ,time to live " << int(header.time_to_live())
        <<" ,protocol " << int(header.protocol())
        <<" ,checksum " << int(header.header_checksum())
        <<" ,src address " << header.source_address().to_string()
        <<" ,dst address " << header.destination_address().to_string();
    return os;
  }

private:
  ipv4_frame rep_;
};