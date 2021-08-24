// MpCrawlerCpplus.cpp : This file contains the 'main' function. Program execution begins and ends there.
// #include "stdafx.h" 
/*
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

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
#include <stack>
#include <regex>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#define CURL_STATICLIB
#include <curl/curl.h> // include directory not reference directory


#define WSWENS MAKEWORD(2,0)

using namespace std;

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

*/

/*
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static void GetHtmlText(const char* url) {
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        cout << readBuffer << endl;
    }

    // return readBuffer;
}

*/

namespace HtmlFunc {


    /// <summary>

    /// </summary>
    /// <param name="htmlText"></param>
    /// <param name="tagName"></param>
    /// <returns></returns>

    /// <summary>
    ///           looking for : < tagName > ... </ tagName >
    ///           Handle the condition that within a qualified tag there are more than one nested qualified tags
    /// 
    ///           if config == 1, find the first valid tag section, if config == 0, find all
    ///           for nested tag section and config == 1, the last element in the list is the first tag
    /// Todo:
    ///            handle fault when stack is not empty in the end, or end find while stack is empty
    /// </summary>
    /// <param name="htmlText"></param>
    /// <param name="tagName"></param>
    /// <param name="config"> 0, find all; 1 find the first one</param>
    /// <returns></returns>
    static list<string> SearchByTagName(string htmlText, string tagName, boolean config) {
        list<string> validTagContent;
        stack<int> nestedBeginTags;
        string tagBegin = "<" + tagName;         // "< tagName" 
        string tagEnd = "</" + tagName + ">";    // "</tagName>"

        for (int i = 0; i < (htmlText.length() - tagBegin.length()); i++)
        {
            // skip if tagBegin is not found
            if (!htmlText.substr(i, tagBegin.length()).compare(tagBegin) == 0)
                continue;

            for (int j = i; j < htmlText.length() - tagEnd.length(); j++) {
                // if tag begin is found, push the index of first character into stack
                if (htmlText.substr(j, tagBegin.length()).compare(tagBegin) == 0) {
                    nestedBeginTags.push(j);
                    j += tagBegin.length();
                    continue;
                }

                // if end tag is found, pair with the most recent tag begin index
                if (htmlText.substr(j, tagEnd.length()).compare(tagEnd) == 0) {
                    int startIndex = nestedBeginTags.top();
                    int sectionLen = j + tagEnd.length() - startIndex;

                    nestedBeginTags.pop();

                    validTagContent.push_back(htmlText.substr(startIndex, sectionLen));

                    // exit if all tag ends are found
                    if (nestedBeginTags.size() == 0) {
                        // if config == 1, find the first qualified tag section,
                        // which is the last one in the list
                        if (config == true)
                            return validTagContent;

                        // else, keep search
                        i = j + tagEnd.length();
                        break;
                    }
                }
            }
        }

        return validTagContent;
    }


    /// <summary>
    ///         look for < tagName ... class = ""... >
    ///         check the string in the first <> in case of nested tags
    /// </summary>
    /// <param name="tagContentList"></param>
    /// <param name="className"></param>
    /// <returns></returns>
    static list<string> FilterByClassName(list<string> tagContentList, string className) {

        list<string> filterResult;

        list<string>::iterator strIter;
        for (strIter = tagContentList.begin(); strIter != tagContentList.end(); strIter++) {
            string currStr = *strIter;
            int searchLen = currStr.find('>') + 1;

            if (currStr.substr(0, searchLen).find(className)) {
                filterResult.push_back(currStr);
            }
        }

        return filterResult;
    }


    /// <summary>
    ///         this function is to search any tag with provided tagName 
    ///         find1 : <tagName> ... </tagName>
    /// </summary>
    /// <param name="htmlText"></param>
    /// <param name="tagNmae"></param>
    /// <returns></returns>
    static list<string> HtmlFindAllTags(string htmlText, string tagName) {
        return HtmlFunc::SearchByTagName(htmlText, tagName, false);
    }

    /// <summary>
    ///         this function is to search first tag with tagName provided
    ///         find1 : <tagName> ... </tagName>
    /// 

