
#include "SysFsWatcher.hpp"

#include "Logging.hpp"

#include <boost/thread/lock_guard.hpp>

#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono_literals;

static SysFsWatcher* w;
SysFsWatcher* GetSysFsWatcher(boost::asio::io_context& io)
{
    if (w)
        return w;
    w = new SysFsWatcher(io);
    return w;
}

// FIXME: This should use BOOST asio, but it's missing proper documentation

void SysFsWatcher::Register(filesystem::path p,
                            std::function<void(filesystem::path)> handler)
{
    auto search = this->callbacks.find(p);
    if (search == this->callbacks.end())
    {
        this->Stop();
        this->callbacks[p] = handler;
        this->Start();
    }
}

void SysFsWatcher::Unregister(filesystem::path p)
{
    auto search = this->callbacks.find(p);
    if (search == callbacks.end())
    {
        this->Stop();
        this->callbacks.erase(p);
        this->Start();
    }
}

void SysFsWatcher::Stop(void)
{
    char dummy = 0;
    {
        boost::lock_guard<boost::mutex> lock(this->lock);
        if (this->controlFd > 0)
        {
            write(this->controlFd, &dummy, 1);
            this->runner->join();
            delete this->runner;
            this->runner = nullptr;
        }
    }
}

int SysFsWatcher::Start(void)
{
    boost::lock_guard<boost::mutex> lock(this->lock);
    int pipefd[2];

    if (pipe(pipefd))
    {
        return -1;
    }
    this->controlFd = pipefd[1];
    this->runner = new thread(&SysFsWatcher::Main, this, pipefd[0]);

    return 0;
}

int SysFsWatcher::Main(int ctrlFd)
{
    int fd, rv;
    char dummyData[1024] = {};
    const int n = this->callbacks.size() + 1;
    struct pollfd* ufds = new struct pollfd[n];

    ufds[0].fd = ctrlFd;
    ufds[0].events = POLLIN;
    ufds[0].revents = 0;

    int i = 1;
    for (auto const& x : this->callbacks)
    {
        // Open a connection to the attribute file.
        if ((fd = open(x.first.c_str(), O_RDONLY)) < 0)
        {
            return fd;
        }
        ufds[i].fd = fd;
        ufds[i].events = POLLPRI | POLLERR;
        ufds[i].revents = 0;
        i++;
        // Someone suggested dummy reads before the poll() call
        read(fd, dummyData, sizeof(dummyData));
    }

    while (1)
    {
        if ((rv = poll(ufds, n, -1)) < 0)
        {
            // error
            break;
        }
        else if (rv == 0)
        {
            // timeout
        }
        else if (ufds[0].revents & POLLIN)
        {
            break;
        }
        int i = 1;
        for (auto const& x : this->callbacks)
        {
            if (ufds[i].revents & POLLIN)
            {
                this->io->post([x] { x.second(x.first); });
                ufds[i].revents = 0;
            }
            i++;
        }
    }

    for (i = 0; i < this->callbacks.size() + 1; i++)
        close(ufds[i].fd);

    {
        boost::lock_guard<boost::mutex> lock(this->lock);
        close(this->controlFd);
        this->controlFd = -1;
    }
    delete ufds;

    return 0;
}

SysFsWatcher::SysFsWatcher(boost::asio::io_context& io) : io(&io), controlFd(-1)
{}

SysFsWatcher::~SysFsWatcher()
{
    this->Stop();
}