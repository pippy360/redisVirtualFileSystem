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


string redis_getFileName(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FILE_%lu_info name", id);

    string output(reply->str);

	freeReplyObject(reply);
    return output;
}

//FIXME: FIXME: temporary fix here, change APIURL TO WEBURL
string redis_getFileWebUrl(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FILE_%lu_info apiUrl", id);
    string output(reply->str);
	freeReplyObject(reply);
    return output;
}

long redis_getFileSizeFromId(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FILE_%lu_info size", id);
	if (reply->str == NULL) {
		freeReplyObject(reply);
		return -1;
	}
	//printf("we asked for the size of id: %lu and got: %s\n", id, reply->str);
	long size = strtol(reply->str + 1, NULL, 10);//FIXME: the +1 is here because we have the "'s, get rid of that
	return size;
}

//^see get file name
string redis_getFolderName(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FOLDER_%lu_info name", id);

    string output(reply->str);

	freeReplyObject(reply);
    return output.substr(1, output.size()-2);
}

//this only works with folders for the moment
long redis_getFolderParentId(redisContext *context, long cwdId) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FOLDER_%lu_info parent", cwdId);
	//printf("the command we ran HGET FOLDER_%lu_info parent\n", cwdId);
	long newId = strtol(reply->str, NULL, 10);
	freeReplyObject(reply);
	//printf("the result %ld\n", newId);
	return newId;
}

void redis_setFolderParent(redisContext *context, long dirId, long newParent) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HSET FOLDER_%lu_info parent %ld", dirId,
			newParent);
	freeReplyObject(reply);
}

string redis_ls(redisContext *context, long dirId) {
	int j = 0;
	long id;
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "LRANGE FOLDER_%lu_folders 0 -1", dirId);
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < reply->elements; j++) {
			id = strtol(reply->element[j]->str, NULL, 10);
			// redis_getFolderName(context, id, name, MAX_FILENAME_SIZE);
			// printf("ls: %s\n", name);
		}
	}
	freeReplyObject(reply);
	reply = (redisReply *) redisCommand(context, "LRANGE FOLDER_%lu_files   0 -1", dirId);
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (j = 0; j < reply->elements; j++) {
			long id = strtol(reply->element[j]->str, NULL, 10);
			// redis_getFileName(context, id, name, MAX_FILENAME_SIZE);
			// printf("ls: %s\n", name);
		}
	}
	freeReplyObject(reply);
}

}