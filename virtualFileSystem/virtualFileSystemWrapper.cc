#include "virtualFileSystemWrapper.h"
#include "./hiredis/hiredis.h"
#include <string>
#include <stdlib.h>
#include <stdio.h>
extern "C" { 
#include "vfs.h"
#include "vfsPathParser.h"
}
#include "vfs_safe.h"
#include <iostream>
#include <regex>

using namespace std;

namespace vfs
{


bool isValidPath(string initialPath)
{
    return true;//FIXME:
    // regex txt_regex("[a-zA-Z0-9]*");
    // smatch pieces_match;
 
    // return (std::regex_match(initialPath, pieces_match, txt_regex));
}

long getParentIdById(FsContext &context, const long childId)
{
    return redis_getFolderParentId(&(context.context), childId);
}

long getDirIdFromPath(FsContext &context, long cwd, const string path){
    auto pathCpy = path;
    if (pathCpy.back() != '/')
    {
        pathCpy = path + "/";
    }
    return vfs_getDirIdFromPath(&(context.context), cwd, pathCpy.c_str(), pathCpy.size());
}

long getFolderIdFromAbsolutePath(FsContext &context, const string absolutePath)
{
    //assert: first character MUST BE '/'
    return getDirIdFromPath(context, ROOT_FOLDER_ID, absolutePath);
}


long getCwdId(FsContext &context)
{
    return getFolderIdFromAbsolutePath(context, context.cwd);
}

long getFolderIdFromPath(FsContext &context, const string path)
{
    long cwd = getCwdId(context);
    return getDirIdFromPath(context, cwd, path);
}

long getFileIdFromPath(FsContext &context, const string path)
{
    long cwd = getCwdId(context);
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, path.c_str(), path.size(), cwd) == -1 )
    {
        return -1;
    }
    if(!parserState.isFilePath){
        return -1;
    }

    return parserState.id;
}

//throws exception for root
string getNonRootFolderName(FsContext &context, long folderId)
{
    if (folderId == ROOT_FOLDER_ID)
    {
        throw "Cannot get name of root folder.";
    }
    return redis_getFolderName(&(context.context), folderId);
}

string getAbsolutePathFromFolderId(FsContext &context, const long folderId)
{
    if(folderId == ROOT_FOLDER_ID)
    {
        return "/";
    }

	long currentId = folderId;
    string output = "";
	while (currentId != 0) {
		string folderName = getNonRootFolderName(context, currentId);
        output = "/" + folderName + output;

        currentId = getParentIdById(context, currentId);
	}
    return output + "/";
}

//removes "./" or "/"
string removeAnyPathPrefix(string path)
{
    if(path.at(0) == '/')
    {
        return path.substr(1, path.size()-1 );
    }else if(path.substr(0, 2) == "./"){
        return path.substr(2, path.size() - 2);
    }else{
        return path;
    }
}

long getParentIdOfLeftSideOfExpression(FsContext &context, const string initialPath)
{
    if(initialPath.size() <= 0 || ! isValidPath(initialPath))
    {
        throw "Could not get parent of invalid path: " + initialPath + "\n";
    }

    if(initialPath.at(0) == '/')
    {
        return ROOT_FOLDER_ID;
    }else{
        return getFolderIdFromPath(context, context.cwd);
    }
}

// cd here becomes cd /home/user/here
// cd /here becomes cd /here
// cd ./here becomes cd /home/user/here
// cd one/two/three becomes cd /home/user/one/two/three
string formatToValidPath(FsContext &context, const string initialPath)
{
    long parentId = getParentIdOfLeftSideOfExpression(context, initialPath);
    if (parentId == -1)
    {
        throw "failed to get left side of expression";
    }

    string leftSide = getAbsolutePathFromFolderId(context, parentId);
    
    string rightSide = removeAnyPathPrefix(initialPath);

    return leftSide + rightSide;
}

FsContext getInitialContext()
{
    FsContext context;
    context.cwd = "/";

	redisContext *c;
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
	if ( !isVirtualFileSystemCreated(c) ) {
        vfs_buildDatabase(c);
    }
    context.context = *c;
    return context;
}

vector<File> getFiles(FsContext &context, const string path)
{
    long id = getFolderIdFromPath(context, path);
    return redis_getFiles(&(context.context), id);
}

vector<Folder> getFolders(FsContext &context, const string path)
{
    long id = getFolderIdFromPath(context, path);
    return redis_getFolders(&(context.context), id);
}

string lsPrettyPrint(FsContext &context, const string path)
{
    string output;
    auto files = getFiles(context, path);
    auto folders = getFolders(context, path);
    for (auto folder: folders)
    {
        output += folder.name + "\n";
    }
    for (auto file: files)
    {
        output += file.name + "\n";
    }
    return output;
}

string pwd(FsContext &context)
{
    return context.cwd;
}

