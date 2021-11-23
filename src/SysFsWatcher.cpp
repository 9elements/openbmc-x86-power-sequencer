
#include "SysFsWatcher.hpp"

#include "Logging.hpp"

#include <boost/thread/lock_guard.hpp>

#include <chrono>
#include <iostream>
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

void SysFsWatcher::Register(
    filesystem::path p,
    std::function<void(filesystem::path, const char*)> handler)
{
    auto search = this->callbacks.find(p);
    if (search == this->callbacks.end())
    {
        this->Stop();
        {
            this->callbacks[p] = handler;
        }
        this->Start();
    }
}

void SysFsWatcher::Unregister(filesystem::path p)
{
    auto search = this->callbacks.find(p);
    if (search == callbacks.end())
    {
        this->Stop();
        {
            this->callbacks.erase(p);
        }
        this->Start();
    }
}

void SysFsWatcher::Stop(void)
{
    char dummy = 0;

    if (this->controlFd > 0)
    {
        write(this->controlFd, &dummy, 1);
        this->runner->join();
        delete this->runner;
        this->runner = nullptr;
        close(this->controlFd);
        this->controlFd = -1;
    }
}

int SysFsWatcher::Start(void)
{
    int pipefd[2];

    if (pipe(pipefd))
    {
        return -1;
    }
    this->controlFd = pipefd[1];
    this->runner = new thread(&SysFsWatcher::Main, this, pipefd[0]);

    return 0;
}

// FIXME: Keep fds open on remove/insertion
int SysFsWatcher::Main(int ctrlFd)
{
    map<int, SysFsEvent> events;
    SysFsEvent event;
    const int n = this->callbacks.size() + 1;
    struct pollfd* ufds = new struct pollfd[n];
    char dummyData[1024] = {};
    int fd, rv, i;

    LOGDEBUG("Starting SysFS watcher thread");

    ufds[0].fd = ctrlFd;
    ufds[0].events = POLLIN;
    ufds[0].revents = 0;

    i = 1;
    for (auto const& x : this->callbacks)
    {
        // Open a connection to the attribute file.
        if ((fd = open(x.first.c_str(), O_RDONLY)) < 0)
        {
            LOGERR("Failed to open " + x.first.string() +
                   ", ret = " + to_string(fd));
            return fd;
        }
        ufds[i].fd = fd;
        ufds[i].events = POLLPRI | POLLERR;
        ufds[i].revents = 0;
        i++;
        // Someone suggested dummy reads before the poll() call
        read(fd, dummyData, sizeof(dummyData));

        // Add to internal event list
        event.path = x.first;
        event.handler = x.second;
        events[fd] = event;
    }

    while (1)
    {
        if ((rv = poll(ufds, n, -1)) <= 0)
            break;

        if (ufds[0].revents & POLLIN)
        {
            char dummy;
            ufds[0].revents = 0;
            if (read(ctrlFd, &dummy, 1) == 1 && dummy == 0)
                break;
            rv--;
        }

        for (i = 1; i < n && rv > 0; i++)
        {
            if ((ufds[i].revents & (POLLPRI | POLLERR)) == (POLLPRI | POLLERR))
            {
                event = events[ufds[i].fd];

                // lseek+read is required to clear the POLLPRI | POLLERR
                // condition
                if (lseek(ufds[i].fd, 0, SEEK_SET) < 0)
                    break;
                int n = read(ufds[i].fd, event.data, sizeof(event.data));
                if (n > 0)
                    this->io->post(
                        [event] { event.handler(event.path, event.data); });

                ufds[i].revents = 0;
                rv--;
            }
        }
    }

    for (i = 0; i < this->callbacks.size() + 1; i++)
        close(ufds[i].fd);

    delete ufds;
    close(ctrlFd);
    LOGDEBUG("Terminating SysFS watcher thread");

    return 0;
}

SysFsWatcher::SysFsWatcher(boost::asio::io_context& io) :
    io(&io), controlFd(-1), runner(nullptr)
{}

SysFsWatcher::~SysFsWatcher()
{
    this->Stop();
}