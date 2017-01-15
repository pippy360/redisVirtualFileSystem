#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "./hiredis/hiredis.h"
#include "vfs.h"
#include "vfsPathParser.h"

#define MAX_FILENAME_SIZE 1000

void vfs_debug_printParserState(vfsPathParserState_t *parserState) {
	printf("id's of oldpath %ld - %ld\n", parserState->parentId,
			parserState->id);
	printf("id                  %ld\n", parserState->id);
	printf("parent Id           %ld\n", parserState->parentId);
	printf("isFile              %d\n", parserState->isFilePath);
	printf("isDir               %d\n", parserState->isDir);
	printf("nameLength          %d\n", parserState->nameLength);
	printf("Existing Object:      %d\n", parserState->isExistingObject);
}

void init_vfsPathParserState(vfsPathParserState_t *parserState) {
	parserState->error = 0;
	parserState->id = -1;
	parserState->isDir = 0;
	parserState->isFilePath = 0;
	parserState->isValid = 0;
	parserState->nameLength = 0;
	parserState->nameOffset = 0;
	parserState->parentId = -1;
	parserState->isExistingObject = 1;
}

//finds parserState->namePtr
//returns 0 if success, non-0 otherwise
int vfs_seperatePathAndName(vfsPathParserState_t *parserState,
		const char *fullFilePath, int fullFilePathLength) {

	int i = 0;
	int containsFileName; //boolean, does it end in a '/' or a file name
	const char *currPos = fullFilePath + fullFilePathLength - 1;

	for (i = 0; currPos >= fullFilePath && *currPos != '/'; i++) {
		currPos--;
	}
	if (currPos < fullFilePath) {
		parserState->nameLength = fullFilePathLength;
		parserState->nameOffset = 0;
	} else {
		parserState->nameLength = i;
		parserState->nameOffset = (currPos + 1) - fullFilePath;
	}

	return 0;
}

//this finds a "name" in a directory (either a file or a folder),
//fills in the id and object type (isDir/isFile) in the parserState
//returns 0 if success, non-0 otherwise
//works for '.' and '..'
int vfs_findObjectInDir(redisContext *context,
		vfsPathParserState_t *parserState, long dirId, const char *name,
		int nameLength) {
	long resultId;
	int isFile = 0, isDir = 0;
	
	if ((resultId = vfs_findFileNameInDir(context, dirId, name, nameLength))
			!= -1) {
		isFile = 1;
	} else if ((resultId = vfs_findDirNameInDir(context, dirId, name,
			nameLength)) != -1) {
		isDir = 1;
	}
	
	if (resultId == -1) {
		return -1;
	}
	parserState->id = resultId;
	parserState->isFilePath = isFile;
	parserState->isDir = isDir;

	return 0;
}

long getLengthOfNextName(const char *remainingPath, long remainingPathLength){
	int i;
	for (i = 0; i < remainingPathLength && remainingPath[i] != '/'; i++) {
		;
	}
	return i;
}

//will follow the string until the last '/' and will ignore any characters after that
long vfs_getDirIdFromPath(redisContext *context, long userCwd, const char *path,
		int pathLength) {
	//printf("the path length %d\n", pathLength);
	const char *currPtr = path;
	const char *nameStart;
	int result;
	long currDir = userCwd;
	vfsPathParserState_t parserState;
	init_vfsPathParserState(&parserState);

	if (*currPtr == '/') {
		currPtr++;
		currDir = 0; //FIXME: REPLACE WITH ROOTid
	}
	while (1) {
		nameStart = currPtr;
		long remainingLength = pathLength - (currPtr-path);
		long lengthOfNextName = getLengthOfNextName(currPtr,
				remainingLength);
		currPtr += lengthOfNextName;
		if (currPtr >= path + pathLength) {
			break;
		}
		currPtr++; //skip over the '/'

		result = vfs_findObjectInDir(context, &parserState, currDir, nameStart,
				lengthOfNextName);
		//printf("the current name of the folder we're processing %lu %.*s\n", lengthOfNextName, (int)lengthOfNextName, nameStart);
		if (result != 0 || parserState.id == -1 || !parserState.isDir) {
			//printf("invalid path, one of the tokens was not a directory\n");
			return -1;
		}
		currDir = parserState.id;
	}

	return currDir;
}

