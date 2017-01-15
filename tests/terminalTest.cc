#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include <algorithm>
#include <string>
#include "../virtualFileSystem/virtualFileSystemWrapper.h"
#include "../virtualFileSystem/vfs.h"

using namespace std;


typedef enum command_t {
    command_cp,
    command_pwd,
    command_cd,
    command_ls,
    command_mv,
    command_mkdir,
    command_mkfile,
    command_stat,
    command_invalid
} command_t;


command_t commandToEnum(string str)
{
    if(strcmp(str.c_str(), "cp") == 0){
        return command_cp;
    }else if(strcmp(str.c_str(), "cd") == 0){
        return command_cd;
    }else if(strcmp(str.c_str(), "pwd") == 0){
        return command_pwd;
    }else if(strcmp(str.c_str(), "ls") == 0){
        return command_ls;
    }else if(strcmp(str.c_str(), "mv") == 0){
        return command_mv;
    }else if(strcmp(str.c_str(), "mkdir") == 0){
        return command_mkdir;
    }else if(strcmp(str.c_str(), "stat") == 0){
        return command_stat;
    }else if(strcmp(str.c_str(), "mkfile") == 0){
        return command_mkfile;
    }
    return command_invalid;
}


vector<string> split(const string& str, const string& delim)
{
    vector<string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == string::npos) pos = str.length();
        string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

void handleCommand(vfs::FsContext &c, vector<string> strs){
    command_t v = commandToEnum(strs[0]);
    switch(v)
    {
        case command_cp:
            if (strs.size() < 3){
                printf("invalid cp!\n");
                return;         
            }
            cout << "copy from: " << strs[1] << " to: " << strs[2] << endl;
            break;
        case command_cd:
            if (strs.size() < 2)
            {
                cout << "invalid cd args" << endl;
                return;         
            }

            if ( !vfs::cd(c, strs[1]) )
            {
                cout << "invalid cd" << endl;
                return;         
            }
            cout << vfs::pwd(c) << endl;
            break;
        case command_pwd:
            cout << vfs::pwd(c) << endl;
            break;
        case command_ls:
            if (strs.size() < 2)
            {
                cout << vfs::lsPrettyPrint(c, c.cwd) << endl;
            }else{
                cout << vfs::lsPrettyPrint(c, strs[1]) << endl;
            }
            break;
        case command_mkdir:
            if (strs.size() < 2)
            {
                cout << "invalid mkdir args" << endl;
                return;
            }

            if ( !vfs::mkdir(c, strs[1]) )
            {
                cout << "failed to mkdir" << endl;
                return;
            }
            break;
        case command_mkfile:
            if (strs.size() < 2)
            {
                cout << "invalid mkfile args" << endl;
                return;
            }

            if ( !vfs::createFile(c, strs[1]) )
            {
                cout << "failed to mkfile" << endl;
                return;
            }
            break;
        case command_mv:
            if (strs.size() < 3)
            {
                cout << "invalid mv args" << endl;
                return;
            }

            if ( !vfs::mv(c, strs[1], strs[2]) )
            {
                cout << "failed to mv" << endl;
                return;
            }
            break;
        case command_stat:
        {
            if (strs.size() < 2)
            {
                cout << "invalid stat args" << endl;
                return;
            }

            vfs::dirItemInfo fileInfo = vfs::stat(c, strs[1]);
            if (!fileInfo.exists)
            {
                printf("that file doesn't exist");
                return;
            }

            if (fileInfo.type == vfs::DIR_ITEM_FILE){
                printf("it's a file: %s\n", fileInfo.name.c_str());
            }else if (fileInfo.type == vfs::DIR_ITEM_FOLDER){
                printf("it's a folder: %s\n", fileInfo.name.c_str());
            }else{
                printf("it's something else???\n");
            }
            break;
        }
        default:
            printf("invalid command!\n");
            ;/*Empty*/
    }
}


int main ()
{
  string str;
  vfs::FsContext c = vfs::getInitialContext();
  while(1)
  {
    cout << "\nCommand> ";
    std::getline (std::cin,str);
    str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    auto vec = split(str, " ");
    handleCommand(c, vec);
  }
  return 0;
}
