// #include "cache.hpp"
// #include "log.hpp"
#include "proxy.hpp"
#include "request.hpp"

int main() {
  try {
    boost::asio::io_context ioContext;
    ProxyServer server(ioContext, 12345);

    ioContext.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}