#ifndef VIRTUALFILESYSTEM_H
#define VIRTUALFILESYSTEM_H

#include "./hiredis/hiredis.h"
#include <string>
#include <vector>

using namespace std;

namespace vfs
{

typedef struct fsContext {
    redisContext context;
    string cwd;
} FsContext;

class File
{
public:
    long id;
    string name;
    long parentId;
};

class Folder
{
public:
    long id;
    string name;
    long parentId;
};

FsContext getInitialContext();

string lsPrettyPrint(FsContext &context, const string path);

vector<File> listFiles(FsContext &context, const string fullPath);

vector<Folder> listFolders(FsContext &context, const string fullPath);

string pwd(FsContext &context);

bool mkdir(FsContext &context, const string fullPath);

bool cd(FsContext &context, const string fullPath);

}

#endif /* VIRTUALFILESYSTEM_H */