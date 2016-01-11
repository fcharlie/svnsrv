/*
* SubversionSession.hpp
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#ifndef SVNSRV_SUBVERSION_SESSION_HPP
#define SVNSRV_SUBVERSION_SESSION_HPP
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/noncopyable.hpp>
using boost::asio::ip::tcp;

class SubversionSession
    : public std::enable_shared_from_this<SubversionSession> {
protected:
  enum {
    kSmallBufferSize = 4096,
    kDefaultBufferSize = 32 * 1024,
    kMaxBufferSize = 10 * 1024 * 1024
  };

public:
  explicit SubversionSession(boost::asio::io_service &io_service);
  tcp::socket &socket() { return socket_; }
  void start();
  void stop();

private:
  void downstream_read(const boost::system::error_code &e,
                       std::size_t bytes_transferred);
  void downstream_write(const boost::system::error_code &e);
  void upstream_read(const boost::system::error_code &e,
                     std::size_t bytes_transferred);
  void upstream_write(const boost::system::error_code &e);
  boost::asio::io_service::strand strand_;
  tcp::socket socket_;
  tcp::socket backend_;
  char buffer_[kDefaultBufferSize];
  char clt_buffer_[kDefaultBufferSize];
};

typedef std::shared_ptr<SubversionSession> SubversionSessionPtr;

#endif
