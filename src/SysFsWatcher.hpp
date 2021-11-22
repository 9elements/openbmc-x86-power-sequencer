

#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/thread/mutex.hpp>

#include <filesystem>
#include <map>

using namespace std;

class SysFsWatcher
{
  public:
    SysFsWatcher(boost::asio::io_context& io);
    ~SysFsWatcher();
    int Main(int ctrlFd);
    bool IsRunning();
    int Start();
    void Stop();
    void Register(filesystem::path,
                  const std::function<void(filesystem::path)> handler);
    void Unregister(filesystem::path);

  private:
    int controlFd;
    std::thread* runner;
    boost::asio::io_context* io;
    boost::mutex lock;
    map<filesystem::path, std::function<void(filesystem::path)>> callbacks;
};

SysFsWatcher* GetSysFsWatcher(boost::asio::io_context& io);
