#ifndef BROWSER_H
#define BROWSER_H

#include "structures.h"
#include "fileManager.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <stack>
using namespace std;

class Browser {
private:
    vector<Tab*> tabs;
    int currentTabIndex, nextTabId;
    unordered_map<string, Page> bookmarks;
    unordered_map<string, int> visitCount;
    HistoryNode* historyHead;
    vector<SessionSnapshot> sessionHistory;

    static const int MAX_SNAPSHOTS = 20;
    time_t lastSnapshotTime = 0;
    static const int SNAPSHOT_INTERVAL = 300;  // 5 minutes

    // Helper Functions 
    void addToHistory(Page p) {
        HistoryNode* node = new HistoryNode(p);
        if (!historyHead) historyHead = node;
        else {
            HistoryNode* temp = historyHead;
            while (temp->next) temp = temp->next;
            temp->next = node;
        }
    }

    void captureSessionSnapshot(const string& desc) {
        SessionSnapshot snapshot(desc);

        for (auto tab : tabs) {
            string tabRecord;

            // Back stack (oldest to newest)
            vector<Page> backHistory;
            {
                stack<Page> temp = tab->backStack;
                while (!temp.empty()) {
                    backHistory.push_back(temp.top());
                    temp.pop();
                }
                reverse(backHistory.begin(), backHistory.end());
            }

            for (size_t i = 0; i < backHistory.size(); ++i) {
                tabRecord += backHistory[i].url;
                tabRecord += " -> ";
            }

            // Current page
            if (!tab->currentPage.url.empty())
                tabRecord += tab->currentPage.url;
            else
                tabRecord += "[empty]";

            // Forward stack
            {
                stack<Page> temp = tab->forwardStack;
                while (!temp.empty()) {
                    tabRecord += " -> " + temp.top().url;
                    temp.pop();
                }
            }

            snapshot.tabData.push_back({tab->id, tabRecord});
        }

        sessionHistory.push_back(snapshot);
        cout << "Captured snapshot: " << snapshot.tabData.size() << " tabs.\n";
    }

    // Sorting and searching
    void merge(vector<Page>& arr, int left, int mid, int right) {
        vector<Page> L(arr.begin() + left, arr.begin() + mid + 1);
        vector<Page> R(arr.begin() + mid + 1, arr.begin() + right + 1);
        int i = 0, j = 0, k = left;

        while (i < L.size() && j < R.size()) {
            if (L[i].title < R[j].title)
                arr[k++] = L[i++];
            else
                arr[k++] = R[j++];
        }

        while (i < L.size()) arr[k++] = L[i++];
        while (j < R.size()) arr[k++] = R[j++];
    }

    void mergeSort(vector<Page>& arr, int left, int right) {
        if (left < right) {
            int mid = (left + right) / 2;
            mergeSort(arr, left, mid);
            mergeSort(arr, mid + 1, right);
            merge(arr, left, mid, right);
        }
    }

    int binarySearch(vector<Page>& arr, string key) {
        int left = 0, right = arr.size() - 1;
        while (left <= right) {
            int mid = (left + right) / 2;
            if (arr[mid].title == key)
                return mid;
            else if (arr[mid].title < key)
                left = mid + 1;
            else
                right = mid - 1;
        }
        return -1;
    }

public:
    // Constructor & Destructor 
    Browser() : currentTabIndex(-1), nextTabId(1), historyHead(nullptr) {
        historyHead = FileManager::loadHistory();
        FileManager::loadBookmarks(bookmarks);
        FileManager::loadVisitCount(visitCount);
        FileManager::loadTabs(tabs, currentTabIndex, nextTabId);
        FileManager::loadSessionHistory(sessionHistory);
        if (tabs.empty()) createNewTab();
    }

    ~Browser() {
        captureSessionSnapshot("Auto-saved on exit");
        FileManager::saveHistory(historyHead);
        FileManager::saveBookmarks(bookmarks);
        FileManager::saveVisitCount(visitCount);
        FileManager::saveTabs(tabs, currentTabIndex, nextTabId);
        FileManager::saveSessionHistory(sessionHistory);
        for (auto tab : tabs) delete tab;
    }

