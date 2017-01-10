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

void handleCommand(vfs::FsContext c, vector<string> strs){
    command_t v = commandToEnum(strs[0]);
    switch(v)
    {
        case command_cp:
            if (strs.size() < 3){
                printf("invalid cp!\n");
            }
            cout << "copy from: " << strs[1] << " to: " << strs[2] << endl;
            break;
        case command_cd:
            if (strs.size() < 2)
            {
                cout << "invalid cd args" << endl;                
            }

            if ( !vfs::cd(c, strs[1]) )
            {
                cout << "invalid cd" << endl;                
            }
            cout << vfs::pwd(c) << endl;            
            break;            
        case command_pwd:
            cout << vfs::pwd(c) << endl;            
            break;
        case command_ls:
            cout << vfs::lsPrettyPrint(c) << endl;
            break;
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
