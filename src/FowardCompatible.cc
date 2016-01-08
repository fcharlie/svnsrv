/*
* FowardCompatible.cc
* oschina.net subversion proxy service
* author: Force.Charlie
* Date: 2015.12
* Copyright (C) 2016. OSChina.NET. All Rights Reserved.
*/
#include <string>
#include <vector>
#include <iostream>
#include "cpptoml.h"
#include "FowardCompatible.hpp"

static bool RangeToIndex(const char *range, uint16_t &l, uint16_t &r) {
  uint8_t in[8] = "--zz";
  uint8_t *p = in;
  int i = 0;
  bool isSplit = false;
  if (range) {
    for (; range[i] && i < 5; i++) {
      if (range[i] == '~') {
        p = in + 2;
        isSplit = true;
      } else {
        *p++ = range[i];
      }
    }
    // when No Tilde like A,Aa
    if (!isSplit) {
      if (i == 1) {
        in[2] = range[0];
      } else if (i == 2) {
        in[2] = range[0];
        in[3] = range[1];
      }
    }
    l = (uint16_t)in[0] * 0x200 + in[1];
    r = (uint16_t)in[2] * 0x200 + in[3];
    return true;
  }
  return false;
}

bool FowardDiscoverManager::InitializeManager(const char *tableFile) {
  std::shared_ptr<cpptoml::table> g;
  try {
    g = cpptoml::parse_file(tableFile);
  } catch (const cpptoml::parse_exception &e) {
    fprintf(stderr, "Failure, Parser router.toml failed: %s \n", e.what());
    return false;
  }
  if (g->contains_qualified("Host.address")) {
    defaultElement_ =
        g->get_qualified("Host.address")->as<std::string>()->get();
  } else {
    defaultElement_ = "192.168.1.12";
  }

  if (g->contains_qualified("Host.port")) {
    auto i64 = g->get_qualified("Host.port")->as<int64_t>()->get();
    hostPort = static_cast<int>(i64);
  } else {
    hostPort = 3690;
  }

  auto ta = g->get_table_array_qualified("Host.Content");
  if (ta == nullptr) {
    fprintf(stderr, "Cannot found any Host.Content from router.toml\n");
    return false;
  }
  for (auto t : *ta) {
    if (!t->contains_qualified("range"))
      continue;
    if (!t->contains_qualified("address"))
      continue;
    HostElement e;
    auto range = t->get_qualified("range")->as<std::string>()->get();
    RangeToIndex(range.c_str(), e.begin, e.end);
    e.address = t->get_qualified("address")->as<std::string>()->get();
    if (t->contains_qualified("enable")) {
      e.IsEnabled = t->get_qualified("enable")->as<bool>()->get();
    } else {
      e.IsEnabled = true;
    }
    hostElement_.push_back(std::move(e));
  }
  return true;
}

bool FowardDiscoverManager::GetAddress(const std::string &owner,
                                       std::string &address) {
  if (owner.size() < 2)
    return false;
  uint16_t index = (uint16_t)owner[0] * 0x200 + owner[1];
  for (auto &e : hostElement_) {
    if (index >= e.begin && index <= e.end) {
      if (e.IsEnabled) {
        address = e.address;
        return true;
      }
    }
  }
  address = defaultElement_;
  return true;
}

FowardDiscoverManager fowardDiscoverManager;
