#include "stdafx.h"
#include "HttpApi.h"
#include <sstream>

HttpApi::HttpApi()
{
    Initialize();
}

HttpApi::~HttpApi()
{
    running = false;

    if (listenSock != INVALID_SOCKET)
        closesocket(listenSock);

    if (serverThread.joinable())
        serverThread.join();

    WSACleanup();
}

void HttpApi::Initialize()
{
    running = false;
    listenSock = INVALID_SOCKET;
    head = 0;
    count = 0;

    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
}

void HttpApi::StartServer(int port)
{
    listenSock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSock, 5);

    running = true;
    serverThread = std::thread(&HttpApi::ServerLoop, this);
}

void HttpApi::ServerLoop()
{
    while (running)
    {
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);

        SOCKET client = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);

        if (client != INVALID_SOCKET)
        {
            std::thread(&HttpApi::HandleClient, this, client).detach();
        }
    }
}

void HttpApi::HandleClient(SOCKET clientSock)
{
    char buffer[1024] = { 0 };

    int bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0)
    {
        closesocket(clientSock);
        return;
    }

    std::string request(buffer);
    std::string response;

    if (request.find("GET /events") != std::string::npos)
    {
        std::string json = GetJsonEvents();

        response = "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n" + json;
    }
    else
    {
        response = "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n"
            "Connection: close\r\n\r\nUnknown endpoint";
    }

    send(clientSock, response.c_str(), (int)response.size(), 0);
    closesocket(clientSock);
}

void HttpApi::ProcessKillDeathEvent(int killerIndex,
    const std::string& killerName,
    int deadIndex,
    const std::string& deadName)
{
    std::lock_guard<std::mutex> lock(eventsMutex);

    events[head] = { killerIndex, killerName, deadIndex, deadName };

    head = (head + 1) % MAX_EVENTS;

    if (count < MAX_EVENTS)
        count++;
}

std::string HttpApi::GetJsonEvents()
{
    // Crear snapshot fuera del lock
    std::array<KillEvent, MAX_EVENTS> snapshot;
    size_t snapshotCount;
    size_t snapshotHead;

    {
        std::lock_guard<std::mutex> lock(eventsMutex);
        snapshot = events;
        snapshotCount = count;
        snapshotHead = head;
    }

    std::ostringstream ss;
    ss << "{ \"events\": [";

    for (size_t i = 0; i < snapshotCount; ++i)
    {
        size_t index = (snapshotHead + MAX_EVENTS - snapshotCount + i) % MAX_EVENTS;
        const auto& e = snapshot[index];

        ss << "{"
            << "\"killerIndex\":" << e.killerIndex << ","
            << "\"killerName\":\"" << e.killerName << "\","
            << "\"deadIndex\":" << e.deadIndex << ","
            << "\"deadName\":\"" << e.deadName << "\""
            << "}";

        if (i + 1 < snapshotCount)
            ss << ",";
    }

    ss << "] }";

    return ss.str();
}