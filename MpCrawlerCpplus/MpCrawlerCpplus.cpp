// MpCrawlerCpplus.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
// #include "stdafx.h" 
// #define CURL_STATICLIB

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <stdlib.h>
#include <list>
#include <unordered_set>

#include<stdio.h>  
#include<string>  
#include<iostream>

#include <winsock2.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

#define WSWENS MAKEWORD(2,0)  

using namespace std;
/*
struct RouteInfo { ///////////////////////////// struct vs class ////////////////////////////////
public:
    string routeName;
    string routeGrade;
    string routeLocation;
    string routeLink;

    RouteInfo(string routeName, string routeGrade, string routeLocation, string routeLink) {
        this->routeName = routeName;
        this->routeGrade = routeGrade;
        this->routeLocation = routeLocation;
        this->routeLink = routeLink;
    }

    RouteInfo(){
        this->routeName = "";
        this->routeGrade = "";
        this->routeLocation = "";
        this->routeLink = "";
    }

    ~RouteInfo(){

    }
};


//static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
//{
//    ((string*)userp)->append((char*)contents, size * nmemb);
//    return size * nmemb;
//}
//
//static string GetHtmlText(const char* url) {
//    CURL* curl;
//    CURLcode res;
//    string readBuffer;
//
//    curl = curl_easy_init();
//    if (curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, url);
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
//        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
//        res = curl_easy_perform(curl);
//        curl_easy_cleanup(curl);
//        //cout << readBuffer << endl;
//    }
//
//    return readBuffer;
//}
namespace HtmlFunc{
    static list<string> HtmlFindAllTags(string htmlText, string tagName, string keywords) {

    }


    static string GetHyperLinks(string htmlText, string tagNmae) {

    }

    static string GetRouteName(string routeHtmlText) {

    }
    static string GetRouteGrade(string routeHtmlText) {

    }
    static string GetRouteLocation(string routeHtmlText) {

    }
}


static list<string> GenSearchUrlList(string locationCode) {

    list<string> gradeList = { "20000", // vB
                                "20050", // v0
                                "20150", // v1
                                "20250", // v2
                                "20350", // v3
                                "20450", // v4
                                "20550", // v5
                                "20650", // v6
                                "20750", // v7                                                  
                                "20850", // v8
                                "20950", // v9
                                "21050", // v10
                                "21150", // v11
                                "21250", // v12
                                "21350", // v13
                                "21450", // v14
                                "21550", // v15
                                "21650", // v16
                                "21750" };// v17
    list<string> urlList;
    list<string> ::iterator gradeIter = gradeList.begin();
    list<string> ::iterator gradeIterLast = gradeList.begin();
    
    for (++gradeIter; gradeIter != gradeList.end(); gradeIter++)
    {
        // generate search url based on grade and location
        // for searching grade Vn for instance, min grade shall be V(n-1) and grade max shall be V(n)

        string searchUrl = "https://www.mountainproject.com/route-finder?selectedIds=" +
            locationCode +
            "&type=boulder&diffMinrock=1800&diffMinboulder=" +
            *(gradeIterLast) +
            "&diffMinaid=70000&diffMinice=30000&diffMinmixed=50000&diffMaxrock=5500&diffMaxboulder=" +
            *(gradeIter) +
            "&diffMaxaid=75260&diffMaxice=38500&diffMaxmixed=60000&is_trad_climb=1&is_sport_climb=1&is_top_rope=1&stars=0&pitches=0&sort1=popularity+desc&sort2=rating&viewAll=1";

        urlList.push_back(searchUrl);

        gradeIterLast = gradeIter;

        cout << searchUrl << endl;
    }

    return urlList;
}

static void CrawlePerGrade(string url, list<RouteInfo> routeInfoList, unordered_set<string> routeLinkSet, int searchGrade) {
    // Function: 
    //          1) get each route page link from the 'url' page from inputs 
    //             (In search url page, 'a' tag with class as "text-black route-row" is the route info page for each route)
    //
    //          2) collect route name, route grade, route location, ruote link and saved in routeInfoList by visiting each route link 
    //
    //          3) Error message:
    //                          A. if location code is incorrect, throw an error message
    //                          B. If 1000 routes link is found in the search page, throw an error message.
    //                             (if search page has "All Locations", it means location code is not correct
    //                             a more accurate check could be find tag name and keywords as below:
    //                             "<span id="single-area-picker-name">All Locations</span>"
    //
    // Inputs:
    //         url :          the url shall include list of route links from mountain project route finder engine
    //         routeInfoList: it contains route info class defined in WebAppDataLib.Models
    //         routeLinkSet:  it contains route links which has been visited from lower grading url
    //         searchGrade:   used for error reporting to indicate which grade has more than 1000 routes
    //

    /// Get each route page
    string htmlText;
    list<string> routeLinkTagList = HtmlFunc::HtmlFindAllTags(htmlText, "a", "class=\"text-black route-row\"");


    /// ERROR A:          
    if (htmlText.find("All Locations") != string::npos)
    {       
        string errMessage = "Incorrect Location Code!";
        throw std::logic_error(errMessage); // ******************************************************************************
    }

    /// ERROR B:    
    if (routeLinkTagList.size() == 1000)
    {
        string errMessage = "Route Number at Grade V" + to_string(searchGrade) + "exceed 1000!";
        throw std::logic_error(errMessage); //*************************************************
    }

    /// Crawling 
    cout << "Collect " << routeLinkTagList.size() << " routes information..." << endl;

    list<string> ::iterator routeLinkTag;

    for (routeLinkTag = routeLinkTagList.begin(); routeLinkTag != routeLinkTagList.end(); routeLinkTag++)
    {
        string routeLink = HtmlFunc::GetHyperLinks(*routeLinkTag, "href=");

        // skip visited routes
        if (routeLinkSet.find(routeLink) != routeLinkSet.end())
        {
            continue;
        }
        else
        {
            string route = ""; //*******************************************************************************
            // ERROR:
            // below fault can be pre-detected by error reporting in 
            if (route.find("The page you're looking for does not exist") != string::npos)
            {
                throw std::logic_error("Page Not Found!"); //********************** Do we wanna stop or continue? ******************
            }

            // Collect route information from each route page
            RouteInfo routeInfo = RouteInfo(HtmlFunc::GetRouteName(route),
                                            HtmlFunc::GetRouteGrade(route),
                                            HtmlFunc::GetRouteLocation(route),
                                            routeLink);
            routeInfoList.push_back(routeInfo);
            routeLinkSet.insert(routeLink);
            // Console.WriteLine("processing route : " + routeName + " ...");
        }
    }
}

static list<RouteInfo> MpBoulderRouteCrawler(string locationCode) {

    list<RouteInfo> routeInfoList;
    unordered_set<string> linkSet;

    // get search url list for the specified location for each v grade
    list<string> urlList = GenSearchUrlList(locationCode);

    // index is used for exception error message to indicate which grade exceed 1000 route limit
    int index = 0;

    unordered_set<string> ::iterator url;
    
    for (url = urlList.begin(); url != urlList.end(); url++)
    {
        // call crawling function for each search url
        CrawlePerGrade(*url, routeInfoList, linkSet, index);
        index++;
    }

    return routeInfoList;
}
*/

