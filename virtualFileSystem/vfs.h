
#include "./hiredis/hiredis.h"

void vfs_buildDatabase(redisContext *context);

int isVirtualFileSystemCreated(redisContext *context);

void vfs_ls(redisContext *context, long dirId);

long vfs_mkdir(redisContext *context, long parentId, const char *name);

char *vfs_listUnixStyle(redisContext *context, long dirId);

long vfs_createFile(redisContext *context, long parentId, const char *name, long size,
		const char *googleId, const char *webUrl, const char *apiUrl);

void vfs_getFileName(redisContext *context, long id, char *outputNameBuffer,
		int outputNameBufferLength);

void vfs_getFileWebUrl(redisContext *context, long id, char *outputNameBuffer,
		int outputNameBufferLength);

long vfs_findFileNameInDir(redisContext *context, long dirId, const char *fileName,
		int fileNameLength);

