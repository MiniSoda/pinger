#pragma once

#include <istream>
#include <ostream>
#include <algorithm>

// ICMP header for both IPv4 and IPv6.
//
// The wire format of an ICMP header is:
// 
// 0               8               16                             31
// +---------------+---------------+------------------------------+      ---
// |               |               |                              |       ^
// |     type      |     code      |          checksum            |       |
// |               |               |                              |       |
// +---------------+---------------+------------------------------+    8 bytes
// |                               |                              |       |
// |          identifier           |       sequence number        |       |
// |                               |                              |       v
// +-------------------------------+------------------------------+      ---
struct icmp_frame
{
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint16_t identifier;
  uint16_t sequence_number;
};

class icmp_header
{
public:
  enum { echo_reply = 0, destination_unreachable = 3, source_quench = 4,
    redirect = 5, echo_request = 8, time_exceeded = 11, parameter_problem = 12,
    timestamp_request = 13, timestamp_reply = 14, info_request = 15,
    info_reply = 16, address_request = 17, address_reply = 18,
       echo_request_v6 = 128, echo_reply_v6 = 129};

  icmp_header() : rep_({}){  }

  unsigned char type() const { return rep_.type; }
  unsigned char code() const { return rep_.code; }
  unsigned short checksum() const { return ::ntohs(rep_.checksum); }
  unsigned short identifier() const { return ::ntohs(rep_.identifier); }
  unsigned short sequence_number() const { return ::ntohs(rep_.sequence_number); }

  void type(unsigned char n) { rep_.type = n; }
  void code(unsigned char n) { rep_.code = n; }
  void checksum(unsigned short n) { rep_.checksum = ::htons(n); }
  void identifier(unsigned short n) { rep_.identifier = ::htons(n); }
  void sequence_number(unsigned short n) { rep_.sequence_number = ::htons(n); }

  friend std::istream& operator>>(std::istream& is, icmp_header& header)
    { return is.read(reinterpret_cast<char*>(&header.rep_), 8); }

  friend std::ostream& operator<<(std::ostream& os, const icmp_header& header)
    { return os.write(reinterpret_cast<const char*>(&header.rep_), 8); }

private:
  icmp_frame rep_;
};

template <typename Iterator>
void compute_checksum(icmp_header& header,
    Iterator body_begin, Iterator body_end)
{
  unsigned int sum = (header.type() << 8) + header.code()
    + header.identifier() + header.sequence_number();

  Iterator body_iter = body_begin;
  while (body_iter != body_end)
  {
    sum += (static_cast<unsigned char>(*body_iter++) << 8);
    if (body_iter != body_end)
      sum += static_cast<unsigned char>(*body_iter++);
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  header.checksum(static_cast<unsigned short>(~sum));
}