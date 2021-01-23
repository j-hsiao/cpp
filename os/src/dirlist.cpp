//wstring_convert<codecvt_utf8<wchar_t>>()
#include <system_error>
#include <dirent.h>
// generic_category
// system_category
// std::error_condition
// std::make_error_condition(std::errc)
// std::
// std::error_code(int ec, error_category &ecat);
//
//

#ifdef _WIN32

struct os__Dirlist;
OS_API const char* os__Dirlist__next(os__Dirlist*);
OS_API os__Dirlist* os__Dirlist__make(const char *path);
OS_API void os__Dirlist__free(os__Dirlist*);





#else

#include <sys/types.h>
#include <dirent.h>

struct os__Dirlist
{
	DIR *dirp
	struct dirent *
};

int closedir(DIR *dirp)
DIR* opendir(const char*)
struct dirent* readdir(const char*)


OS_API const char* os__Dirlist__next(os__Dirlist*);
OS_API os__Dirlist* os__Dirlist__make(const char *path);
OS_API void os__Dirlist__free(os__Dirlist*);







#endif
