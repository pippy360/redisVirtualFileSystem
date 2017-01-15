#ifndef VFS_SAFE_H
#define VFS_SAFE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <stdlib.h>
#include <vector>

#include "./hiredis/hiredis.h"

using namespace std;

namespace vfs
{

string redis_getFileName(redisContext *context, long id);

string redis_getFileWebUrl(redisContext *context, long id);

long redis_getFileSizeFromId(redisContext *context, long id);

string redis_getFolderName(redisContext *context, long id);

long redis_getFolderParentId(redisContext *context, long cwdId);

long redis_getFileParentId(redisContext *context, long cwdId);

void redis_setFolderParent(redisContext *context, long dirId, long newParent);

string redis_ls(redisContext *context, long dirId);

vector<Folder> redis_getFolders(redisContext *context, long id);

vector<File> redis_getFiles(redisContext *context, long id);

int redis_isDirectory(redisContext *context, long objId);

int redis_isFile(redisContext *context, long objId);

vector<long> redis_getFolderIds(redisContext *context, long id);

vector<long> redis_getFileIds(redisContext *context, long id);

}

#endif // !VFS_SAFE_H