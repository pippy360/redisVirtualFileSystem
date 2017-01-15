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
#include "virtualFileSystemWrapper.h"

#include "./hiredis/hiredis.h"

using namespace std;

namespace vfs
{


string redis_getFileName(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FILE_%lu_info name", id);

    string output(reply->str);

	freeReplyObject(reply);
    return output.substr(1,output.size()-2);
}

long redis_getFolderParentId(redisContext *context, long cwdId) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FOLDER_%lu_info parent", cwdId);
	if(reply->str == NULL){
		return -1;
	}
	long newId = strtol(reply->str, NULL, 10);
	freeReplyObject(reply);
	return newId;
}

long redis_getFileParentId(redisContext *context, long cwdId) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FILE_%lu_info parent", cwdId);
	if(reply->str == NULL){
		return -1;
	}
	long newId = strtol(reply->str, NULL, 10);
	freeReplyObject(reply);
	return newId;
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

vector<long> redis_getFolderIds(redisContext *context, long id) {
	vector<long> ret;
	redisReply *reply;
	long tempId;
	int i = 0;
	reply = (redisReply *) redisCommand(context, "LRANGE FOLDER_%lu_folders 0 -1", id);
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (i = 0; i < reply->elements; i++) {
			tempId = strtol(reply->element[i]->str, NULL, 10);
			ret.push_back(tempId);
		}
	}
	return ret;
}

vector<long> redis_getFileIds(redisContext *context, long id) {
	vector<long> ret;
	redisReply *reply;
	long tempId;
	int i = 0;
	reply = (redisReply *) redisCommand(context, "LRANGE FOLDER_%lu_files 0 -1", id);
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (i = 0; i < reply->elements; i++) {
			tempId = strtol(reply->element[i]->str, NULL, 10);
			ret.push_back(tempId);
		}
	}
	return ret;
}

//^see get file name
string redis_getFolderName(redisContext *context, long id) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HGET FOLDER_%lu_info name", id);
    string output(reply->str);

	freeReplyObject(reply);
    return output.substr(1, output.size()-2);
}

Folder redis_getFolder(redisContext *context, long id) {
	Folder ret;
	ret.id = id;
	ret.name = redis_getFolderName(context, id);
	ret.parentId = redis_getFolderParentId(context, id);
	return ret;
}

vector<Folder> redis_getFolders(redisContext *context, long id) {
	vector<Folder> output;
	auto ids = redis_getFolderIds(context, id);
	for (auto id: ids)
	{
		output.push_back( redis_getFolder(context, id) );
	}
	return output;
}

File redis_getFile(redisContext *context, long id) {
	File ret;
	ret.id = id;
	ret.name = redis_getFileName(context, id);
	ret.parentId = redis_getFileParentId(context, id);
	return ret;
}

vector<File> redis_getFiles(redisContext *context, long id) {
	vector<File> output;
	auto ids = redis_getFileIds(context, id);
	for (auto id: ids)
	{
		output.push_back( redis_getFile(context, id) );
	}
	return output;
}

void redis_setFolderParent(redisContext *context, long dirId, long newParent) {
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "HSET FOLDER_%lu_info parent %ld", dirId,
			newParent);
	freeReplyObject(reply);
}


void redis_mvFileToFolderWithRename(redisContext *context, const char *filePath, 
	const char *targetFolderAbsolutePath, const char *newFileName)
{
	//atomically{
		//unlink it
		//change the name
		//link it
	//}
}

void redis_mvOverwriteFile(redisContext *context, const char *filePath, 
	const char *targetFolderAbsolutePath)
{
	//check if the new file already exists
	//if so overwrite
	//
	//else create
	//
}

void redis_mvOverwriteFolder(redisContext *context, const char *filePath, 
	const char *targetFolderAbsolutePath)
{
	//check if the new file already exists
	//if so overwrite
	//
	//else create
	//
}

void redis_getDirItemInfo()
{
	
}

long redis_findFileIdInFolder(redisContext *context, long folderId, const string filename)
{

}

int redis_isFile(redisContext *context, long objId)
{
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "EXISTS FILE_%lu_info", objId);
	long newId = (int) reply->integer;
	freeReplyObject(reply);
	return newId;
}

int redis_isDirectory(redisContext *context, long objId)
{
	redisReply *reply;
	reply = (redisReply *) redisCommand(context, "EXISTS FOLDER_%lu_info", objId);
	long newId = (int) reply->integer;
	freeReplyObject(reply);
	return newId;
}

}