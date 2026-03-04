#pragma once
#include <winsock2.h>
#include <thread>
#include <mutex>
#include <array>
#include <string>

#define MAX_EVENTS 100

struct KillEvent
{
    int killerIndex;
    std::string killerName;
    int deadIndex;
    std::string deadName;
};

class HttpApi
{
public:
    HttpApi();
    ~HttpApi();

    void Initialize();
    void StartServer(int port);

    void ProcessKillDeathEvent(int killerIndex,
        const std::string& killerName,
        int deadIndex,
        const std::string& deadName);

private:
    void ServerLoop();
    void HandleClient(SOCKET clientSock);
    std::string GetJsonEvents();

private:
    SOCKET listenSock;
    std::thread serverThread;
    bool running;

    std::array<KillEvent, MAX_EVENTS> events;
    size_t head;
    size_t count;

    std::mutex eventsMutex;
};