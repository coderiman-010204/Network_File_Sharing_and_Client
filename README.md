# Network File Sharing (C++ Socket Programming)

A client-server application that allows file listing, downloading, uploading, and secure communication using Winsock on Windows.

## Features
- Server-Client socket communication
- File listing and selection
- File transfer (download & upload)
- Authentication and encryption (planned)
- Windows compatible (no Linux required)

## Tech
- C++ (MinGW, Winsock2)
- Visual Studio Code

## Usage
1. Run `server.exe` from the `server` folder.
2. Run `client.exe` from the `client` folder.
3. Follow on-screen menu options.

## How to Run this files in Windows Os 
1.Open two terminals in VS Code or Powershell. 
2.Change the directory for each one for "cd server" and another "cd client".
3.Then on the "Server" terminal write "g++ server.cpp -o server.exe" and "./server.exe"
4.After that on the "Client" terminal write "g++ client.cpp -o client.exe" and "./client.exe"
5.Then choose the accordingly what you need to do LIST,UPLOAD,DOWNLOAD etc.
