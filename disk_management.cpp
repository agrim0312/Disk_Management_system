#include<bits/stdc++.h>
#include <iostream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <Windows.h>
#include <cstdio>
#include <iomanip>
#include <wincrypt.h>
#include <cstring>
#include <iostream>

using namespace std;

// Function to get size of a disk
bool GetDiskSize(const string& driveLetter, ULARGE_INTEGER& totalSize, ULARGE_INTEGER& freeSpace) {
    wchar_t rootPath[4];
    rootPath[0] = driveLetter[0];
    rootPath[1] = ':';
    rootPath[2] = '\\';
    rootPath[3] = '\0';

    return GetDiskFreeSpaceExW(rootPath, &freeSpace, &totalSize, nullptr) != 0;
}
// Function to get size of a disk
void GetDiskSize(const string& driveLetter) {
    ULARGE_INTEGER totalSize, freeSpace;
    string driveLetterReq = driveLetter;

    if (GetDiskSize(driveLetterReq, totalSize, freeSpace)) {
        // Convert to human-readable format (optional)
        double totalGB = static_cast<double>(totalSize.QuadPart) / (1024 * 1024 * 1024);
        double freeGB = static_cast<double>(freeSpace.QuadPart) / (1024 * 1024 * 1024);
        cout <<"Total disk size on drive: "<<totalGB << " GB\n";
        cout <<"free disk space on drive: "<<freeGB << " GB\n";
    } else {
        cerr << "Error getting disk size. Error code: " << GetLastError() << "\n";
    }
}
// Function to provide a breakdown of space utilization based on file types
void breakdownSpaceUtilization(const string& path,unordered_map<string, uintmax_t>& fileTypes) {
    uintmax_t totalSize = 0;
    if(path.empty() || path[0]=='\0'){
        cout << "Invalid path" << endl;
        return;
    }
    // cout<<path<<"-";
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return;
    }

    struct dirent* enter;
    while ((enter = readdir(dir))) {
        if(enter==NULL || enter->d_name==NULL){
            cout<<"Error in reading directory"<<endl;
            break;
        }
        if (strcmp(enter->d_name, ".") == 0 || strcmp(enter->d_name, "..") == 0) {
            // Skip "." and ".." entries to avoid infinite loops
            continue;
        }
        string filePath = path + "/" + enter->d_name;
        // cout<<filePath<<endl;
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                // If it's a directory, recursively call the function on it
                breakdownSpaceUtilization(filePath.c_str(),fileTypes);
            }
            else if((fileInfo.st_mode & S_IFMT) == S_IFMT){
                continue;
            }
            else if(S_ISREG(fileInfo.st_mode)) {
                string exten = "";
                char* extension = strrchr(enter->d_name, '.');
                if(extension==NULL){
                    continue;
                }
                else{
                    exten = string(extension);
                }
                if (!exten.empty()) {
                    fileTypes[exten] += fileInfo.st_size;
                    totalSize += fileInfo.st_size;
                }
            }
        }
    }

    closedir(dir);

    cout << "Space utilization breakdown:"<<path<< endl;
    for (const auto& pair : fileTypes) {
        cout << pair.first << ": " << pair.second << " bytes (" << ((double)pair.second / totalSize) * 100 << "%)" << endl;
    }
}
// Function to detect duplicate files
void detectDuplicateFiles(const string& path) {
    if(path.empty() || path[0]=='\0'){
        cout << "Invalid path" << endl;
        return;
    }
    unordered_map<uintmax_t, vector<string>> fileSizes;
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return;
    }

    struct dirent* enter;
    while ((enter = readdir(dir))) {
        if(enter==NULL || enter->d_name==NULL){
            cout<<"Error in reading directory"<<endl;
            break;
        }
        if (strcmp(enter->d_name, ".") == 0 || strcmp(enter->d_name, "..") == 0) {
            // Skip "." and ".." entries to avoid infinite loops
            continue;
        }
        string filePath = path + "/" + enter->d_name;
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                // If it's a directory, recursively call the function on it
                detectDuplicateFiles(filePath.c_str());
            }
            if (S_ISREG(fileInfo.st_mode)) {
                fileSizes[fileInfo.st_size].push_back(filePath);
            }
        }
    }
    for (const auto& pair : fileSizes) {
        if (pair.second.size() > 1) {
            // Check for duplicates based on last modification time (optional)
            unordered_map<long long, vector<string>> modifiedTimeGroups;
            for (const auto& filePath : pair.second) {
                struct stat fileInfo;
                if (stat(filePath.c_str(), &fileInfo) == 0) {
                    long long modifiedTime = fileInfo.st_mtime;
                    modifiedTimeGroups[modifiedTime].push_back(filePath);
                }
            }

            for (const auto& modifiedPair : modifiedTimeGroups) {
                if (modifiedPair.second.size() > 1) {
                    cout << "Duplicate files found (size " << pair.first << " bytes, modified time " << modifiedPair.first << "):" << endl;
                    for (const auto& file : modifiedPair.second) {
                        cout << " - " << file << endl;
                    }
                    cout << endl;
                }
            }
        }
    }
    closedir(dir);
}
// Function to identify large files
void identifyLargeFiles(const string& path, uintmax_t threshold) {
    cout << "Large files (size greater than " << threshold << " bytes):" << endl;
    if(path.empty() || path[0]=='\0'){
        cout << "Invalid path" << endl;
        return;
    }
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return;
    }

    struct dirent* enter;
    while ((enter = readdir(dir))) {
        if(enter==NULL || enter->d_name==NULL){
            cout<<"Error in reading directory"<<endl;
            continue;
        }
        if (strcmp(enter->d_name, ".") == 0 || strcmp(enter->d_name, "..") == 0) {
            // Skip "." and ".." entries to avoid infinite loops
            continue;
        }
        string filePath = path + "/" + enter->d_name;
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                // If it's a directory, recursively call the function on it
                identifyLargeFiles(filePath.c_str(),threshold);
            }
            else if((fileInfo.st_mode & S_IFMT) == S_IFMT){
                continue;
            }
            else if (S_ISREG(fileInfo.st_mode) && fileInfo.st_size >= threshold) {
                cout << filePath << " - " << fileInfo.st_size << " bytes" << endl;
            }
        }
    }

    closedir(dir);
}
//Function to get count of specific file types
void scanSpecificFileTypes(const string& path, const string& fileType) {
    cout << "Counting " << fileType << " files:" << endl;
    if(path.empty() || path[0]=='\0'){
        cout << "Invalid path" << endl;
        return;
    }
    unordered_map<string, uintmax_t> fileTypes;

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return;
    }

    struct dirent* enter;
    while ((enter = readdir(dir))) {
        if(enter==NULL || enter->d_name==NULL){
            cout<<"Error in reading directory"<<endl;
            break;
        }
        if (strcmp(enter->d_name, ".") == 0 || strcmp(enter->d_name, "..") == 0) {
            // Skip "." and ".." entries to avoid infinite loops
            continue;
        }
        string filePath = path + "/" + enter->d_name;
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                // If it's a directory, recursively call the function on it
                scanSpecificFileTypes(filePath.c_str(),fileType);
            }
            else if((fileInfo.st_mode & S_IFMT) == S_IFMT){
                continue;
            }
            if (S_ISREG(fileInfo.st_mode)) {
                string exten = "";
                char* extension = strrchr(enter->d_name, '.');
                if(extension==NULL){
                    continue;
                }
                else{
                    exten = string(extension);
                }
                if (!exten.empty()) {
                    // cout<<extension<<endl;
                    fileTypes[exten]++;
                }
            }
        }
    }

    closedir(dir);

    cout<<"Number of files of type "<<fileType<<"in"<<path<<" are "<<fileTypes[fileType]<<endl;

}
//Function to delete specific file or folder
void deleteFile(const string& path){
    cout<<"Do you want to delete this file (Y/N)";
    char choice;
    cin>>choice;
    if(choice=='Y'||choice=='y')
    {
        remove(path.c_str());
    }
    cout<<"File deleted successfully"<<endl;
}
//Function to enable user to delete files of specific type
void deleteSpecificFileTypes(const string& path, const string& fileType,char& choice) {
    if(path.empty() || path[0]=='\0'){
        cout << "Invalid path" << endl;
        return;
    }
    unordered_map<string, vector<string>> fileTypes;

    DIR* dir = opendir(path.c_str());
    if (!dir) {
        cerr << "Error opening directory: " << path << endl;
        return;
    }
    struct dirent* enter;
    while ((enter = readdir(dir))) {
        if(enter==NULL || enter->d_name==NULL){
            cout<<"Error in reading directory"<<endl;
            break;
        }
        if (strcmp(enter->d_name, ".") == 0 || strcmp(enter->d_name, "..") == 0) {
            // Skip "." and ".." entries to avoid infinite loops
            continue;
        }
        string filePath = path + "/" + enter->d_name;
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0) {
            if (S_ISDIR(fileInfo.st_mode)) {
                // If it's a directory, recursively call the function on it
                deleteSpecificFileTypes(filePath.c_str(),fileType,choice);
            }
            else if (S_ISREG(fileInfo.st_mode)) {
                string exten = "";
                char* extension = strrchr(enter->d_name, '.');
                if(extension==NULL){
                    continue;
                }
                else{
                    exten = string(extension);
                }
                if (!exten.empty()) {
                    // cout<<extension<<endl;
                    fileTypes[exten].push_back(filePath);
                }
            }
        }
    }

    closedir(dir);
    // cout<<"Number of files of type "<<fileType<<" are "<<fileTypes[fileType]<<endl;
    if(choice=='Y'||choice=='y')
        {
            for(auto& file : fileTypes[fileType]){
                remove(file.c_str());
            }
        }
        cout<<"Files of type "<<fileType<<" deleted successfully"<<endl;
}
//Function to get user choice
int getUserChoice()
{
    int choice;
    cout << "Choose a function to call:" << endl;
    cout << "1. GetDiskSize" << endl;
    cout << "2. breakdownSpaceUtilization" << endl;
    cout << "3. detectDuplicateFiles" << endl;
    cout << "4. scanSpecificFileTypes" << endl;
    cout << "5. identifyLargeFiles" << endl;
    cout << "6. deleteFile" << endl;
    cout << "7. deleteSpecificFileTypes" << endl;
    cout << "Enter your choice (1, 2, 3, 4, 5, 6 or 7): ";
    cin >> choice;
    return choice;
}

