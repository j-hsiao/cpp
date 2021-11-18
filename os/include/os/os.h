#ifndef OS_H
#define OS_H

#include <os/os_dllcfg.h>

#include <stdio.h>
#include <stddef.h>

//TODO update

CPP_EXTERNC_BEGIN

//buf to store first size bytes of exception messages
typedef struct { char *buf; size_t size; } os_ebuf;

//paths
typedef struct os_Path os_Path;
OS_API os_Path* os_Path_make(const char *path, os_ebuf*);
OS_API void os_Path_free(os_Path*);
//modifiers
OS_API int os_Path_abs(os_Path *, os_ebuf*);
OS_API int os_Path_normalize(os_Path *, os_ebuf*);
OS_API int os_Path_add(os_Path *, const char *extra, os_ebuf*);
OS_API int os_Path_add_ext(os_Path *, const char *ext, os_ebuf*);
OS_API void os_Path_pop_ext(os_Path *);
OS_API void os_Path_pop_base(os_Path *);
//inspect
OS_API bool os_Path_isroot(const os_Path*);
OS_API const char* os_Path_c_str(const os_Path *);
OS_API const char* os_Path_base(const os_Path *);
OS_API const char* os_Path_ext(const os_Path *);
OS_API size_t os_Path_size(const os_Path*);
OS_API const char* os_sysdir(const char *id, os_ebuf*);


typedef struct os_Dirlist os_Dirlist;
OS_API os_Dirlist* os_Dirlist_make(const char *path, os_ebuf*);
OS_API void os_Dirlist_free(os_Dirlist*);
OS_API const char* os_Dirlist_next(os_Dirlist*, os_ebuf*);

//< 0 = failed to make
typedef unsigned long long os_Filenode;
OS_API os_Filenode os_Filenode_make(const char *path, os_ebuf*);
OS_API bool os_Filenode_exists(os_Filenode node);
OS_API bool os_Filenode_is_dir(os_Filenode node);
OS_API bool os_Filenode_is_file(os_Filenode node);

OS_API int os_mkdir(const char *path, os_ebuf *);
//recursive makedirs
OS_API int os_makedirs(const char *path, os_ebuf*);
//works for directories and files
OS_API int os_rename(const char *before, const char *after, os_ebuf*);
OS_API int os_remove(const char *path, os_ebuf*);

//terminal
OS_API void os_hide_window();
OS_API void os_show_window();

typedef struct os_Term os_Term;
OS_API os_Term* os_Term_open(const char *mode);
OS_API void os_Term_free(os_Term *term);
OS_API char* os_Term_readline(os_Term *term, size_t *sz, bool strip);
OS_API void os_Term_linefree(char *p);

//return flags
//&0x01 = bool, what was original value? (showing or not)
//&0x02 = bool, errored
OS_API int os_hide_input(os_Term*, os_ebuf*);
OS_API int os_show_input(os_Term*, os_ebuf*);

//environment
// nullptr for value = unset named environment variable.
OS_API int os_setenv(const char *name, const char *value, os_ebuf *ebuf);
//>=0 -> length of environment variable (if > bufsize, then buf was untouched)
//-1: not found
//-2: other
OS_API int os_getenv(const char *name, char *buf, size_t bufsize, os_ebuf *ebuf);

CPP_EXTERNC_END

#endif
