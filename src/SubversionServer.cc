/*
* SubversionServer.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.11
* Copyright (C) 2015. OSChina.NET. All Rights Reserved.
*/
#include <cstdio>
#include <cstdlib>
#include <array>
#include <memory>
#include <utility>
#include <thread>
#include <mutex>
#include "svnsrv.h"
#include "SubversionServer.hpp"
#include "SubversionSession.hpp"
#include "klog.h"

class io_service_pool : public boost::noncopyable {
public:
  explicit io_service_pool(std::size_t pool_size = (std::max)(
                               2u, std::thread::hardware_concurrency() * 2))
      : next_io_service_(0) {
    // printf("Initialize io_service_pool, pool size: %lu\n",pool_size);
    if (pool_size == 0)
      throw std::runtime_error("io_service_pool size is 0");
    for (std::size_t i = 0; i < pool_size; ++i) {
      io_service_ptr io_service(new boost::asio::io_service);
      work_ptr work(new boost::asio::io_service::work(*io_service));
      io_services_.push_back(io_service);
      work_.push_back(work);
    }
  }
  void run() {
    std::vector<std::shared_ptr<std::thread>> threads;
    for (auto &it : io_services_) {
      std::shared_ptr<std::thread> thread(
          new std::thread(boost::bind(&boost::asio::io_service::run, it)));
      threads.push_back(thread);
    }
    for (auto &t : threads) {
      t->join();
    }
  }
  void stop() {
    for (auto &i : io_services_) {
      i->stop();
    }
  }

  boost::asio::io_service &get_io_service() {
    // printf("get io service\n");
    std::unique_lock<std::mutex> lock(mtx);
    boost::asio::io_service &io_service = *io_services_[next_io_service_];
    ++next_io_service_;
    if (next_io_service_ == io_services_.size()) {
      next_io_service_ = 0;
    }
    return io_service;
  }

private:
  typedef std::shared_ptr<boost::asio::io_service> io_service_ptr;
  typedef std::shared_ptr<boost::asio::io_service::work> work_ptr;
  typedef std::shared_ptr<std::thread> thread_ptr;
  std::mutex mtx;
  std::vector<io_service_ptr> io_services_;
  std::vector<work_ptr> work_;
  std::size_t next_io_service_;
};

/**
* class SubversionServer
**/
class SubversionServer {
public:
  explicit SubversionServer(const NetworkServerArgs &networkArgs)
      : io_service_pool_(networkArgs.poolSize),
        acceptor_(io_service_pool_.get_io_service()) {
    boost::asio::ip::tcp::endpoint endpoint(
        boost::asio::ip::address::from_string(networkArgs.address),
        networkArgs.port);
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();
    start_accept();
  }
  void run() { io_service_pool_.run(); }

private:
  void start_accept() {
    new_session_.reset(
        new SubversionSession(io_service_pool_.get_io_service()));
    acceptor_.async_accept(new_session_->socket(),
                           boost::bind(&SubversionServer::handle_accept, this,
                                       boost::asio::placeholders::error));
  }
  void handle_accept(const boost::system::error_code &e) {
    if (!e) {
      new_session_->start();
      klogger::FileFlush();
      start_accept();
      // new_session_.reset(new
      // SubversionSession(io_service_pool_.get_io_service()));
    }
  }
  io_service_pool io_service_pool_;
  SubversionSessionPtr new_session_;
  tcp::acceptor acceptor_;
};

int SubversionServerInitialize(const NetworkServerArgs &networkArgs) {
  klogger::Log(klogger::kInfo, "Subversion Server Running");
  SubversionServer subversionServer(networkArgs);
  subversionServer.run();
  klogger::Log(klogger::kInfo, "Subversion Server shutdown");
  return 0;
}