int main()
{
    cout << "Hello World!\n";
    // const char* url = "https://www.mountainproject.com/";
    // cout<< GetHtmlText(url) << endl;
    //list<string> res = GenSearchUrlList("11111");
    /*
    sockaddr_in sin;
    WSADATA wsadata;
    //WSAStartup()的调用方和Windows Sockets DLL互相通知对方它们可以支持的最高版本,  
    //并且互相确认对方的最高版本是可接受的. 在WSAStartup()函数的入口,  
    //Windows Sockets DLL检查了应用程序所需的版本.如果版本高于DLL支持的最低版本,  
    //则调用成功并且DLL在wHighVersion中返回它所支持的最高版本,  
    //在 wVersion中返回它的高版本和wVersionRequested中的较小者.  
    //然后Windows Sockets DLL就会假设应用程序将使用wVersion.  
    if (WSAStartup(WSWENS, &wsadata) != 0)

        cout << "startup failed" << endl;

    SOCKET s = socket(PF_INET, SOCK_STREAM, 0);
    // memset 是对一段内存空间全部设置为某个字符  
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    //sin.sin_addr.S_un.S_addr=inet_addr("203.208.37.99");  
    hostent* hptr = gethostbyname("www.google.cn");
    // 将有参数的string内容copy到左边参数  
    memcpy(&sin.sin_addr.S_un.S_addr, hptr->h_addr, hptr->h_length);
    printf("IP address:%d.%d.%d.%d/n", sin.sin_addr.S_un.S_un_b.s_b1,
        sin.sin_addr.S_un.S_un_b.s_b2,
        sin.sin_addr.S_un.S_un_b.s_b3,
        sin.sin_addr.S_un.S_un_b.s_b4);
    // 将sockaddr_in transfer to sockaddr  
    if (connect(s, (sockaddr*)&sin, sizeof(sin)))
    {
        cout << "connect failed" << endl;
        return 0;
    }
    else
    {
        cout << "connect success" << endl;
    }

    char buffersend[] = "GET/HTTP1.1/nHOST:www.google.cn/nconnection:close/n/n";
    send(s, buffersend, (int) strlen(buffersend), 0); ////////////////////////////////// casting 
    string ss;
    int len = recv(s,const_cast<char*>(ss.c_str()),2000,0);
    char bufferecv[10240];

    len = recv(s, bufferecv, 10240, 0);
    printf("the length of page is %d/n", len);
    if (len == -1)
    {
        cout << "receive failed" << endl;
        return -1;
    }
    else
        cout << "receive success" << endl;
    for (int i = 0; i < len; i++)
        printf("%c", bufferecv[i]);
    WSACleanup();
    closesocket(s);
    return 0;

    */
    ///////////////////////////////////////////////////////////////
    // Socket1.cpp : 定义控制台应用程序的入口点。

//


        sockaddr_in sin;

        WSADATA wsadata;

        if (WSAStartup(WSWENS, &wsadata) != 0)

            cout << "startup failed" << endl;

        SOCKET s = socket(PF_INET, SOCK_STREAM, 0);

        memset(&sin, 0, sizeof(sin));

        sin.sin_family = AF_INET;

        sin.sin_port = htons(80);

        // sin.sin_addr.S_un.S_addr=inet_addr("203.208.37.99");

        hostent* hptr = gethostbyname("www.google.cn");

        memcpy(&sin.sin_addr.S_un.S_addr, hptr->h_addr, hptr->h_length);

        printf("IP address:%d.%d.%d.%d\n", sin.sin_addr.S_un.S_un_b.s_b1,

            sin.sin_addr.S_un.S_un_b.s_b2,

            sin.sin_addr.S_un.S_un_b.s_b3,

            sin.sin_addr.S_un.S_un_b.s_b4);

        // 将sockaddr_in transfer to sockaddr

        if (connect(s, (sockaddr*)&sin, sizeof(sin)))
        {
            cout << "connect failed" << endl;
            return 0;
        }
        else
        {
            cout << "connect success" << endl;
        }

        //char buffersend[] = "GET/HTTP1.1\nHOST:www.google.cn\nconnection:close\n\n";

        //send(s, buffersend, strlen(buffersend), 0);

        /*string ss;

        int len = recv(s,const_cast(ss.c_str()),2000,0);*/


        //char bufferecv[10240];

        //int len = recv(s, bufferecv, 10240, 0);

        //cout << bufferecv << endl;

        // Do-while loop to send and receive data
        char buf[4096];
        char buffersend[] = "GET/HTTP1.1\nHOST:www.google.cn\nconnection:close\n\n";

        do
        {
            // Prompt the user for some text

            // Send the text
            int sendResult = send(s, buffersend, strlen(buffersend) + 1, 0);
            if (sendResult != SOCKET_ERROR)
            {
                // Wait for response
                ZeroMemory(buf, 4096);
                int bytesReceived = recv(s, buf, 4096, 0);
                if (bytesReceived > 0)
                {
                    // Echo response to console
                    cout << "SERVER> " << string(buf, 0, bytesReceived) << endl;
                }
            }

        } while (strlen(buffersend) > 0);

        // Gracefully close down everything
        closesocket(s);
        // WSACleanup();

        //printf("the length of page is %d\n", len);
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
