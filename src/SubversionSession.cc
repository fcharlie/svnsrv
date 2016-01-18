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

void SubversionSession::stop() {
  boost::system::error_code ec;
  if (socket_.is_open()) {
    socket_.shutdown(tcp::socket::shutdown_both, ec);
    socket_.close(ec);
  }
  if (backend_.is_open()) {
    backend_.shutdown(tcp::socket::shutdown_both, ec);
    backend_.close(ec);
  }
}

void SubversionSession::start() {
  socket_.set_option(boost::asio::ip::tcp::no_delay(false));
  const char mbuffer[] =
      "( success ( 2 2 ( ) ( edit-pipeline svndiff1 absent-entries "
      "depth inherited-props log-revprops ) ) ) ";

  socket_.async_write_some(
      boost::asio::buffer(mbuffer, sizeof(mbuffer) - 1),
      boost::bind(&SubversionSession::echo_downstream_handshake,
                  shared_from_this(), boost::asio::placeholders::error,
                  boost::asio::placeholders::bytes_transferred));
}

void SubversionSession::sendError(int eid, const char *msg, size_t length) {
  auto self(shared_from_this());
  auto ml = snprintf(buffer_, length + 64,
                     "( failure ( ( %d %zu:%s 0: 0 ) ) ) ", eid, length, msg);
  socket_.async_write_some(
      boost::asio::buffer(buffer_, ml),
      [this, self](boost::system::error_code ec, std::size_t) {
        if (!ec) {
          boost::system::error_code ignored_ec;
          socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                           ignored_ec);
        }
        if (ec != boost::asio::error::operation_aborted) {
        }
      });
  // done sendError
}

void SubversionSession::echo_downstream_handshake(
    const boost::system::error_code &e, std::size_t bytes_transferred) {
  if (!e) {
    socket_.async_read_some(
        boost::asio::buffer(clt_buffer_),
        boost::bind(&SubversionSession::read_downstream_handshake,
                    shared_from_this(), boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    socket_.cancel();
  }
}
void SubversionSession::read_downstream_handshake(
    const boost::system::error_code &e, std::size_t bytes_transferred) {
  if (!e) {
    handshakeSize = bytes_transferred;
    if (!hinfo.Parse(clt_buffer_, handshakeSize)) {
      sendError(210004, hinfo.getLastErrorMessage().c_str(),
                hinfo.getLastErrorMessage().size());
      return;
    }
    auto svnurl = hinfo.getSubversionURL();

    if (!DiscoverStorageNode(svnurl, node) || node.address.empty()) {
      klogger::Log(klogger::kError,
                   "Cannot found Storage machine address, URL:%s",
                   hinfo.getBaseURL().c_str());
      const char msg[] = "Cannot found Storage machine address ";
      sendError(200042, msg, sizeof(msg) - 1);
      return;
    }

    klogger::Access("Storage: %s | URL: %s ", node.address.c_str(),
                    hinfo.getBaseURL().c_str());

    boost::system::error_code ec;
    tcp::endpoint upstream_point(
        boost::asio::ip::address::from_string(node.address, ec), node.port);
    if (ec) {
      const char msg[] = "Error upstream Address !";
      sendError(200042, msg, sizeof(msg) - 1);
      return;
    }
    backend_.async_connect(
        upstream_point,
        boost::bind(&SubversionSession::async_connect_upstream,
                    shared_from_this(), boost::asio::placeholders::error));
  } else {
    klogger::Log(klogger::kError,
                 "Read downstrean handshake message failure !");
    socket_.cancel();
  }
}
void SubversionSession::async_connect_upstream(
    const boost::system::error_code &e) {
  if (!e) {
    backend_.async_read_some(
        boost::asio::buffer(buffer_),
        boost::bind(&SubversionSession::read_upstream_handshake,
                    shared_from_this(), boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    klogger::Log(klogger::kError,
                 "Storage server is not available ,Address: %s",
                 node.address.c_str());
    sendError(200042, "Storage server is not available",
              sizeof("Storage server is not available") - 1);
  }
}
void SubversionSession::read_upstream_handshake(
    const boost::system::error_code &e, std::size_t bytes_transferred) {
  if (!e) {
    backend_.async_write_some(
        boost::asio::buffer(clt_buffer_, handshakeSize),
        boost::bind(&SubversionSession::echo_upstream_handshake,
                    shared_from_this(), boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    klogger::Log(klogger::kError, "Cannot read storage ,URL: %S ,Address: %s",
                 hinfo.getBaseURL().c_str(), node.address.c_str());
    sendError(200042, "Storage server is not available",
              sizeof("Storage server is not available") - 1);
  }
}
void SubversionSession::echo_upstream_handshake(
    const boost::system::error_code &e, std::size_t bytes_transferred) {
  if (!e) {
    backend_.async_read_some(
        boost::asio::buffer(buffer_),
        strand_.wrap(
            boost::bind(&SubversionSession::upstream_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
    socket_.async_read_some(
        boost::asio::buffer(clt_buffer_),
        strand_.wrap(
            boost::bind(&SubversionSession::downstream_read, shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
  } else {
    klogger::Log(klogger::kError,
                 "Echo handshake to storage failure ,URL: %s, Storage: %s",
                 hinfo.getBaseURL().c_str(), node.address.c_str());
    backend_.cancel();
    sendError(200042, "Echo handshake to storage failure",
              sizeof("Echo handshake to storage failure") - 1);
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
    socket_.cancel();
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
    backend_.cancel();
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
