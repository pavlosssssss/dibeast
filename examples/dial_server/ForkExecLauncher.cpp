#include "ForkExecLauncher.h"

#include <cerrno>
#include <csignal>
#include <fcntl.h>
#include <print>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>
#include <vector>

namespace {

std::string errorText(int error) {
  return std::system_category().message(error);
}

int exitCode(int status) {
  if (WIFEXITED(status))
    return WEXITSTATUS(status);

  if (WIFSIGNALED(status))
    return WTERMSIG(status);

  return -1;
}

}  // namespace

ForkExecLauncher::ForkExecLauncher(std::string executable)
    : executable_{std::move(executable)} {}

ForkExecLauncher::~ForkExecLauncher() {
  std::map<std::string, Child, std::less<>> children;
  {
    std::lock_guard lock{mutex_};

    for (const auto& [name, child] : children_)
      if (child.pid > 0)
        kill(child.pid, SIGKILL);

    children = std::move(children_);
  }
  // Joining the waiters happens here, outside the lock they need to finish.
}

dibeast::VoidResult ForkExecLauncher::launch(const LaunchRequest& request, ExitCallback onExit) {
  std::jthread finished;
  {
    std::lock_guard lock{mutex_};

    if (auto it = children_.find(request.appName); it != children_.end()) {
      if (it->second.pid > 0)
        return dibeast::Err(dibeast::ErrorCode::Internal, "'{}' is already running",
                            request.appName);

      finished = std::move(it->second.waiter);
      children_.erase(it);
    }
  }

  auto pid = spawn(request);

  if (!pid)
    return dibeast::Err(pid.error());

  std::println("fork-exec: '{}' started, pid {}", request.appName, *pid);

  std::lock_guard lock{mutex_};
  auto& child = children_[request.appName];
  child.pid = *pid;
  child.waiter = std::jthread{[this, name = request.appName, pid = *pid,
                               onExit = std::move(onExit)] { waitForExit(name, pid, onExit); }};

  return dibeast::Ok();
}

dibeast::VoidResult ForkExecLauncher::stop(std::string_view appName) {
  std::lock_guard lock{mutex_};
  auto it = children_.find(appName);

  if (it == children_.end() || it->second.pid <= 0)
    return dibeast::Err(dibeast::ErrorCode::NotFound, "'{}' is not running", appName);

  if (kill(it->second.pid, SIGTERM) != 0)
    return dibeast::Err(dibeast::ErrorCode::Internal, "kill '{}': {}", appName, errorText(errno));

  return dibeast::Ok();
}

dibeast::Result<pid_t> ForkExecLauncher::spawn(const LaunchRequest& request) {
  // Everything the child needs is prepared before fork(): after it, the child of a multi-threaded
  // process may only call async-signal-safe functions (no allocation) until execv.
  std::vector<std::string> args{executable_, "--dial-payload", request.payload,
                                "--additional-data-url", request.additionalDataUrl};
  std::vector<char*> argv;

  for (auto& arg : args)
    argv.push_back(arg.data());

  argv.push_back(nullptr);

  int execPipe[2];

  if (pipe2(execPipe, O_CLOEXEC) != 0)
    return dibeast::Err(dibeast::ErrorCode::ServiceUnavailable, "pipe: {}", errorText(errno));

  const pid_t pid = fork();

  if (pid < 0) {
    const int error = errno;
    close(execPipe[0]);
    close(execPipe[1]);

    return dibeast::Err(dibeast::ErrorCode::ServiceUnavailable, "fork: {}", errorText(error));
  }

  if (pid == 0) {
    close(execPipe[0]);
    close_range(3, ~0U, CLOSE_RANGE_CLOEXEC);
    execv(argv[0], argv.data());

    const int error = errno;
    [[maybe_unused]] auto written = write(execPipe[1], &error, sizeof error);
    _exit(127);
  }

  // The pipe closes on a successful exec (EOF); a failed exec sends errno through it first.
  close(execPipe[1]);
  int execError = 0;
  ssize_t bytes = 0;

  do {
    bytes = read(execPipe[0], &execError, sizeof execError);
  } while (bytes < 0 && errno == EINTR);

  close(execPipe[0]);

  if (bytes == sizeof execError) {
    waitpid(pid, nullptr, 0);

    return dibeast::Err(dibeast::ErrorCode::ServiceUnavailable, "execv '{}': {}", executable_,
                        errorText(execError));
  }

  return dibeast::Ok(pid);
}

void ForkExecLauncher::waitForExit(std::string name, pid_t pid, ExitCallback onExit) {
  int status = 0;

  while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
  }

  {
    std::lock_guard lock{mutex_};

    if (auto it = children_.find(name); it != children_.end() && it->second.pid == pid)
      it->second.pid = -1;
  }

  std::println("fork-exec: '{}' exited, code {}", name, exitCode(status));
  onExit(exitCode(status));
}
