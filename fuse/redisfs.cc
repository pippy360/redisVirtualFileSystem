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

int RedisFs::Getattr(const char *path, struct stat *statbuf) {
	printf("getattr(%s)\n", path);
	vfs::dirItemInfo info = vfs::stat(_context, path);
    // long id;
    // bool exists;
    // dirItemType type;
    // long parentId;
    // string name;
	statbuf->st_nlink = info.id;

	if ( info.type == vfs::DIR_ITEM_FOLDER ) {
		statbuf->st_mode = S_IFDIR;
	}else{
		statbuf->st_mode = S_IFREG;
	}
	statbuf->st_size = 0;

	return 0;
}

int RedisFs::Readlink(const char *path, char *link, size_t size) {
	printf("readlink(path=%s, link=%s, size=%d)\n", path, link, (int)size);
	return 0;
}

int RedisFs::Mknod(const char *path, mode_t mode, dev_t dev) {
	printf("mknod(path=%s, mode=%d)\n", path, mode);
	return 0;
}

int RedisFs::Mkdir(const char *path, mode_t mode) {
	printf("**mkdir(path=%s, mode=%d)\n", path, (int)mode);
	return 0;
}

int RedisFs::Unlink(const char *path) {
	printf("unlink(path=%s\n)", path);
	return 0;
}

int RedisFs::Rmdir(const char *path) {
	printf("rmkdir(path=%s\n)", path);
	return 0;
}

int RedisFs::Symlink(const char *path, const char *link) {
	printf("symlink(path=%s, link=%s)\n", path, link);
	return 0;
}

int RedisFs::Rename(const char *path, const char *newpath) {
	printf("rename(path=%s, newPath=%s)\n", path, newpath);
	return 0;
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
	return 0;
}

int RedisFs::Truncate(const char *path, off_t newSize) {
	printf("truncate(path=%s, newSize=%d\n", path, (int)newSize);
	return 0;
}

int RedisFs::Utime(const char *path, struct utimbuf *ubuf) {
	printf("utime(path=%s)\n", path);
	char fullPath[PATH_MAX];
	return RETURN_ERRNO(utime(fullPath, ubuf));
}

int RedisFs::Open(const char *path, struct fuse_file_info *fileInfo) {
	printf("open(path=%s)\n", path);
	return 0;
}

int RedisFs::Read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("read(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return -1;
}

int RedisFs::Write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fileInfo) {
	printf("write(path=%s, size=%d, offset=%d)\n", path, (int)size, (int)offset);
	return 0;
}

int RedisFs::Statfs(const char *path, struct statvfs *statInfo) {
	printf("statfs(path=%s)\n", path);

	return 0;
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
	return 0;
}

int RedisFs::Setxattr(const char *path, const char *name, const char *value, size_t size, int flags) {
	printf("setxattr(path=%s, name=%s, value=%s, size=%d, flags=%d\n",
		path, name, value, (int)size, flags);
	return 0;
}

int RedisFs::Getxattr(const char *path, const char *name, char *value, size_t size) {
	printf("getxattr(path=%s, name=%s, size=%d\n", path, name, (int)size);
	return 0;
}

int RedisFs::Listxattr(const char *path, char *list, size_t size) {
	printf("listxattr(path=%s, size=%d)\n", path, (int)size);
	return 0;
}

int RedisFs::Removexattr(const char *path, const char *name) {
	printf("removexattry(path=%s, name=%s)\n", path, name);

	return 0;
}


int RedisFs::Opendir(const char *path, struct fuse_file_info *fileInfo) {
	printf("opendir(path=%s)\n", path);
	return 0;
}

int RedisFs::Readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fileInfo) {
	printf("readdir(path=%s, offset=%d)\n", path, (int)offset);
	vector<string> names = vfs::ls(_context, path);

	for(auto name: names){
		if(filler(buf, name.c_str(), NULL, 0) != 0) {
			return -ENOMEM;
		}
	}
	return 0;
}

int RedisFs::Releasedir(const char *path, struct fuse_file_info *fileInfo) {
	printf("releasedir(path=%s)\n", path);
	return 0;
}

int RedisFs::Fsyncdir(const char *path, int datasync, struct fuse_file_info *fileInfo) {
	return 0;
}

int RedisFs::Init(struct fuse_conn_info *conn) {
	return 0;
}

int RedisFs::Truncate(const char *path, off_t offset, struct fuse_file_info *fileInfo) {
	return 0;
}


