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

typedef enum dirItemType {
    DIR_ITEM_FOLDER,
    DIR_ITEM_FILE,
    DIR_ITEM_UNKNOWN,//used when you can't tell the type, for example /here/there
    DIR_ITEM_ERROR
} dirItemType;

class dirItemInfo
{
public:
    long id;
    bool exists;
    dirItemType type;
    long parentId;
    string name;
};

class Path
{

private:
    string path_;

public:
    Path (string path) {
        //if it's a relative path convert it
    }

    inline string getPathExcludingName() {
        return "";
    }

    inline string getPathIncludingName() {
        return "";
    }

    inline string getName() {
        return "";
    }
};

FsContext getInitialContext();

string lsPrettyPrint(FsContext &context, const string path);

vector<File> listFiles(FsContext &context, const string path);

vector<Folder> listFolders(FsContext &context, const string path);

string pwd(FsContext &context);

bool mkdir(FsContext &context, const string fullPath);

bool createFile(FsContext &context, const string fullPath);

bool cd(FsContext &context, const string fullPath);

bool mv(FsContext &context, const string oldPath, const string newPath);

dirItemInfo stat(FsContext &context, const string absolutePath);

vector<string> ls(FsContext &context, const string path);

}

#endif /* VIRTUALFILESYSTEM_H */