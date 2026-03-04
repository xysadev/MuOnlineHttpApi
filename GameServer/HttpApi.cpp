#include "stdafx.h"
#include "HttpApi.h"
#include <sstream>
#include <algorithm>

HttpApi::HttpApi() { Initialize(); }

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
        response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n" + GetJsonEvents();
    }
    else
    {
        response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nUnknown endpoint";
    }

    send(clientSock, response.c_str(), response.size(), 0);
    closesocket(clientSock);
}

void HttpApi::ProcessKillDeathEvent(int killerIndex, const std::string& killerName,
    int deadIndex, const std::string& deadName)
{
    std::lock_guard<std::mutex> lock(eventsMutex);

    KillEvent evt{ killerIndex, killerName, deadIndex, deadName };
    events.push_back(evt);

    if (events.size() > MAX_EVENTS)
    {
        events.pop_front();
    }
}

std::string HttpApi::GetJsonEvents()
{
    std::lock_guard<std::mutex> lock(eventsMutex);
    std::ostringstream ss;
    ss << "{ \"events\": [";

    for (size_t i = 0; i < events.size(); ++i)
    {
        auto& e = events[i];
        ss << "{"
            << "\"killerIndex\":" << e.killerIndex << ","
            << "\"killerName\":\"" << e.killerName << "\","
            << "\"deadIndex\":" << e.deadIndex << ","
            << "\"deadName\":\"" << e.deadName << "\""
            << "}";
        if (i + 1 < events.size()) ss << ",";
    }

    ss << "] }";
    return ss.str();
}