//takes in an empty vfsPathParserState_t
int vfs_parsePath(redisContext *context, vfsPathParserState_t *parserState,
		const char *fullPath, int fullPathLength, long clientCwd) {

	//FIXME: handle 0 strings
	if (fullPathLength <= 0) {
		//stuff
		return -1;
	}

	//printf("the path: --%s--\n", fullPath);

	long cwd = (*fullPath == '/') ? ROOT_FOLDER_ID : clientCwd; //if it's not a relative path set CWD to ROOT_FOLDER_ID
	long tempId;
	int isLastCharSlash, fixedPathLength, i;
	char *tempPathPtr, *formattedPath;

	//wipe the parsing state
	init_vfsPathParserState(parserState);

	//remove the last '/' if it has one
	isLastCharSlash = (*(fullPath + fullPathLength - 1) == '/');
	fixedPathLength = (isLastCharSlash) ? fullPathLength - 1 : fullPathLength;//cut the last '/' to make processing easier

	//special case for root, '/' or '/././' or '///'
	//if the string is empty and our CWD is root then
	//FIXME: this won't work for '/././'
	if (cwd == ROOT_FOLDER_ID && fixedPathLength == 0) {
		parserState->id = ROOT_FOLDER_ID;
		parserState->parentId = ROOT_PARENT_ID;
		parserState->isDir = 1;
		parserState->isFilePath = 0;
		return 0;
	}

	//from this point on the string is '/etc' '/a/b.txt' 'a/b'

	//get all characters after the last '/',
	//After preprocessing and checking if root there will always be characters in the name
	//the name could be ".."
	if (vfs_seperatePathAndName(parserState, fullPath, fixedPathLength) != 0) {
		printf("failed to get the last part of path, stuff after last '/'\n");
		return -1;
	}

	if ((tempId = vfs_getDirIdFromPath(context, cwd, fullPath, fixedPathLength))
			== -1) {
		printf("failed to get the parent id for path: %s\n", fullPath);
		return -1;
	}
	if (parserState->nameLength > 0) {
		// printf("searching the name now: %ld\n", tempId);
		if (vfs_findObjectInDir(context, parserState, tempId,
				fullPath + parserState->nameOffset, parserState->nameLength)
				!= 0) {
			//failed to find in directory so we will now set it's isExistingObject to false
			// printf("searching the name now3\n");
			parserState->isExistingObject = 0;
			parserState->parentId = tempId;
		} else {
			parserState->isExistingObject = 1;
			// printf("searching the name now1\n");
			if (parserState->isFilePath == 1){

				// printf("searching the name now2\n");
				parserState->parentId = tempId;
			}else {
				//parserState->parentId = tempId;//the parent might not be tempId, like if name == '..'
				//printf("lets get the dir parent\n");
				parserState->parentId = vfs_getDirParent(context,
						parserState->id);
				//printf("got the dir parent\n");
			}
		}
	} else {
		printf("The name length was zero???\n");
	}
	return 0;
}

//returns 0 if success, non-0 otherwise
int vfs_deleteObjectWithPath(redisContext *context, char *path, long cwd){
	vfsPathParserState_t parserState;
	int result1 = vfs_parsePath(context, &parserState, path, strlen(path), cwd);
	if(!parserState.isExistingObject){
		return -1;
	}

	printf("removing path %ld %s\n", cwd, path);

	if(parserState.isDir){
		//FIXME:
		printf("removing dir %ld %ld\n", parserState.parentId, parserState.id);
		__removeIdFromFolderList(context, parserState.parentId, parserState.id);
	}else if(parserState.isFilePath){
		__removeIdFromFileList(context, parserState.parentId, parserState.id);
	}
	return 0;
}

