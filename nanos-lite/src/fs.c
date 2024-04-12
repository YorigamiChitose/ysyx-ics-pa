#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

typedef struct {
  size_t open_offset;
} SEEKinfo;

#define ARRLEN(arr) (int)(sizeof(arr) / sizeof(arr[0]))

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_END};

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN   ] = {"stdin",           0, 0, invalid_read,  invalid_write },
  [FD_STDOUT  ] = {"stdout",          0, 0, invalid_read,  invalid_write },
  [FD_STDERR  ] = {"stderr",          0, 0, invalid_read,  invalid_write },
  [FD_FB      ] = { "/dev/fb",        0, 0, invalid_read,  fb_write      },
  [FD_EVENTS  ] = { "/dev/events",    0, 0, events_read,   invalid_write },
  [FD_DISPINFO] = { "/proc/dispinfo", 0, 0, dispinfo_read, invalid_write },
#include "files.h"
};

static SEEKinfo file_offset[ARRLEN(file_table)] = {0};

void init_fs() {
  file_table[FD_FB].size = io_read(AM_GPU_CONFIG).vmemsz;
}

int fs_open(const char *pathname, int flags, int mode) {
  Debug("%s", pathname);
  for (int i = 0; i < ARRLEN(file_table); i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      Debug("file fd:%s(%d, at %d) found. Openning...", pathname, i, (int)(file_table[i].disk_offset));
      file_offset[i].open_offset = 0;
      return i;
    }
  }
  panic("file:%s not found", pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  Assert(buf != NULL, "File operation error: read the NULL");
  Assert(fd < ARRLEN(file_table), "file fd:%d not found", fd);
  Debug("file fd:%d found. Reading (%d)...", fd, len);
  size_t read_len = 0;
  if (file_offset[fd].open_offset >= file_table[fd].size && fd >= FD_END) {return 0;}
  if(file_table[fd].read != NULL) {
    read_len = file_table[fd].read(buf, file_table[fd].disk_offset + file_offset[fd].open_offset, len);
  } else {
    read_len = ramdisk_read(buf, file_table[fd].disk_offset + file_offset[fd].open_offset, len);
  }
  file_offset[fd].open_offset += read_len;
  return read_len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  Assert(fd < ARRLEN(file_table), "file fd:%d not found", fd);
  Debug("file fd:%d found. Writing (%d)...", fd, len);
  size_t write_len = 0;
  if (file_offset[fd].open_offset >= file_table[fd].size && fd >= FD_END) {assert(0); return 0;}
  if(file_table[fd].write != NULL) {
    write_len = file_table[fd].write(buf, file_table[fd].disk_offset + file_offset[fd].open_offset, len);
  } else {
    write_len = ramdisk_write(buf, file_table[fd].disk_offset + file_offset[fd].open_offset, len);
  }
  file_offset[fd].open_offset += write_len;
  return write_len;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  Assert(fd < ARRLEN(file_table), "file fd:%d not found", fd);
  Debug("file fd:%d found. Seting...", fd);
  switch (whence) {
    case SEEK_SET: file_offset[fd].open_offset = offset; break;
    case SEEK_CUR: file_offset[fd].open_offset += offset; break;
    case SEEK_END: file_offset[fd].open_offset = file_table[fd].size + offset; break;
    default: break;
  }
  Debug("Offset:%d", (int)(file_offset[fd].open_offset));
  return file_offset[fd].open_offset;
}

int fs_close(int fd) {
  Assert(fd < ARRLEN(file_table), "file fd:%d not found", fd);
  Debug("file fd:%d found. Closing...", fd);
  file_offset[fd].open_offset = 0;
  return 0;
}