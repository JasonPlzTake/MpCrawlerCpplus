//#include <winsock2.h>
//
//#include <WS2tcpip.h>
//#include <windows.h>
//#include <iostream>
//#pragma comment(lib,"ws2_32.lib")

//using namespace std;

#define WIN32_LEAN_AND_MEAN

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>
#include <rpc.h>
#include <ntdsapi.h>
#include <stdio.h>
#include <tchar.h>
#include <iostream>
using namespace std;

#define RECV_DATA_BUF_SIZE 256

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

// link with fwpuclnt.lib for Winsock secure socket extensions
#pragma comment(lib, "fwpuclnt.lib")

// link with ntdsapi.lib for DsMakeSpn function
#pragma comment(lib, "ntdsapi.lib")

// The following function assumes that Winsock 
// has already been initialized

int
SecureTcpConnect(
    IN const struct sockaddr* serverAddr,
    IN ULONG serverAddrLen,
    IN const wchar_t* serverSPN,
    IN const SOCKET_SECURITY_SETTINGS* securitySettings,
    IN ULONG settingsLen)
    /**
    Routine Description:

        This routine creates a TCP client socket, securely connects to the
        specified server, sends & receives data from the server, and then closes
        the socket

    Arguments:

        serverAddr - a pointer to the sockaddr structure for the server.

        serverAddrLen - length of serverAddr in bytes

        serverSPN - a NULL-terminated string representing the SPN
                   (service principal name) of the server host computer

        securitySettings - pointer to the socket security settings that should be
                           applied to the connection

        serverAddrLen - length of securitySettings in bytes

    Return Value:

        Winsock error code indicating the status of the operation, or NO_ERROR if
        the operation succeeded.

    --*/
{
    int iResult = 0;
    int sockErr = 0;
    SOCKET sock = INVALID_SOCKET;

    WSABUF wsaBuf = { 0 };
    char* dataBuf = "12345678";
    DWORD bytesSent = 0;
    char recvBuf[RECV_DATA_BUF_SIZE] = { 0 };

    DWORD bytesRecvd = 0;
    DWORD flags = 0;
    SOCKET_PEER_TARGET_NAME* peerTargetName = NULL;
    DWORD serverSpnStringLen = (DWORD)wcslen(serverSPN);
    DWORD peerTargetNameLen = sizeof(SOCKET_PEER_TARGET_NAME) +
        (serverSpnStringLen * sizeof(wchar_t));

    //-----------------------------------------
    // Create a TCP socket
    sock = WSASocket(serverAddr->sa_family,
        SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
    if (sock == INVALID_SOCKET) {
        iResult = WSAGetLastError();
        wprintf(L"WSASocket returned error %ld\n", iResult);
        goto cleanup;
    }
    //-----------------------------------------
    // Turn on security for the socket.
    sockErr = WSASetSocketSecurity(sock,
        securitySettings, settingsLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASetSocketSecurity returned error %ld\n", iResult);
        goto cleanup;
    }
    //-----------------------------------------
    // Specify the server SPN
    peerTargetName = (SOCKET_PEER_TARGET_NAME*)HeapAlloc(GetProcessHeap(),
        HEAP_ZERO_MEMORY, peerTargetNameLen);
    if (!peerTargetName) {
        iResult = ERROR_NOT_ENOUGH_MEMORY;
        wprintf(L"Out of memory\n");
        goto cleanup;
    }
    // Use the security protocol as specified by the settings
    peerTargetName->SecurityProtocol = securitySettings->SecurityProtocol;
    // Specify the server SPN 
    peerTargetName->PeerTargetNameStringLen = serverSpnStringLen;
    RtlCopyMemory((BYTE*)peerTargetName->AllStrings,
        (BYTE*)serverSPN, serverSpnStringLen * sizeof(wchar_t)
    );

    sockErr = WSASetSocketPeerTargetName(sock,
        peerTargetName,
        peerTargetNameLen, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASetSocketPeerTargetName returned error %ld\n", iResult);
        goto cleanup;
    }
    //-----------------------------------------
    // Connect to the server
    sockErr = WSAConnect(sock,
        serverAddr, serverAddrLen, NULL, NULL, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSAConnect returned error %ld\n", iResult);
        goto cleanup;
    }
    // At this point a secure connection must have been established.
    wprintf(L"Secure connection established to the server\n");

    //-----------------------------------------
    // Send some data securely
    wsaBuf.len = (ULONG)strlen(dataBuf);
    wsaBuf.buf = dataBuf;
    sockErr = WSASend(sock, &wsaBuf, 1, &bytesSent, 0, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSASend returned error %ld\n", iResult);
        goto cleanup;
    }
    wprintf(L"Sent %d bytes of data to the server\n", bytesSent);

    //-----------------------------------------
    // Receive server's response securely
    wsaBuf.len = RECV_DATA_BUF_SIZE;
    wsaBuf.buf = recvBuf;
    sockErr = WSARecv(sock, &wsaBuf, 1, &bytesRecvd, &flags, NULL, NULL);
    if (sockErr == SOCKET_ERROR) {
        iResult = WSAGetLastError();
        wprintf(L"WSARecv returned error %ld\n", iResult);
        goto cleanup;
    }
    wprintf(L"Received %d bytes of data from the server\n", bytesRecvd);

cleanup:
    if (sock != INVALID_SOCKET) {
        //This will trigger the cleanup of all IPsec filters and policies that
        //were added for this socket. The cleanup will happen only after all
        //outstanding data has been sent out on the wire.
        closesocket(sock);
    }
    if (peerTargetName) {
        HeapFree(GetProcessHeap(), 0, peerTargetName);
    }
    return iResult;
}


int main() {
    // Initialize Dependencies to the Windows Socket.
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cout << "WSAStartup failed.\n";
        system("pause");
        return -1;
    }

    // We first prepare some "hints" for the "getaddrinfo" function
    // to tell it, that we are looking for a IPv4 TCP Connection.
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;          // We are targeting IPv4
    hints.ai_protocol = IPPROTO_TCP;    // We are targeting TCP
    hints.ai_socktype = SOCK_STREAM;    // We are targeting TCP so its SOCK_STREAM


    // Aquiring of the IPv4 address of a host using the newer
    // "getaddrinfo" function which outdated "gethostbyname".
    // It will search for IPv4 addresses using the TCP-Protocol.
    struct addrinfo* targetAdressInfo = NULL;
    DWORD getAddrRes = getaddrinfo("www.mountainproject.com", NULL, &hints, &targetAdressInfo);
    if (getAddrRes != 0 || targetAdressInfo == NULL)
    {
        cout << "Could not resolve the Host Name" << endl;
        system("pause");
        WSACleanup();
        return -1;
    }


    // Create the Socket Address Informations, using IPv4
    // We dont have to take care of sin_zero, it is only used to extend the length of SOCKADDR_IN to the size of SOCKADDR
    SOCKADDR_IN sockAddr;
    sockAddr.sin_addr = ((struct sockaddr_in*)targetAdressInfo->ai_addr)->sin_addr;    // The IPv4 Address from the Address Resolution Result
    sockAddr.sin_family = AF_INET;  // IPv4
    sockAddr.sin_port = htons(80);  // HTTP Port: 80

    SecureTcpConnect((const struct sockaddr*)&sockAddr, sizeof(sockAddr.sin_addr), );
    //IN const struct sockaddr* serverAddr,
    //IN ULONG serverAddrLen,
    //IN const wchar_t* serverSPN,
    //IN const SOCKET_SECURITY_SETTINGS * securitySettings,
    //IN ULONG settingsLen);


    printf("IP address:%d.%d.%d.%d\n",
        sockAddr.sin_addr.S_un.S_un_b.s_b1,
        sockAddr.sin_addr.S_un.S_un_b.s_b2,
        sockAddr.sin_addr.S_un.S_un_b.s_b3,
        sockAddr.sin_addr.S_un.S_un_b.s_b4);

    // We have to free the Address-Information from getaddrinfo again
    freeaddrinfo(targetAdressInfo);

    // Creation of a socket for the communication with the Web Server,
    // using IPv4 and the TCP-Protocol
    SOCKET webSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (webSocket == INVALID_SOCKET)
    {
        cout << "Creation of the Socket Failed" << endl;
        system("pause");
        WSACleanup();
        return -1;
    }


    ///////////////////////////////////////////////////////////////////////
    // Establishing a connection to the web Socket
    cout << "Connecting...\n";
    if (connect(webSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr)) != 0)
    {
        cout << "Could not connect";
        system("pause");
        closesocket(webSocket);
        WSACleanup();
        return -1;
    }
    cout << "Connected.\n";

    // Sending a HTTP-GET-Request to the Web Server
    // const char* httpRequest = "GET /route-guide HTTP/1.1\r\nHost: www.mountainproject.com\r\nConnection: close\r\n\r\n";
    const char* httpRequest = "GET // HTTP/1.1\r\nHost: www.mountainproject.com\r\nConnection: Upgrade\r\nUpgrade: websocket\r\n\r\n";
    int sentBytes = send(webSocket, httpRequest, strlen(httpRequest), 0);
    if (sentBytes < strlen(httpRequest) || sentBytes == SOCKET_ERROR)
    {
        cout << "Could not send the request to the Server" << endl;
        system("pause");
        closesocket(webSocket);
        WSACleanup();
        return -1;
    }

    // Receiving and Displaying an answer from the Web Server
    char buffer[10000];
    ZeroMemory(buffer, sizeof(buffer));
    int dataLen;
    while ((dataLen = recv(webSocket, buffer, sizeof(buffer), 0) > 0))
    {
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
            cout << buffer[i];
            i += 1;
        }
    }

    // Cleaning up Windows Socket Dependencies
    closesocket(webSocket);
    WSACleanup();

    //system("pause");
    return 0;
}


