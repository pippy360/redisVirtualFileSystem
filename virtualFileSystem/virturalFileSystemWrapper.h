#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include "./hiredis/hiredis.h"

using namespace std;

namespace vfs
{

typedef struct fsContext {
    redisContext context;
    long cwd;
} FsContext;

class file
{
public:
    long id;
    string name;
    long parentId;
}

class folder
{
public:
    long id;
    string name;
    long parentId;
}

string lsPrettyPrint();

vector<File> listFiles(const FsContext *context, string fullPath);

vector<Folder> listFolders(const FsContext *context, string fullPath);

string pwd(const FsContext *context);

bool mkdir(FsContext *context, string fullPath);

bool cd(FsContext *context, string fullPath);

}

#endif /* VIRTUALFILESYSTEM_H */