    /// </summary>
    /// <param name="htmlText"></param>
    /// <param name="tagName"></param>
    /// <returns> 
    ///         Get the last element from the returned string list. 
    ///         If it is not a nested list, it shall only include one element
    /// </returns>
    static string HtmlFindFirstTag(string htmlText, string tagName) {

        return HtmlFunc::SearchByTagName(htmlText, tagName, true).back();
    }


    /// <summary>
    /// 
    ///            this function is to search any tag with tagName. Also, it shall include typeName 
    ///            find2 : <tagName class="..."> ... </tagName>, <tagName href="..."> ... </tagName>
    /// Notes:
    ///              type can be " any="any... "
    /// </summary>
    /// <param name="routeHtmlText"></param>
    /// <returns></returns>
    static list<string> HtmlFindAllTags(string htmlText, string tagName, string className) {

        return HtmlFunc::FilterByClassName(HtmlFunc::SearchByTagName(htmlText, tagName, false), className);
    }


    /// <summary>
    ///            this function is to search the first tag with tagName. Also, it shall include typeName 
    ///            find2 : <tagName class="..."> ... </tagName>, <tagName href="..."> ... </tagName>
    /// Notes:
    ///              type can be " any="any... "
    /// </summary>
    /// <param name="routeHtmlText"></param>
    /// <returns></returns>
    static string HtmlFindFirstTag(string htmlText, string tagName, string className) {
        return HtmlFunc::FilterByClassName(HtmlFunc::SearchByTagName(htmlText, tagName, true), className).back();
    }


    /// <summary>
    ///              Specify the first <> as the search range, in case of nested tag
    ///              Return the first found url begins with linkName. Otherwise empty string
    ///              For instance, if "href="http://" is found within the first <>, extract link and add it to the list
    ///
    /// Notes:
    ///              type can be " any="any... "
    /// 
    /// </summary>
    /// <param name="currStr"></param>
    /// <param name="linkKeywords"></param>
    /// <returns></returns>
    static string GetHyperLinks(string currStr, string linkKeywords) {
        string hyperLink = "";
        int searchEnd = currStr.find('>');
        string searchStr = currStr.substr(0, searchEnd + 1);

        if (searchStr.find(linkKeywords) != string::npos)
        {
            int linkBeginAt = currStr.find("http");
            int linkEndAt = currStr.substr(linkBeginAt, searchEnd - linkBeginAt).find('"');  // find " which ends the url
            hyperLink = currStr.substr(linkBeginAt, linkEndAt);
        }

        return hyperLink;
    }
}


namespace GetRouteInfo {

    /// <summary>
    ///          target section is as below example:
    ///          < title > Climb Horse Fly, Olympics & amp; Pacific Coast < / title >
    ///          1) find the first title and split at first ','
    ///          2) remove "<title> Climb"
    /// </summary>
    /// <param name="routeHtmlText"></param>
    /// <returns></returns>
    static string GetRouteName(string routeHtmlText) {
        // get tag section which contains route name information
        string nameSection = HtmlFunc::HtmlFindFirstTag(routeHtmlText, "title");
        int beginIndex = 13; // "<title>Climb " begin since 13th char
        int len = nameSection.find(",") - beginIndex + 1;
        return nameSection.substr(beginIndex, len);
    }

    /// <summary>
    ///Function:
    ///target area is as below example:
    //    < h2 class ="inline-block mr-2" >
    //      < span class ='rateYDS' > V6
    //          <a href="https://www.mountainproject.com/international-climbing-grades" class ="font-body" >
    //              < span class ="small" > YDS
    //              < / span>
    //          < / a >
    //      < / span >
    //      < span class ='rateFont' > 7A
    //          <a href="https://www.mountainproject.com/international-climbing-grades" class ="font-body" >
    //              < span class ="small" > Font
    //              < / span>
    //          < / a >
    //      < / span >
    //    < / h2 >
    /// </summary>
    /// <param name="routeHtmlText"></param>
    /// <returns></returns>
    static string GetRouteGrade(string routeHtmlText) {
        list<string> gradeSection = HtmlFunc::HtmlFindAllTags(routeHtmlText, "h2", "inline-block mr-2");
        string routeGradeRow = HtmlFunc::HtmlFindAllTags(routeHtmlText, "span", "class='rateYDS'").front();
        int beginIndex = routeGradeRow.find("'rateYDS'>") + 10;
        int len = routeGradeRow.find(" <a href=") - beginIndex;
        return routeGradeRow.substr(beginIndex, len);
    }

