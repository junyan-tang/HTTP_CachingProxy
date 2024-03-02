#include "proxy.hpp"
#include "request.hpp"

#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();

  if (pid < 0) {
    std::cerr << "Failed to fork process\n";
    return 1;
  }

  if (pid > 0) {
    return 0;
  }

  pid_t sid = setsid();
  if (sid < 0) {
    std::cerr << "Failed to create a new session\n";
    return 1;
  }

  pid = fork();
  if (pid < 0) {
    std::cerr << "Failed to fork process\n";
    return 1;
  }

  if (pid > 0) {
    return 0;
  }

  umask(0);

  if ((chdir("/")) < 0) {
    std::cerr << "Failed to change directory\n";
    return 1;
  }

  std::cout << "Daemon started with pid " << getpid() << std::endl;
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Daemon-specific initialization goes here
  try {
    boost::asio::io_context ioContext;
    ProxyServer server(ioContext, 12345);

    ioContext.run();
  } catch (std::exception &e) {
    // Log the exception
  }

  return 0;
}