//#define WIN32_LEAN_AND_MEAN
//#include <winsock2.h>
//#include <windows.h>
//#include <ws2tcpip.h>
//#include <stdlib.h>
//#include <stdio.h>
//#include <iostream>
//
//using namespace std;
//// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
//#pragma comment (lib, "Ws2_32.lib")
//#pragma comment (lib, "Mswsock.lib")
//#pragma comment (lib, "AdvApi32.lib")
//
//
//#define DEFAULT_BUFLEN 512
//#define DEFAULT_PORT "27015"
//
//int main()
//{
//    WSADATA wsaData;
//    SOCKET ConnectSocket = INVALID_SOCKET;
//    struct addrinfo* result = NULL,* ptr = NULL,hints;
//    const char* sendbuf = "GET / route-guide HTTP / 1.1\r\nHost: www.mountainproject.com\r\nConnection: close\r\n\r\n";
//    char recvbuf[DEFAULT_BUFLEN];
//    int iResult;
//    int recvbuflen = DEFAULT_BUFLEN;
//
//    // Validate the parameters
//    //if (argc != 2) {
//    //    printf("usage: %s server-name\n", argv[0]);
//    //    return 1;
//    //}
//
//    // Initialize Winsock
//    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
//    if (iResult != 0) {
//        printf("WSAStartup failed with error: %d\n", iResult);
//        return 1;
//    }  
//
//    ZeroMemory(&hints, sizeof(hints));
//    hints.ai_family = AF_INET;
//    hints.ai_socktype = SOCK_STREAM;
//    hints.ai_protocol = IPPROTO_TCP;
//
//    // Resolve the server address and port
//    iResult = getaddrinfo("www.mountainproject.com", DEFAULT_PORT, &hints, &result);
//    if (iResult != 0) {
//        printf("getaddrinfo failed with error: %d\n", iResult);
//        WSACleanup();
//        return 1;
//    }
//    else {
//        cout << "success get address info" << endl;
//    }
//    int index = 0;
//    // Attempt to connect to an address until one succeeds
//    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
//
//        // Create a SOCKET for connecting to server
//        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
//            ptr->ai_protocol);
//        if (ConnectSocket == INVALID_SOCKET) {
//            printf("socket failed with error: %ld\n", WSAGetLastError());
//            WSACleanup();
//            return 1;
//        }
//
//        cout << index++ << endl;
//        // Connect to server.
//        iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
//
//        if (iResult == SOCKET_ERROR) {
//            closesocket(ConnectSocket);
//            ConnectSocket = INVALID_SOCKET;
//            continue;
//        }
//        break;
//    }
//
//    
//    freeaddrinfo(result);
//
//    if (ConnectSocket == INVALID_SOCKET) {
//        printf("Unable to connect to server!\n");
//        WSACleanup();
//        return 1;
//    }
//
//    // Send an initial buffer
//    iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
//    if (iResult == SOCKET_ERROR) {
//        printf("send failed with error: %d\n", WSAGetLastError());
//        closesocket(ConnectSocket);
//        WSACleanup();
//        return 1;
//    }
//    cout << "here2" << endl;
//    printf("Bytes Sent: %ld\n", iResult);
//
//    // shutdown the connection since no more data will be sent
//    iResult = shutdown(ConnectSocket, SD_SEND);
//    if (iResult == SOCKET_ERROR) {
//        printf("shutdown failed with error: %d\n", WSAGetLastError());
//        closesocket(ConnectSocket);
//        WSACleanup();
//        return 1;
//    }
//
//    // Receive until the peer closes the connection
//    do {
//
//        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
//        if (iResult > 0)
//            printf("Bytes received: %d\n", iResult);
//        else if (iResult == 0)
//            printf("Connection closed\n");
//        else
//            printf("recv failed with error: %d\n", WSAGetLastError());
//
//    } while (iResult > 0);
//
//    // cleanup
//    closesocket(ConnectSocket);
//    WSACleanup();
//
//    return 0;
//}






