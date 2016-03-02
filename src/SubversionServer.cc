/*
* SubversionServer.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <array>
#include <memory>
#include <utility>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <boost/thread.hpp>
#include "svnsrv.h"
#include "Runtime.hpp"
#include "SubversionServer.hpp"
#include "SubversionSession.hpp"
#include "klog.h"

class SubversionServer : private boost::noncopyable {
public:
  SubversionServer(const SubversionServer &) = delete;
  SubversionServer &operator=(const SubversionServer &) = delete;
  explicit SubversionServer(const NetworkServerArgs &networkArgs)
      : thread_pool_size_(networkArgs.poolSize), acceptor_(io_service_),
        new_connection_() {
    if (thread_pool_size_ == 0) {
      klogger::Log(klogger::kFatal, "io_service_pool size is 0");
      throw std::runtime_error("io_service_pool size is 0");
    }
    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(networkArgs.address),
        networkArgs.port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    start_accept();
  }
  void run() {
    std::vector<std::shared_ptr<boost::thread>> threads;
    for (std::size_t i = 0; i < thread_pool_size_; ++i) {
      std::shared_ptr<boost::thread> thread(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &io_service_)));
      threads.push_back(thread);
    }
    for (std::size_t i = 0; i < threads.size(); ++i)
      threads[i]->join();
  }

private:
  void start_accept() {
    new_connection_.reset(new SubversionSession(io_service_));
    acceptor_.async_accept(new_connection_->socket(),
                           boost::bind(&SubversionServer::handle_accept, this,
                                       boost::asio::placeholders::error));
  }
  void handle_accept(const boost::system::error_code &e) {
    if (!e) {
      new_connection_->start();
    }
    start_accept();
  }
  void handle_stop() { io_service_.stop(); }
  std::size_t thread_pool_size_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;
  SubversionSessionPtr new_connection_;
};

int SubversionServerInitialize(const NetworkServerArgs &networkArgs) {
  klogger::Log(klogger::kInfo, "svnsrv running");
  SubversionServer subversionServer(networkArgs);
  klogger::Log(klogger::kInfo, "Listener address: %s:%d Thread counts: %d",
               networkArgs.address.c_str(), networkArgs.port,
               networkArgs.poolSize);
  subversionServer.run();
  klogger::Log(klogger::kInfo, "svnsrv shutdown");
  return 0;
}