// Driver function

int main() {
    
    string path;
    unordered_map<string, uintmax_t> fileTypes;
    // unordered_map<uintmax_t, vector<string>> fileSizes;
    cout << "Enter the path to analyze: ";
    cin >> path;
    int choice = getUserChoice();
    string extension;
    uintmax_t threshold;
    string filePathtoDelete;
    string extension2;

    switch(choice) {
        case 1:
            GetDiskSize(path.substr(0,3));
            break;
        case 2:
            breakdownSpaceUtilization(path,fileTypes);
            break;
        case 3:
            detectDuplicateFiles(path);
            break;
        case 4:
            cout<<"Enter the file type to scan for: ";
            cin >> extension;
            scanSpecificFileTypes(path, extension);
            break;
        case 5:
            cout << "Enter the threshold size for identifying large files (in bytes): ";
            cin >> threshold;
            identifyLargeFiles(path, threshold);
            break;
        case 6:
            cout<<"Enter the file path to delete: ";
            cin>>filePathtoDelete;
            deleteFile(filePathtoDelete);
            break;
        case 7:
            cout<<"Enter the file type to delete: ";
            cin>>extension2;
            char choice2;
            cout<<"Do you want to delete these files of file type specified above (Y/N)";
            cin>>choice;
            deleteSpecificFileTypes(path,extension2,choice2);
            break;
        default:
            cout << "Invalid choice" << endl;
            break;
    }
    return 0;
}
