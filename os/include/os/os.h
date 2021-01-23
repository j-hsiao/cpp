#ifndef OS_H
#define OS_H

#include <os/os_dllcfg.h>

#include <stdio.h>
#include <stddef.h>

CPP_EXTERNC_BEGIN

//buf to store first size bytes of exception messages
typedef struct { char *buf; size_t size; } os__ebuf;

//paths
typedef struct os__Path os__Path;
OS_API os__Path* os__Path__make(const char *path, os__ebuf*);
OS_API os__Path* os__Path__copy(const os__Path*, os__ebuf*);
OS_API void os__Path__free(os__Path*);
//modifiers
OS_API int os__Path__add(os__Path *, const char *extra, os__ebuf*);
OS_API void os__Path__pop_ext(os__Path *);
OS_API int os__Path__add_ext(os__Path *, const char *ext, os__ebuf*);
OS_API void os__Path__dirname(os__Path *);
//inspect
OS_API const char* os__Path__c_str(const os__Path *);
OS_API const char* os__Path__base(const os__Path *);
OS_API const char* os__Path__ext(const os__Path *);
OS_API size_t os__Path__segments(const os__Path*);
OS_API size_t os__Path__size(const os__Path*);


typedef struct os__Dirlist os__Dirlist;
OS_API os__Dirlist* os__Dirlist__make(const char *path, os__ebuf*);
OS_API int os__Dirlist__free(os__Dirlist*, os__ebuf*);
OS_API const char* os__Dirlist__next(os__Dirlist*, os__ebuf*);

//< 0 = failed to make
typedef long long os__Filenode;
OS_API os__Filenode os__Filenode__make(const char *path, os__ebuf*);
OS_API bool os__Filenode__exists(os__Filenode node);
OS_API bool os__Filenode__is_dir_nc(os__Filenode node);
OS_API bool os__Filenode__is_file_nc(os__Filenode node);
//check exist too
static inline bool os__Filenode__is_dir(os__Filenode node)
{ return os__Filenode__exists(node) && os__Filenode__is_dir_nc(node); }
static inline bool os__Filenode__is_file(os__Filenode node)
{ return os__Filenode__exists(node) && os__Filenode__is_file_nc(node); }

OS_API int os__mkdir(const char *path, os__ebuf *);
//recursive makedirs
OS_API int os__makedirs(const char *path, os__ebuf*);
//works for directories and files
OS_API int os__rename(const char *before, const char *after, os__ebuf*);
OS_API int os__remove(const char *path, os__ebuf*);

OS_API const char* os__get_sysdir(const char *name);

//terminal
OS_API void os__hide_window();
OS_API void os__show_window();

OS_API FILE* os__term_open(const char *mode);
inline int os__term_close(FILE *term)
{ return fclose(term); }
OS_API int os__hide_input(FILE*, os__ebuf*);
OS_API int os__show_input(FILE*, os__ebuf*);

//environment
OS_API int os__set_env(const char *val, const char *var, os__ebuf*);
//> bufsize: required length
//0 <= ret < = bufsize : string size
//-1: not found
//-2: other
OS_API int os__get_env(char *buf, const char *var, size_t bufsize, os__ebuf *ebuf);

CPP_EXTERNC_END

#endif