//#define WIN32_LEAN_AND_MEAN
//
//#ifndef UNICODE
//#define UNICODE
//#endif
//
//#include <windows.h>
//#include <winsock2.h>
//#include <mstcpip.h>
//#include <ws2tcpip.h>
//#include <rpc.h>
//#include <ntdsapi.h>
//#include <stdio.h>
//#include <tchar.h>
//
//#define RECV_DATA_BUF_SIZE 256
//
//// Link with ws2_32.lib
//#pragma comment(lib, "Ws2_32.lib")
//
//// link with fwpuclnt.lib for Winsock secure socket extensions
//#pragma comment(lib, "fwpuclnt.lib")
//
//// link with ntdsapi.lib for DsMakeSpn function
//#pragma comment(lib, "ntdsapi.lib")
//
//// The following function assumes that Winsock 
//// has already been initialized
//
//
//int
//SecureTcpConnect(IN const struct sockaddr* serverAddr,
//    IN ULONG serverAddrLen,
//    IN const wchar_t* serverSPN,
//    IN const SOCKET_SECURITY_SETTINGS* securitySettings,
//    IN ULONG settingsLen)
//    /**
//    Routine Description:
//
//        This routine creates a TCP client socket, securely connects to the
//        specified server, sends & receives data from the server, and then closes
//        the socket
//
//    Arguments:
//
//        serverAddr - a pointer to the sockaddr structure for the server.
//
//        serverAddrLen - length of serverAddr in bytes
//
//        serverSPN - a NULL-terminated string representing the SPN
//                   (service principal name) of the server host computer
//
//        securitySettings - pointer to the socket security settings that should be
//                           applied to the connection
//
//        serverAddrLen - length of securitySettings in bytes
//
//    Return Value:
//
//        Winsock error code indicating the status of the operation, or NO_ERROR if
//        the operation succeeded.
//
//    --*/
//{
//    int iResult = 0;
//    int sockErr = 0;
//    SOCKET sock = INVALID_SOCKET;
//
//    WSABUF wsaBuf = { 0 };
//    //char* dataBuf = "12345678";
//    DWORD bytesSent = 0;
//    char recvBuf[RECV_DATA_BUF_SIZE] = { 0 };
//
//    DWORD bytesRecvd = 0;
//    DWORD flags = 0;
//    SOCKET_PEER_TARGET_NAME* peerTargetName = NULL;
//    DWORD serverSpnStringLen = (DWORD)wcslen(serverSPN);
//    DWORD peerTargetNameLen = sizeof(SOCKET_PEER_TARGET_NAME) +
//        (serverSpnStringLen * sizeof(wchar_t));
//
//    //-----------------------------------------
//    // Create a TCP socket
//    sock = WSASocket(serverAddr->sa_family,
//        SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
//    if (sock == INVALID_SOCKET) {
//        iResult = WSAGetLastError();
//        wprintf(L"WSASocket returned error %ld\n", iResult);
//        goto cleanup;
//    }
//    //-----------------------------------------
//    // Turn on security for the socket.
//    sockErr = WSASetSocketSecurity(sock,
//        securitySettings, settingsLen, NULL, NULL);
//    if (sockErr == SOCKET_ERROR) {
//        iResult = WSAGetLastError();
//        wprintf(L"WSASetSocketSecurity returned error %ld\n", iResult);
//        goto cleanup;
//    }
//    //-----------------------------------------
//    // Specify the server SPN
//    peerTargetName = (SOCKET_PEER_TARGET_NAME*)HeapAlloc(GetProcessHeap(),
//        HEAP_ZERO_MEMORY, peerTargetNameLen);
//    if (!peerTargetName) {
//        iResult = ERROR_NOT_ENOUGH_MEMORY;
//        wprintf(L"Out of memory\n");
//        goto cleanup;
//    }
//    // Use the security protocol as specified by the settings
//    peerTargetName->SecurityProtocol = securitySettings->SecurityProtocol;
//    // Specify the server SPN 
//    peerTargetName->PeerTargetNameStringLen = serverSpnStringLen;
//    RtlCopyMemory((BYTE*)peerTargetName->AllStrings,
//        (BYTE*)serverSPN, serverSpnStringLen * sizeof(wchar_t)
//    );
//
//    sockErr = WSASetSocketPeerTargetName(sock,
//        peerTargetName,
//        peerTargetNameLen, NULL, NULL);
//    if (sockErr == SOCKET_ERROR) {
//        iResult = WSAGetLastError();
//        wprintf(L"WSASetSocketPeerTargetName returned error %ld\n", iResult);
//        goto cleanup;
//    }
//    //-----------------------------------------
//    // Connect to the server
//    sockErr = WSAConnect(sock,
//        serverAddr, serverAddrLen, NULL, NULL, NULL, NULL);
//    if (sockErr == SOCKET_ERROR) {
//        iResult = WSAGetLastError();
//        wprintf(L"WSAConnect returned error %ld\n", iResult);
//        goto cleanup;
//    }
//    // At this point a secure connection must have been established.
//    wprintf(L"Secure connection established to the server\n");
//
//    //-----------------------------------------
//    // Send some data securely
//    //wsaBuf.len = (ULONG)strlen(dataBuf);
//    //wsaBuf.buf = dataBuf;
//    //sockErr = WSASend(sock, &wsaBuf, 1, &bytesSent, 0, NULL, NULL);
//    //if (sockErr == SOCKET_ERROR) {
//    //    iResult = WSAGetLastError();
//    //    wprintf(L"WSASend returned error %ld\n", iResult);
//    //    goto cleanup;
//    //}
//    //wprintf(L"Sent %d bytes of data to the server\n", bytesSent);
//
//    //-----------------------------------------
//    // Receive server's response securely
//    wsaBuf.len = RECV_DATA_BUF_SIZE;
//    wsaBuf.buf = recvBuf;
//    sockErr = WSARecv(sock, &wsaBuf, 1, &bytesRecvd, &flags, NULL, NULL);
//    if (sockErr == SOCKET_ERROR) {
//        iResult = WSAGetLastError();
//        wprintf(L"WSARecv returned error %ld\n", iResult);
//        goto cleanup;
//    }
//    wprintf(L"Received %d bytes of data from the server\n", bytesRecvd);
//
//cleanup:
//    if (sock != INVALID_SOCKET) {
//        //This will trigger the cleanup of all IPsec filters and policies that
//        //were added for this socket. The cleanup will happen only after all
//        //outstanding data has been sent out on the wire.
//        closesocket(sock);
//    }
//    if (peerTargetName) {
//        HeapFree(GetProcessHeap(), 0, peerTargetName);
//    }
//    return iResult;
//}
