/*******************************************************************************
The MIT License (MIT)

Copyright (c) 2015 Dmitry "Dima" Korolev <dmitry.korolev@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

// Defines class `DemoServer`, which defines the endpoints and keeps the `State` of the demo.

#ifndef DEMO_DEMO_H
#define DEMO_DEMO_H

#include "../Bricks/dflags/dflags.h"

#include "uptime.h"
#include "state.h"

DECLARE_int32(demo_port);

namespace demo {

using bricks::FileSystem;
using bricks::net::api::HTTP;
using bricks::net::api::Request;
using bricks::net::HTTPResponseCode;
using namespace bricks::gnuplot;

struct DemoServer {
  DemoServer(int port = FLAGS_demo_port) : port_(port) {
    HTTP(port).Register("/ok", [](Request r) { r.connection.SendHTTPResponse("OK\n"); });
    HTTP(port).Register("/uptime", UptimeTracker());
    HTTP(port).Register("/", State::ClassBoundaries);
    FileSystem::ScanDir("static", [&port](const std::string& fn) {
      // Hack with string ownership.
      // TODO(dkorolev): Make this cleaner.
      std::string* ps = new std::string();
      *ps = FileSystem::ReadFileAsString("static/" + fn);
      HTTP(port).Register("/static/" + fn, [ps](Request r) {
        r.connection.SendHTTPResponse(*ps, HTTPResponseCode::OK, "text/plain");
      });
    });
    HTTP(port).Register("/demo_id", [this](Request r) { state_.DemoRequest(std::move(r)); });
  }

  void Join() {
    using bricks::net::api::HTTP;

    printf("Listening on port %d\n", port_);
    HTTP(port_).Join();
    printf("Done.\n");
  }

  State state_;
  const int port_;
};

}  // namespace demo

#endif  // DEMO_DEMO_H
