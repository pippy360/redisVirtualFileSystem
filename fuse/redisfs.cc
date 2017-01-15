#include "redisfs.hpp"

#include "../virtualFileSystem/virtualFileSystemWrapper.h"

RedisFs* RedisFs::_instance = NULL;

#define RETURN_ERRNO(x) (x) == 0 ? 0 : -errno

RedisFs* RedisFs::Instance() {
	if(_instance == NULL) {
		_instance = new RedisFs();
	}
	return _instance;
}

RedisFs::RedisFs() {
    _context = vfs::getInitialContext();
}

RedisFs::~RedisFs() {

}

void RedisFs::AbsPath(char dest[PATH_MAX], const char *path) {
	strcpy(dest, _root);
	strncat(dest, path, PATH_MAX);
	//printf("translated path: %s to %s\n", path, dest);
}

void RedisFs::setRootDir(const char *path) {
	printf("setting FS root to: %s\n", path);
	_root = path;
}

int RedisFs::Getattr(const char *path, struct stat *statbuf) {
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	printf("getattr(%s)\n", fullPath);
	return RETURN_ERRNO(lstat(fullPath, statbuf));
}

int RedisFs::Readlink(const char *path, char *link, size_t size) {
	printf("readlink(path=%s, link=%s, size=%d)\n", path, link, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(readlink(fullPath, link, size));
}

int RedisFs::Mknod(const char *path, mode_t mode, dev_t dev) {
	printf("mknod(path=%s, mode=%d)\n", path, mode);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	
	//handles creating FIFOs, regular files, etc...
	return RETURN_ERRNO(mknod(fullPath, mode, dev));
}

int RedisFs::Mkdir(const char *path, mode_t mode) {
	printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(mkdir(fullPath, mode));
}

int RedisFs::Unlink(const char *path) {
	printf("unlink(path=%s\n)", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(unlink(fullPath));
}

int RedisFs::Rmdir(const char *path) {
	printf("rmkdir(path=%s\n)", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(rmdir(fullPath));
}

int RedisFs::Symlink(const char *path, const char *link) {
	printf("symlink(path=%s, link=%s)\n", path, link);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(symlink(fullPath, link));
}

int RedisFs::Rename(const char *path, const char *newpath) {
	printf("rename(path=%s, newPath=%s)\n", path, newpath);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(rename(fullPath, newpath));
}

int RedisFs::Link(const char *path, const char *newpath) {
	printf("link(path=%s, newPath=%s)\n", path, newpath);
	return 0;
}

int RedisFs::Chmod(const char *path, mode_t mode) {
	printf("chmod(path=%s, mode=%d)\n", path, mode);
	return 0;
}

int RedisFs::Chown(const char *path, uid_t uid, gid_t gid) {
	printf("chown(path=%s, uid=%d, gid=%d)\n", path, (int)uid, (int)gid);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(chown(fullPath, uid, gid));
}

int RedisFs::Truncate(const char *path, off_t newSize) {
	printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(truncate(fullPath, newSize));
}

int RedisFs::Utime(const char *path, struct utimbuf *ubuf) {
	printf("utime(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(utime(fullPath, ubuf));
}

int RedisFs::Open(const char *path, struct fuse_file_info *fileInfo) {
	printf("open(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	fileInfo->fh = open(fullPath, fileInfo->flags);
	return 0;
}

int RedisFs::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("read(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return RETURN_ERRNO(pread(fileInfo->fh, buf, size, offset));
}

int RedisFs::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("write(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return RETURN_ERRNO(pwrite(fileInfo->fh, buf, size, offset));
}

int RedisFs::Statfs(const char *path, struct statvfs *statInfo) {
	printf("statfs(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(statvfs(fullPath, statInfo));
}

int RedisFs::Flush(const char *path, struct fuse_file_info *fileInfo) {
	printf("flush(path=%s)\n", path);
	//noop because we don't maintain our own buffers
	return 0;
}

int RedisFs::Release(const char *path, struct fuse_file_info *fileInfo) {
	printf("release(path=%s)\n", path);
	return 0;
}

int RedisFs::Fsync(const char *path, int datasync, struct fuse_file_info *fi) {
	printf("fsync(path=%s, datasync=%d\n", path, datasync);
	if(datasync) {
		//sync data only
		return RETURN_ERRNO(fdatasync(fi->fh));
	} else {
		//sync data + file metadata
		return RETURN_ERRNO(fsync(fi->fh));
	}
}

int RedisFs::Setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
	printf("setxattr(path=%s, name=%s, value=%s, size=%d, flags=%d\n",
		path, name, value, (int)size, flags);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(lsetxattr(fullPath, name, value, size, flags));
}

int RedisFs::Getxattr(const char *path, const char *name, char *value, size_t size) {
	printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(getxattr(fullPath, name, value, size));
}

int RedisFs::Listxattr(const char *path, char *list, size_t size) {
	printf("listxattr(path=%s, size=%d)\n", path, (int)size);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(llistxattr(fullPath, list, size));
}

int RedisFs::Removexattr(const char *path, const char *name) {
	printf("removexattry(path=%s, name=%s)\n", path, name);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(lremovexattr(fullPath, name));
}

int RedisFs::Opendir(const char *path, struct fuse_file_info *fileInfo) {
	printf("opendir(path=%s)\n", path);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	DIR *dir = opendir(fullPath);
	fileInfo->fh = (uint64_t)dir;
	return NULL == dir ? -errno : 0;
}

int RedisFs::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
	printf("readdir(path=%s, offset=%d)\n", path, (int)offset);
	
	//FIXME: check if isValidExistingFolder()

	// auto files = listFiles(_context, path);

	// auto folders = listFolders(_context, path);

	// for (auto folder : folders){
	// 	if(filler(buf, folder.name.c_str(), NULL, 0) != 0) {
	// 		return -ENOMEM;
	// 	}
	// }

	// for (auto file : files){
	// 	if(filler(buf, file.name.c_str(), NULL, 0) != 0) {
	// 		return -ENOMEM;
	// 	}
	// }

	return 0;
}

int RedisFs::Releasedir(const char *path, struct fuse_file_info *fileInfo) {
	printf("releasedir(path=%s)\n", path);
	closedir((DIR*)fileInfo->fh);
	return 0;
}

int RedisFs::Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
	return 0;
}

int RedisFs::Init(struct fuse_conn_info *conn) {
	return 0;
}

int RedisFs::Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
	printf("truncate(path=%s, offset=%d)\n", path, (int)offset);
	char fullPath[PATH_MAX];
	AbsPath(fullPath, path);
	return RETURN_ERRNO(ftruncate(fileInfo->fh, offset));
}

