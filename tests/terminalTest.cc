#include <stdio.h>
#include <string.h>
#include <string>
#include <iostream>
#include "../virtualFileSystem/vfs.h"
using namespace std;


typedef enum command_t {
    command_cp,
    command_pwd,
    command_ls,
    command_mv,
    command_invalid
} command_t;


command_t commandToEnum(string str)
{
    if(strcmp(str.c_str(), "cp") == 0){
        return command_cp;
    }else if(strcmp(str.c_str(), "pwd") == 0){
        return command_pwd;
    }else if(strcmp(str.c_str(), "ls") == 0){
        return command_ls;
    }else if(strcmp(str.c_str(), "mv") == 0){
        return command_mv;
    }
    return command_invalid;
}


void handleCommand(string str){
    command_t v = commandToEnum(str);
    switch(v)
    {
        case command_cp:
            printf("cp command!\n");
            break;
        case command_pwd:
            printf("pwd command!\n");
            break;
        default:
            printf("invalid command!\n");
            ;/*Empty*/
    }
}


int main ()
{
  string str;
  while(1)
  {
    cout << "\nCommand> ";
    cin >> str;
    handleCommand(str);
  }
  return 0;
}
