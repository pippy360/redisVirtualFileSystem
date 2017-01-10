#include "virtualFileSystemWrapper.h"
#include "./hiredis/hiredis.h"
#include <string>
#include <stdlib.h>
extern "C" { 
#include "vfs.h"
#include "vfsPathParser.h"
}
using namespace std;

namespace vfs
{

    
long getFolderIdFromAbsolutePath(FsContext &context, const string absolutePath)
{
    //assert first character is '/'
    return vfs_getDirIdFromPath(&(context.context), ROOT_FOLDER_ID, absolutePath.c_str(), absolutePath.size());
}

long getFolderIdFromPath(FsContext &context, const string path)
{
    long cwd = getFolderIdFromAbsolutePath(context, path);
    return vfs_getDirIdFromPath(&(context.context), cwd, path.c_str(), path.size());
}

bool isExistingDirectory(FsContext &context, const string path)
{

}

FsContext getInitialContext()
{
    FsContext context;
    context.cwd = "/";

    unsigned int j;
	redisContext *c;
	redisReply *reply;
	const char *hostname = "127.0.0.1";
	int port = 6379;

	struct timeval timeout = { 1, 500000 }; // 1.5 seconds
	c = redisConnectWithTimeout(hostname, port, timeout);
	if (c == NULL || c->err) {
		if (c) {
			printf("Connection error: %s\n", c->errstr);
			redisFree(c);
		} else {
			printf("Connection error: can't allocate redis context\n");
		}
		exit(1);
	}

	printf("building the database\n");
	;
	vfs_buildDatabase(c);
    context.context = *c;
    return context;
}

string lsPrettyPrint(FsContext &context)
{
    return "prettyPrint";
}

vector<File> listFiles(FsContext &context, const string path)
{
    long cwdId = getFolderIdFromAbsolutePath(context, path);     
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, path.c_str(), path.size(), cwdId) == -1){
        return vector<File>();//FIXME: throw error
    }
    //TODO: make sure it's a directory
    if (parserState.isDir)
    {
        vfs_ls(&(context.context), parserState.id);
    }
    return vector<File>();
}

vector<Folder> listFolders(FsContext &context, const string fullPath)
{
    return vector<Folder>();
}

string pwd(FsContext &context)
{
    return context.cwd;
}

bool mkdir(FsContext &context, const string path)
{
    long cwdId = getFolderIdFromAbsolutePath(context, path);
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, path.c_str(), path.size(), cwdId) == -1){
        return false;
    }

    vfs_mkdir(&(context.context), parserState.parentId, path.c_str() + parserState.nameOffset);
    return true;
}

bool cd(FsContext &context, const string path)
{
    long cwdId = getFolderIdFromAbsolutePath(context, path);
    long dirId = -1;

    long cwdId = getFolderIdFromAbsolutePath(context, path);
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, path.c_str(), path.size(), cwdId) == -1){
        return false;
    }

    if()

    if( (dirId = vfs_getDirIdFromPath(&(context.context), cwdId, path.c_str(), path.size())) == -1 ){
        //a dir with that path does not exist
        printf("invalid\n");
        return false;
    }

    printf("valid %ld\n", dirId);
    context.cwd = path;

    return true;
}

}