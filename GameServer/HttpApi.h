#pragma once
#include "stdafx.h"
#include <winsock2.h>
#include <thread>
#include <string>
#include <vector>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")

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
    void StartServer(int port = 8080);
    void ProcessKillDeathEvent(int killerIndex, const std::string& killerName,
        int deadIndex, const std::string& deadName);

private:
    void ServerLoop();
    void HandleClient(SOCKET clientSock);
    std::string GetJsonEvents();

    SOCKET listenSock;
    bool running;
    std::thread serverThread;
    std::vector<KillEvent> events;
    std::mutex eventsMutex;

    static constexpr size_t MAX_EVENTS = 100;
};