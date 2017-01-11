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
    //assert first character is '/'
    return getDirIdFromPath(context, ROOT_FOLDER_ID, absolutePath);
}

long getFolderIdFromPath(FsContext &context, const string path)
{
    long cwd = getFolderIdFromAbsolutePath(context, path);
    return getDirIdFromPath(context, cwd, path);
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
    cout << "formatter, parentid: " << parentId << endl;
    if (parentId == -1)
    {
        throw "failed to get left side of expression";
    }

    string leftSide = getAbsolutePathFromFolderId(context, parentId);
    cout << "formatter, leftSide: " << leftSide << endl;
    
    string rightSide = removeAnyPathPrefix(initialPath);

    cout << "formatter, rightSize: " << rightSide << endl;

    return leftSide + rightSide;
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

bool mkdir(FsContext &context, const string initPath)
{
    string formattedPath = formatToValidPath(context, initPath);
    
    long cwdId = getFolderIdFromAbsolutePath(context, formattedPath);
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
    
    //DEBUG
    cout << "CD formattedPath: " << formattedPath << endl;
    //DEBUG

    //check if it exists and it's a valid Folder
    long folderId = -1;
    if( (folderId = getFolderIdFromPath(context, formattedPath)) == -1 ){
        return false;
    }

    context.cwd = getAbsolutePathFromFolderId(context, folderId);
    return true;
}

}