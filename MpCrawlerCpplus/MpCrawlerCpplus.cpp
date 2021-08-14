// MpCrawlerCpplus.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define CURL_STATICLIB
#include <iostream>
#include <stdio.h>
// #include <set>
// #include <string>

//#include <string.h>
#include <stdlib.h>
//#include <string>
//#include <curl/curl.h>
#include <list>
#include <unordered_set>

using namespace std;

class RouteInfo {
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

static void CrawlePerGrade(string url, list<RouteInfo> routeInfoList, unordered_set<string> linkSet, int index) {

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


void main()
{
    cout << "Hello World!\n";
    // const char* url = "https://www.mountainproject.com/";
    // cout<< GetHtmlText(url) << endl;
    list<string> res = GenSearchUrlList("11111");


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
