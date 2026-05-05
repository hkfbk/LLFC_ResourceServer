// ResourceServer.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>

// TODO: 在此处引用程序需要的其他标头。
#include <boost/asio/version.hpp>
#include "global.hpp"
#include <filesystem>
#include <toml++/toml.hpp>
#include <spdlog/spdlog.h>
#include "singleton.hpp"
#include "cserver.h"
#include "ioservicepool.h"
#include "boost/asio/signal_set.hpp"
namespace asio = boost::asio;
