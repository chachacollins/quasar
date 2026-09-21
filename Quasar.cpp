#include <cstring>
#include <sys/select.h>
#include <unistd.h>

#include <chrono>

#include "Quasar.hpp"


namespace Quasar
{
    long long get_time_ms()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
                ).count();
    }
    int input_waiting()
    {
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        select(16, &readfds, nullptr, nullptr, &tv);
        return FD_ISSET(fileno(stdin), &readfds);
    }
    void read_input()
    {
        if (!input_waiting()) return;

        char input[256] = "";
        int bytes;
        do { bytes = read(fileno(stdin), input, sizeof(input) - 1); } while (bytes < 0);
        if (bytes <= 0) return;

        if (char* endc = strchr(input, '\n')) *endc = '\0';

        if (strlen(input) > 0)
        {
            if (!strncmp(input, "quit", 4)) { quit = 1; stopped = 1; }
            else if (!strncmp(input, "stop", 4)) { stopped = 1; }
        }
    }

    void communicate()
    {
        if (timeset == 1 && get_time_ms() > stoptime) stopped = 1;
        read_input();
    }
}
