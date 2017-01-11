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


#include "./hiredis/hiredis.h"

using namespace std;

namespace vfs
{

string redis_getFileName(redisContext *context, long id);

string redis_getFileWebUrl(redisContext *context, long id);

long redis_getFileSizeFromId(redisContext *context, long id);

string redis_getFolderName(redisContext *context, long id);

long redis_getFolderParentId(redisContext *context, long cwdId);

void redis_setFolderParent(redisContext *context, long dirId, long newParent);

string redis_ls(redisContext *context, long dirId);

}

#endif // !VFS_SAFE_H