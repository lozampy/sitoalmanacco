#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <cstdlib>

const int PORT = 8080;
const std::string WEB_ROOT = "/workspace";

std::string getContentType(const std::string& path) {
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".html") return "text/html";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".css") return "text/css";
    if (path.size() >= 3 && path.substr(path.size() - 3) == ".js") return "application/javascript";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".json") return "application/json";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".png") return "image/png";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".jpg") return "image/jpeg";
    if (path.size() >= 5 && path.substr(path.size() - 5) == ".jpeg") return "image/jpeg";
    if (path.size() >= 4 && path.substr(path.size() - 4) == ".gif") return "image/gif";
    return "application/octet-stream";
}

std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string buildResponse(int statusCode, const std::string& statusText, 
                          const std::string& contentType, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

std::string build404Response() {
    std::string body = "<!DOCTYPE html><html><head><title>404 Not Found</title></head>"
                       "<body><h1>404 - Not Found</h1><p>The requested resource was not found.</p></body></html>";
    return buildResponse(404, "Not Found", "text/html", body);
}

void handleRequest(int clientSocket) {
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    
    std::string request(buffer);
    std::istringstream requestStream(request);
    std::string method, path, protocol;
    
    requestStream >> method >> path >> protocol;
    
    // Handle API endpoint example
    if (path == "/api/hello") {
        std::string jsonResponse = "{\"message\": \"Hello from C++ backend!\", \"status\": \"ok\"}";
        std::string response = buildResponse(200, "OK", "application/json", jsonResponse);
        write(clientSocket, response.c_str(), response.size());
        close(clientSocket);
        return;
    }
    
    // Default to index.html for root
    if (path == "/") {
        path = "/index.html";
    }
    
    // Security: prevent directory traversal
    if (path.find("..") != std::string::npos) {
        std::string response = build404Response();
        write(clientSocket, response.c_str(), response.size());
        close(clientSocket);
        return;
    }
    
    std::string filepath = WEB_ROOT + path;
    std::string content = readFile(filepath);
    
    if (content.empty()) {
        std::string response = build404Response();
        write(clientSocket, response.c_str(), response.size());
    } else {
        std::string contentType = getContentType(path);
        std::string response = buildResponse(200, "OK", contentType, content);
        write(clientSocket, response.c_str(), response.size());
    }
    
    close(clientSocket);
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen;
    
    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Allow socket reuse
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Setup server address
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket on port " << PORT << std::endl;
        close(serverSocket);
        return EXIT_FAILURE;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        close(serverSocket);
        return EXIT_FAILURE;
    }
    
    std::cout << "C++ Backend Server running on http://localhost:" << PORT << std::endl;
    std::cout << "Serving files from: " << WEB_ROOT << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    // Signal handler for graceful shutdown
    signal(SIGINT, [](int) {
        std::cout << "\nShutting down server..." << std::endl;
        exit(0);
    });
    
    // Accept connections
    while (true) {
        clientLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
        
        std::cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        handleRequest(clientSocket);
    }
    
    close(serverSocket);
    return EXIT_SUCCESS;
}
