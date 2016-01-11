/*
* SubversionSession.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <string>
#include <array>
#include <memory>
#include <functional>
#include <algorithm>
#include <iostream>
#include <string.h>
#include <iostream>
#include "svnsrv.h"
#include "URLTokenizer.hpp"
#include "SubversionSyntactic.hpp"
#include "SubversionStorage.hpp"
#include "SubversionSession.hpp"
#include "klog.h"
/**
*    origin client <---> svnsrv <---> sserver
step 1.   origin  receive server capacity
step 2.   origin send request header to  svnsrv
step 3.   svnsrv parse request header,cache this header ,get domain,repository
owner user, query ip
step 4.   svnsrv create socket use this ip
step 5.    svnsrv receive sserver capacity, discard
step 6.    svnsrv send cached header, send to sserver,
step 7.    svnsrv transfer begin.
*
**/

SubversionSession::SubversionSession(boost::asio::io_service &io_service)
    : strand_(io_service), socket_(io_service), backend_(io_service) {
  /////
}

void SubversionSession::start() {
  socket_.set_option(boost::asio::ip::tcp::no_delay(false));
  const char mbuffer[] =
      "( success ( 2 2 ( ) ( edit-pipeline svndiff1 absent-entries "
      "depth inherited-props log-revprops ) ) ) ";
  boost::system::error_code e;
  std::string addr = socket_.remote_endpoint().address().to_string();
  klogger::Log(klogger::kInfo, "New Connection,Remote: %s", addr.c_str());
  socket_.write_some(boost::asio::buffer(mbuffer, sizeof(mbuffer) - 1), e);
  if (e) {
    klogger::Log(klogger::kError,
                 "Write handshake message to downstream fatal,Remote: %s",
                 addr.c_str());
    return;
  }
  /////////////////////////////////////////////////////// sendError
  auto sendError = [&](int eid, const char *msg, size_t length) -> bool {
    auto ml = snprintf(buffer_, length + 64,
                       "( failure ( ( %d %zu:%s 0: 0 ) ) ) ", eid, length, msg);
    socket_.write_some(boost::asio::buffer(buffer_, ml), e);
    return !e;
  };
  auto greetingLength = socket_.read_some(boost::asio::buffer(clt_buffer_), e);
  if (e) {
    klogger::Log(klogger::kError,
                 "Read handshake message from downstream failed,Remote: %s",
                 addr.c_str());
    return;
  }
  ExchangeCapabilities eca;
  if (!eca.Parse(clt_buffer_, greetingLength)) {
    klogger::Log(klogger::kError, "Bad Network data: %s Remote: %s",
                 eca.getLastErrorMessage().c_str(), addr.c_str());
    sendError(210004, eca.getLastErrorMessage().c_str(),
              eca.getLastErrorMessage().size());
    return;
  }
  auto us = eca.getSubversionURL();
  SubversionStorageNode node;
  if (!DiscoverStorageNode(us, node)) {
    klogger::Log(klogger::kError,
                 "Unable to get storage machine address, URL %s",
                 us.origin.c_str());
    const char msg[] =
        "Unable to get storage machine address, maybe you typed the wrong URL";
    sendError(200042, msg, sizeof(msg) - 1);
    return;
  }
  klogger::Access("Remote: %s Storage: %s | URL %s ",
                  socket_.remote_endpoint().address().to_string().c_str(),
                  node.address.c_str(), eca.getBaseURL().c_str());
  if (node.address.empty())
    return;
  tcp::endpoint backend_point(
      boost::asio::ip::address::from_string(node.address), node.port);
  backend_.connect(backend_point, e);
  if (e) {
    klogger::Log(klogger::kError, "Storage is not available,address: %s",
                 node.address.c_str());
    sendError(200042, "Storage is not available",
              sizeof("Storage is not available") - 1);
    return;
  }

  backend_.read_some(boost::asio::buffer(buffer_), e);
  if (e) {
    return;
  }
  backend_.write_some(boost::asio::buffer(clt_buffer_, greetingLength), e);
  if (e) {
    return;
  }
  if (e)
    return;
  backend_.async_read_some(
      boost::asio::buffer(buffer_),
      strand_.wrap(boost::bind(&SubversionSession::upstream_read,
                               shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)));
  socket_.async_read_some(
      boost::asio::buffer(clt_buffer_),
      strand_.wrap(boost::bind(&SubversionSession::downstream_read,
                               shared_from_this(),
                               boost::asio::placeholders::error,
                               boost::asio::placeholders::bytes_transferred)));
}

void SubversionSession::stop() {
  if (socket_.is_open()) {
    socket_.close();
  }
  if (backend_.is_open()) {
    backend_.close();
  }
}

void SubversionSession::upstream_read(const boost::system::error_code &e,
                                      std::size_t bytes_transferred) {
  if (!e) {
    boost::asio::async_write(
        socket_, boost::asio::buffer(buffer_, bytes_transferred),
        strand_.wrap(boost::bind(&SubversionSession::downstream_write,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
  } else {
    socket_.shutdown(tcp::socket::shutdown_both);
  }
}

void SubversionSession::downstream_write(const boost::system::error_code &e) {
  if (!e) {
    backend_.async_read_some(
        boost::asio::buffer(buffer_),
        strand_.wrap(
            boost::bind(&SubversionSession::upstream_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
  }
}

void SubversionSession::downstream_read(const boost::system::error_code &e,
                                        std::size_t bytes_transferred) {
  if (!e) {
    boost::asio::async_write(
        backend_, boost::asio::buffer(clt_buffer_, bytes_transferred),
        strand_.wrap(boost::bind(&SubversionSession::upstream_write,
                                 shared_from_this(),
                                 boost::asio::placeholders::error)));
  } else {
    backend_.shutdown(tcp::socket::shutdown_both);
  }
}
void SubversionSession::upstream_write(const boost::system::error_code &e) {
  if (!e) {
    socket_.async_read_some(
        boost::asio::buffer(clt_buffer_),
        strand_.wrap(
            boost::bind(&SubversionSession::downstream_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
  }
}