    // Core Browser Features 
    void createNewTab() {
        tabs.push_back(new Tab(nextTabId++));
        currentTabIndex = tabs.size() - 1;
        cout << "\n New tab created (Tab #" << tabs.back()->id << ")\n";
        captureSessionSnapshot("New tab created");
    }

    void switchTab(int index) {
        if (index < 0 || index >= tabs.size()) {
            cout << "\n Invalid tab index!\n";
            return;
        }
        currentTabIndex = index;
        cout << "\n Switched to Tab #" << tabs[index]->id;
        if (!tabs[index]->currentPage.url.empty())
            cout << " - " << tabs[index]->currentPage.title;
        cout << "\n";
    }

    void closeCurrentTab() {
        if (tabs.size() == 1) {
            cout << "\n Cannot close the last tab!\n";
            return;
        }
        cout << "\n Closing Tab #" << tabs[currentTabIndex]->id << "\n";
        delete tabs[currentTabIndex];
        tabs.erase(tabs.begin() + currentTabIndex);
        if (currentTabIndex >= tabs.size()) currentTabIndex = tabs.size() - 1;
        cout << "Now on Tab #" << tabs[currentTabIndex]->id << "\n";
        captureSessionSnapshot("Tab closed");
    }

    void viewAllTabs() {
        cout << "\n========= Open Tabs (" << tabs.size() << ") =========\n";
        for (int i = 0; i < tabs.size(); i++) {
            cout << "[" << i << "] Tab #" << tabs[i]->id;
            if (i == currentTabIndex) cout << " (Current)";
            if (!tabs[i]->currentPage.url.empty())
                cout << " - " << tabs[i]->currentPage.title << " (" << tabs[i]->currentPage.url << ")";
            else cout << " - Empty";
            cout << "\n";
        }
        cout << "====================================\n";
    }

    void visitPage(string url, string title) {
        Tab* tab = tabs[currentTabIndex];
        if (!tab->currentPage.url.empty()) tab->backStack.push(tab->currentPage);
        while (!tab->forwardStack.empty()) tab->forwardStack.pop();
        tab->currentPage = Page(url, title);
        addToHistory(tab->currentPage);
        visitCount[url]++;
        cout << "\n Now visiting: " << title << " (" << url << ") in Tab #" << tab->id << "\n";
    }

    void goBack() {
        Tab* tab = tabs[currentTabIndex];
        if (tab->backStack.empty()) {
            cout << "\n No previous page!\n";
            return;
        }
        tab->forwardStack.push(tab->currentPage);
        tab->currentPage = tab->backStack.top();
        tab->backStack.pop();
        cout << "\n⬅ Back to: " << tab->currentPage.title << "\n";
    }

    void goForward() {
        Tab* tab = tabs[currentTabIndex];
        if (tab->forwardStack.empty()) {
            cout << "\n No forward page!\n";
            return;
        }
        tab->backStack.push(tab->currentPage);
        tab->currentPage = tab->forwardStack.top();
        tab->forwardStack.pop();
        cout << "\nForward to: " << tab->currentPage.title << "\n";
    }

    // Bookmarks 
    void addBookmark() {
        Tab* tab = tabs[currentTabIndex];
        if (tab->currentPage.url.empty()) {
            cout << "\n No active page to bookmark.\n";
            return;
        }
        bookmarks[tab->currentPage.url] = tab->currentPage;
        cout << "\n Bookmarked: " << tab->currentPage.title << "\n";
    }

    void viewBookmarks() {
        if (bookmarks.empty()) {
            cout << "\n No bookmarks.\n";
            return;
        }

        vector<Page> sortedBookmarks;
        for (auto& b : bookmarks)
            sortedBookmarks.push_back(b.second);

        mergeSort(sortedBookmarks, 0, sortedBookmarks.size() - 1);

        cout << "\n========= Sorted Bookmarks =========\n";
        for (auto& p : sortedBookmarks)
            cout << "- " << p.title << " (" << p.url << ")\n";
        cout << "====================================\n";
    }