    /// <summary>
    // Function:
    //          search all<a> tag which shall include location link following below sequence
    //          [parent location1 link, parent location2 link, ..., finest area location link, boulder link]
    // Todo:
    //          are there any location name originally with '-' ?
    //
    //    Target location area is as below example:
    //
    //    < div class ="mb-half small text-warm" >
    //        < a href="https://www.mountainproject.com/route-guide" > All Locations< / a>
    //        & gt;
    //        < a href="https://www.mountainproject.com/area/105708966/washington" > Washington < / a >
    //        &gt;
    //        < a href="https://www.mountainproject.com/area/108471374/central-west-cascades-seattle" > Central - W Casca & hellip; < / a >
    //        & gt;
    //        < a href="https://www.mountainproject.com/area/108471672/skykomish-valley" > Skykomish Valley< / a>
    //        & gt;
    //        < a href="https://www.mountainproject.com/area/105805788/gold-bar-boulders" > Gold Bar Boulders< / a >
    //        & gt;
    //        < a href="https://www.mountainproject.com/area/105970461/zekes-trail-boulders" > Zeke &  # 039;s Trail Bo&hellip;</a>
    //        &gt;
    //        < a href="https://www.mountainproject.com/area/118994021/jaws-boulder" > Jaws Boulder< / a>
    //    < / div > 
    /// </summary>
    /// <param name="routeHtmlText"></param>
    /// <returns></returns>