//PUBLIC
bool mkdir(FsContext &context, const string initPath)
{
    string formattedPath = formatToValidPath(context, initPath);
    
    long cwdId = getCwdId(context);
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, formattedPath.c_str(), formattedPath.size(), cwdId) == -1){
        return false;
    }
    if( vfs_mkdir(&(context.context), parserState.parentId, formattedPath.c_str() + parserState.nameOffset) == -1){
        cout << "mkdir failed when calling redis command" << endl;
    }
    return true;
}

//PUBLIC
bool cd(FsContext &context, const string path)
{
    string formattedPath = formatToValidPath(context, path);
    
    //check if it exists and it's a valid Folder
    long folderId = -1;
    if( (folderId = getFolderIdFromPath(context, formattedPath)) == -1 ){
        return false;
    }

    context.cwd = getAbsolutePathFromFolderId(context, folderId);
    return true;
}

bool createFile(FsContext &context, const string path)
{
    string formattedPath = formatToValidPath(context, path);

    long cwdId = getCwdId(context);
    vfsPathParserState_t parserState;
    if( vfs_parsePath(&(context.context), &parserState, formattedPath.c_str(), formattedPath.size(), cwdId) == -1) {
        return false;
    }
    //if( vfs_mkdir(&(context.context), parserState.parentId, formattedPath.c_str() + parserState.nameOffset) == -1) {
    if( vfs_createFile(&(context.context), parserState.parentId, formattedPath.c_str() + parserState.nameOffset, 1, "none", "https://none.com", "http://none") == -1)
    {
        cout << "mkdir failed when calling redis command" << endl;
    }
    return true;
}

vector<long> listDirFileAndFolderIds(FsContext &context, long dirId){
    vector<long> a = redis_getFolderIds(&(context.context), dirId);
    vector<long> b = redis_getFileIds(&(context.context), dirId);
    a.insert(end(a), begin(b), end(b));
    return a;
}

dirItemInfo getDirItemInfo(FsContext &context, long objId){

    dirItemInfo ret;
    ret.id = -1;
    ret.exists = false;

    if( redis_isDirectory(&(context.context), objId) ){
        ret.id = objId;
        ret.exists = false;
        ret.type = DIR_ITEM_FOLDER;
        ret.parentId = redis_getFolderParentId(&(context.context), ret.id);
        ret.name = redis_getFolderName(&(context.context), ret.id);
    }else{
        ret.id = objId;
        ret.exists = false;
        ret.type = DIR_ITEM_FILE;
        ret.parentId = redis_getFileParentId(&(context.context), ret.id);
        ret.name = redis_getFileName(&(context.context), ret.id);
    }

    return ret;
}

vector<dirItemInfo> ls_info(FsContext &context, long dirId){
    vector<dirItemInfo> ret;
    auto ids = listDirFileAndFolderIds(context, dirId);
    for (auto id: ids)
    {
        dirItemInfo info = getDirItemInfo(context, id);
        ret.push_back(info);
    }
    return ret;
}

vector<string> ls(FsContext &context, long dirId){
    vector<string> ret;
    auto ids = listDirFileAndFolderIds(context, dirId);
    for (auto id: ids)
    {
        dirItemInfo info = getDirItemInfo(context, id);
        ret.push_back(info.name);
    }
    return ret;
}

vector<string> ls(FsContext &context, const string path){
    long id = getFolderIdFromPath(context, path);
    return ls(context, id);
}

dirItemInfo getDirItemInfo(FsContext &context, const string absolutePath){

    dirItemInfo ret;
    ret.id = -1;
    ret.exists = false;
    
    //look for a file using that path
    if (absolutePath.back() != '/')
    {
        if ( (ret.id = getFileIdFromPath(context, absolutePath)) == -1 )
        {
            //set the errorno
            //printf("Couldn't find a file by that name.\n");
        }else{
            ret.type = DIR_ITEM_FILE;
        }
    }else{
        printf("skipped the check for a file id\n");
    }

    //look for a folder using that path
    if (ret.id == -1)
    {
        if ( (ret.id = getFolderIdFromPath(context, absolutePath)) == -1 )
        {
            printf("Couldn't find a file or folder by that name.\n");
            return ret;
        }else{
            ret.type = DIR_ITEM_FOLDER;
        }
    }

    ret.exists = true;
    if(ret.type == DIR_ITEM_FILE){
        ret.name = redis_getFileName(&(context.context), ret.id);
    }else{
        ret.name = redis_getFolderName(&(context.context), ret.id);        
    }    
    return ret;
}

//converts relative path to absolute path


bool mv(FsContext &context, const string oldPath, const string newPath)
{
    // long cwdId = getCwdId(context);
    // if () {

    // }
    //validate and format the paths
    //get the paths
    //validate the path before the name 

    //check the errorno
    string oldValidatedAbsolutePath, newValidatedAbsolutePath;
    
    //then call
    //redis_mvWithValidatedAbsolutePaths(&(context.context), oldValidatedAbsolutePath, newValidatedAbsolutePath)
    return true;
}

dirItemInfo stat(FsContext &context, const string absolutePath){
    return getDirItemInfo(context, absolutePath);
}

}