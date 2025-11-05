#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

// 📘 Load credentials from users.txt
unordered_map<string, string> loadUsers() {
    unordered_map<string, string> users;
    ifstream file("users.txt");
    string username, password;
    while (file >> username >> password) {
        users[username] = password;
    }
    return users;
}

// 🔐 Authenticate client
bool authenticateClient(SOCKET clientSocket, const unordered_map<string, string> &users) {
    char buffer[256];

    // Receive username
    int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) return false;
    buffer[bytes] = '\0';
    string username(buffer);

    // Receive password
    bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0) return false;
    buffer[bytes] = '\0';
    string password(buffer);

    if (users.count(username) && users.at(username) == password) {
        string success = "AUTH_SUCCESS";
        send(clientSocket, success.c_str(), success.size(), 0);
        cout << "✅ " << username << " logged in successfully.\n";
        return true;
    } else {
        string fail = "AUTH_FAIL";
        send(clientSocket, fail.c_str(), fail.size(), 0);
        cout << "❌ Authentication failed for user: " << username << endl;
        return false;
    }
}

// 📁 List all files in the "server_files" folder
void listFiles(SOCKET clientSocket) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("server_files\\*", &findFileData);
    string fileList = "";

    if (hFind == INVALID_HANDLE_VALUE) {
        fileList = "Error: Folder not found.\n";
    } else {
        do {
            string name = findFileData.cFileName;
            if (name != "." && name != "..")
                fileList += name + "\n";
        } while (FindNextFileA(hFind, &findFileData));
        FindClose(hFind);
    }

    send(clientSocket, fileList.c_str(), fileList.size(), 0);
}

// 📤 Send file to client
void sendFile(SOCKET clientSocket, const string &fileName) {
    string filePath = "server_files\\" + fileName;
    ifstream file(filePath, ios::binary);
    if (!file.is_open()) {
        string msg = "Error: File not found.";
        send(clientSocket, msg.c_str(), msg.size(), 0);
        return;
    }

    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    send(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    char buffer[1024];
    while (!file.eof()) {
        file.read(buffer, sizeof(buffer));
        int bytesRead = file.gcount();
        send(clientSocket, buffer, bytesRead, 0);
    }

    file.close();
    cout << "📤 Sent file: " << fileName << endl;
}

// 📥 Receive file from client
void receiveFile(SOCKET clientSocket, const string &fileName) {
    string filePath = "server_files\\" + fileName;
    ofstream file(filePath, ios::binary);
    if (!file.is_open()) {
        string msg = "Error: Cannot create file.";
        send(clientSocket, msg.c_str(), msg.size(), 0);
        return;
    }

    size_t fileSize;
    recv(clientSocket, reinterpret_cast<char*>(&fileSize), sizeof(fileSize), 0);

    char buffer[1024];
    size_t totalReceived = 0;

    while (totalReceived < fileSize) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) break;
        file.write(buffer, bytesReceived);
        totalReceived += bytesReceived;
    }

    file.close();
    cout << "📥 Received file: " << fileName << endl;
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    unordered_map<string, string> users = loadUsers();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    cout << "✅ Server running on port " << PORT << " and waiting for connection..." << endl;

    SOCKET clientSocket;
    sockaddr_in clientAddr{};
    int clientAddrSize = sizeof(clientAddr);
    clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    cout << "✅ Client connected!" << endl;

    // Authenticate user before anything
    if (!authenticateClient(clientSocket, users)) {
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
        return 0;
    }

    while (true) {
        char option[256];
        int bytesReceived = recv(clientSocket, option, sizeof(option) - 1, 0);
        if (bytesReceived <= 0) break;
        option[bytesReceived] = '\0';
        string choice(option);

        if (choice == "LIST_SERVER") {
            listFiles(clientSocket);
        } else if (choice.rfind("UPLOAD:", 0) == 0) {
            string filename = choice.substr(7);
            receiveFile(clientSocket, filename);
        } else if (choice.rfind("DOWNLOAD:", 0) == 0) {
            string filename = choice.substr(9);
            sendFile(clientSocket, filename);
        } else if (choice == "EXIT") {
            cout << "Client disconnected.\n";
            break;
        } else {
            string msg = "Invalid command.";
            send(clientSocket, msg.c_str(), msg.size(), 0);
        }
    }

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