    static string GetRouteLocation(string routeHtmlText) {
        stringstream locationStr;
        string locationSection = HtmlFunc::HtmlFindAllTags(routeHtmlText, "div", "class=\"mb-half small text-warm\"").front();
        list<string> aTagList = HtmlFunc::HtmlFindAllTags(locationSection, "a");
        int index = 0;

        list<string>::iterator aTag;
        for (aTag = aTagList.begin(); aTag != aTagList.end(); aTag++)
        {
            index += 1;

            if (index <= 1)
            {
                continue;
            }

            string locationUrl = HtmlFunc::GetHyperLinks(*aTag, "href=\"");

            int beginIndex = locationUrl.find_last_of("/") + 1;
            string rawLocation = locationUrl.substr(beginIndex, locationUrl.size() - beginIndex);
            string refinedLocation = regex_replace(rawLocation, regex("\\-"), " "); // repalce '-' with space
            locationStr << refinedLocation;

            if (index < aTagList.size())
                locationStr << "->"; // skip the last add "->"
        }

        return locationStr.str();
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
            *(gradeIterLast)+
            "&diffMinaid=70000&diffMinice=30000&diffMinmixed=50000&diffMaxrock=5500&diffMaxboulder=" +
            *(gradeIter)+
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
            RouteInfo routeInfo = RouteInfo(GetRouteInfo::GetRouteName(route),
                GetRouteInfo::GetRouteGrade(route),
                GetRouteInfo::GetRouteLocation(route),
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



namespace Url {
    static void getHtmlText(string url) {
        // Init Winsock
        WSADATA wsadata;
        if (WSAStartup(WSWENS, &wsadata) != 0)
            cout << "startup failed" << endl;

        // Create socket
        SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            ExitProcess(EXIT_FAILURE);
        }

        //sin.sin_addr.S_un.S_addr=inet_addr("203.208.37.99");
        hostent* hptr = gethostbyname("www.google.com");


        //hostent* hptr = gethostbyname(url.c_str());
        //hostent* hptr = gethostbyname("en.wikipedia.org");


        // Get host name
        //hostent* hptr = gethostbyname("www.mountainproject.com");
        if (hptr == nullptr) {
            ExitProcess(EXIT_FAILURE);
        }

        // Defien server info
        sockaddr_in sin;
        memset(&sin, 0, sizeof(sin));
        sin.sin_port = htons(80);
        sin.sin_family = AF_INET;

        memcpy(&sin.sin_addr.S_un.S_addr, hptr->h_addr, hptr->h_length);



        printf("IP address:%d.%d.%d.%d\n",
            sin.sin_addr.S_un.S_un_b.s_b1,
            sin.sin_addr.S_un.S_un_b.s_b2,
            sin.sin_addr.S_un.S_un_b.s_b3,
            sin.sin_addr.S_un.S_un_b.s_b4);

        // sockaddr_in transfer to sockaddr

        if (connect(s, (sockaddr*)&sin, sizeof(sin))) {
            cout << "connect failed" << endl;
            ExitProcess(EXIT_FAILURE);
        }
        else {
            cout << "connect success" << endl;
        }

        //char buffersend[] = "GET/HTTP1.0\nHOST:www.google.cn\nconnection:close\n\n";
        //send(s, buffersend, strlen(buffersend), 0);

        //int len = recv(s,const_cast(ss.c_str()),2000,0);
        /*char bufferecv[10240];
        int len = recv(s, bufferecv, 10240, 0);
        cout << bufferecv << endl;*/




        //dynamic allocate size
        // 
        // 
        //char buffersend[] = "GET / HTTP/1.0\r\nHost:www.mountainproject.com\r\nConnection: close\r\n\r\n";

        //int sendResult = send(s, buffersend, strlen(buffersend), 0);

        //printf("the length of request is %d\n", sendResult);




        // Do-while loop to send and receive data
        char buf[10240];
        char buffersend[] = "GET / HTTP/1.1\r\nHost:www.google.com\r\nConnection: close\r\n\r\n";
        // char buffersend[] = "HEAD / HTTP/1.0\r\n\r\n";


        string ss;
        // send GET / HTTP
        //                                    send(s, buffersend, 10240, 0);

        // recieve html
        while ((recv(s, buf, 10240, 0)) > 0) {
            int i = 0;
            while (buf[i] >= 32 || buf[i] == '\n' || buf[i] == '\r') {

                ss += buf[i];
                i += 1;
            }
        }

        cout << ss;
        // close down everything
        closesocket(s);
        WSACleanup();

        //return "";    
    }
}

/*
int main()
{
    cout << "Hello World!\n";

    //const string url = "www.mountainproject.com";
    //const char* url = "www.mountainproject.com";
    //Url::getHtmlText("www.mountainproject.com");
    //GetHtmlText("www.mountainproject.com");

    static std::string readBuffer;
    CURL* curl;
    CURLcode res;
    string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "www.mountainproject.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        cout << readBuffer << endl;
    }

    /*
    WSADATA wsaData;
    SOCKET Socket;
    SOCKADDR_IN SockAddr;
    int lineCount = 0;
    int rowCount = 0;
    struct hostent* host;
    locale local;
    char buffer[10000];
    int i = 0;
    int nDataLength;
    string website_HTML;

    // website url
    string url = "www.mountainproject.com";//"www.stackoverflow.com";;// "www.mountainproject.com";//"www.google.com";"en.wikipedia.org";

    //HTTP GET
    // string get_http = "GET /route-finder?type=boulder&amp;diffMinrock=1800&amp;diffMinboulder=20000&amp;diffMinaid=70000&amp;diffMinice=30000&amp;diffMinmixed=50000&amp;diffMaxrock=2800&amp;diffMaxboulder=20050&amp;diffMaxaid=75260&amp;diffMaxice=38500&amp;diffMaxmixed=60000&amp;is_trad_climb=1&amp;is_sport_climb=1&amp;is_top_rope=1&amp;stars=2.8&amp;pitches=0&amp;selectedIds=0 HTTP/1.1\r\nHost: " + url + "\r\nConnection: close\r\n\r\n";

    //string get_http = "GET / HTTP/1.1\r\nHost: " + url + "\r\nConnection: close\r\n\r\n";

    string get_http = "HEAD / HTTP/1.0\r\n\r\n";
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
        cout << "WSAStartup failed.\n";
        system("pause");
        //return 1;
    }

    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    host = gethostbyname(url.c_str());

    SockAddr.sin_port = htons(80);
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);

    if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
        cout << "Could not connect";
        system("pause");
        //return 1;
    }

    // send GET / HTTP
    send(Socket, get_http.c_str(), strlen(get_http.c_str()), 0);

    // recieve html
    while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0) {
        int i = 0;
        while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {

            website_HTML += buffer[i];
            i += 1;
        }
    }

    closesocket(Socket);
    WSACleanup();

    // Display HTML source
    cout << website_HTML;

    return 0;
}

*/


#include <iostream>
#include <string>
#include <curl/curl.h>


static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

int main(void)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::cout << readBuffer << std::endl;
    }
    return 0;
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