    void searchBookmarks(string title) {
        if (bookmarks.empty()) {
            cout << "\n No bookmarks to search.\n";
            return;
        }

        vector<Page> sortedBookmarks;
        for (auto& b : bookmarks)
            sortedBookmarks.push_back(b.second);

        mergeSort(sortedBookmarks, 0, sortedBookmarks.size() - 1);
        int index = binarySearch(sortedBookmarks, title);

        cout << "\n========= Bookmark Search =========\n";
        if (index != -1)
            cout << " Found: " << sortedBookmarks[index].title << " (" << sortedBookmarks[index].url << ")\n";
        else
            cout << " Bookmark not found.\n";
        cout << "===================================\n";
    }

    // Other Features 
    void viewHistory() {
        if (!historyHead) {
            cout << "\n No browsing history.\n";
            return;
        }
        cout << "\n========= Browsing History =========\n";
        HistoryNode* temp = historyHead;
        while (temp) {
            cout << "- " << temp->page.title << " (" << temp->page.url << ")\n";
            temp = temp->next;
        }
        cout << "====================================\n";
    }

    void showMostVisited() {
        if (visitCount.empty()) {
            cout << "\n No visit data.\n";
            return;
        }
        vector<pair<string, int>> sorted(visitCount.begin(), visitCount.end());
        sort(sorted.begin(), sorted.end(), [](auto& a, auto& b) { return a.second > b.second; });
        cout << "\n========= Most Visited Sites =========\n";
        for (auto& s : sorted)
            cout << "- " << s.first << " (" << s.second << " visits)\n";
        cout << "=====================================\n";
    }
    void showCurrent() {
        Tab* tab = tabs[currentTabIndex];
        cout << "\n===== Current Tab #" << tab->id << " =====\n";
        if (tab->currentPage.url.empty())
            cout << "No page currently open.\n";
        else
            cout << "Current Page: " << tab->currentPage.title << " (" << tab->currentPage.url << ")\n";
        cout << "Back: " << tab->backStack.size() << ", Forward: " << tab->forwardStack.size()
             << ", Total tabs: " << tabs.size() << "\n";
    }

    void saveCurrentSession() {
        string desc;
        cout << "Enter session description: ";
        getline(cin, desc);
        captureSessionSnapshot(desc);
        cout << "\n Session snapshot saved!\n";
    }

    void viewSessionHistory() {
        if (sessionHistory.empty()) {
            cout << "\n No session snapshots recorded.\n";
            return;
        }
        cout << "\n========= Session History =========\n";
        for (int i = 0; i < sessionHistory.size(); i++) {
            char timeStr[100];
            strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", localtime(&sessionHistory[i].timestamp));
            cout << "[" << i << "] " << timeStr << " - " << sessionHistory[i].description << "\n";
            cout << "    Tabs (" << sessionHistory[i].tabData.size() << "): ";
            for (auto& td : sessionHistory[i].tabData)
                cout << "Tab#" << td.first << "(" << td.second << ") ";
            cout << "\n";
        }
        cout << "===================================\n";
    }

    void restoreSession(int index) {
        if (index < 0 || index >= sessionHistory.size()) {
            cout << "\n Invalid snapshot index!\n";
            return;
        }
        for (auto tab : tabs) delete tab;
        tabs.clear();
        for (auto& td : sessionHistory[index].tabData) {
            Tab* newTab = new Tab(td.first);
            if (td.second != "empty") {
                if (bookmarks.find(td.second) != bookmarks.end())
                    newTab->currentPage = bookmarks[td.second];
                else
                    newTab->currentPage = Page(td.second, td.second);
            }
            tabs.push_back(newTab);
            if (td.first >= nextTabId) nextTabId = td.first + 1;
        }
        currentTabIndex = 0;
        cout << "\n Session restored! " << tabs.size() << " tabs reopened.\n";
    }
};

#endif
