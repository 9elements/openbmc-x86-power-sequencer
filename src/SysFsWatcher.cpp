
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

void SysFsWatcher::Register(filesystem::path p,
                            std::function<void(filesystem::path)> handler)
{
    auto search = this->callbacks.find(p);
    if (search == this->callbacks.end())
    {
        this->Stop();
        {
            boost::lock_guard<boost::mutex> lock(this->lock);
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
            boost::lock_guard<boost::mutex> lock(this->lock);
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

int SysFsWatcher::Main(int ctrlFd)
{
    boost::lock_guard<boost::mutex> lock(this->lock);
    const int n = this->callbacks.size() + 1;
    struct pollfd* ufds = new struct pollfd[n];
    char dummyData[1024] = {};
    int fd, rv;

    ufds[0].fd = ctrlFd;
    ufds[0].events = POLLIN;
    ufds[0].revents = 0;

    int i = 1;
    for (auto const& x : this->callbacks)
    {
        cout << "register sysfs " << x.first.string() << " watcher" << endl;
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
            char dummy;
            ufds[0].revents = 0;
            if (read(ctrlFd, &dummy, 1) == 1 && dummy == 0)
                break;
        }
        cout << "got poll event, " << rv << endl;
        int i = 1;
        for (auto const& x : this->callbacks)
        {
            cout << i << " " << ufds[i].revents << endl;

            if ((ufds[i].revents & (POLLPRI | POLLERR)) == (POLLPRI | POLLERR))
            {
                if (lseek(ufds[i].fd, 0, SEEK_SET) == -1)
                {
                    break;
                }
                read(ufds[i].fd, dummyData, sizeof(dummyData));

                this->io->post([x] { x.second(x.first); });
                ufds[i].revents = 0;
            }
            i++;
        }
    }

    for (i = 0; i < this->callbacks.size() + 1; i++)
        close(ufds[i].fd);

    delete ufds;
    close(ctrlFd);

    return 0;
}

SysFsWatcher::SysFsWatcher(boost::asio::io_context& io) :
    io(&io), controlFd(-1), runner(nullptr)
{}

SysFsWatcher::~SysFsWatcher()
{
    this->Stop();
}