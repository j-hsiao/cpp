#include <os/os.h>
#include <.os/util.hpp>

#include "path_incl.cpp"

#include <iostream>
#include <codecvt>
#include <cstring>
#include <locale>
#include <map>
#include <string>
#include <system_error>

#include <stdio.h>
#include <io.h>

//https://docs.microsoft.com/en-us/windows/win32/winprog/using-the-windows-headers?redirectedfrom=MSDN
//The following table describes the preferred macros used in the Windows header files. If you define NTDDI_VERSION, you must also define _WIN32_WINNT.
#include <windows.h>
#include <Shlobj.h>
#include <Knownfolders.h>

#include <accctrl.h>
#include <fcntl.h>

//new process with piped in/out
//https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output

namespace
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> strcvt;

	//mingw system_error is not compatible with GetLastError() codes
	std::string syserror(DWORD code)
	{
		const DWORD Bufsize = 256;
		wchar_t buf[Bufsize];
		DWORD result = ::FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			buf,
			Bufsize,
			nullptr);
		if (result)
		{ return strcvt.to_bytes(buf, buf + result); }
		else
		{ return "error code " + std::to_string(code); }
	}

	os__ebuf& operator<<(os__ebuf &buf, int code)
	{
		std::strncpy(
			buf.buf, std::generic_category().message(code).c_str(), buf.size);
		buf.buf[buf.size - 1] = 0;
		return buf;
	}
	os__ebuf& operator<<(os__ebuf &buf, DWORD code)
	{
		std::strncpy(buf.buf, syserror(code).c_str(), buf.size);
		buf.buf[buf.size - 1] = 0;
		return buf;
	}
	//bit 29 = user bit
	//https://docs.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
	static const DWORD User_Flag = 1 << 29;
}

struct os__Dirlist
{
	DWORD code;
	WIN32_FIND_DATAW info;
	HANDLE handle;
	std::string buf;
};
#if OS_DEBUG
namespace
{
	size_t os__dirlist__made = 0;
	size_t os__dirlist__freed = 0;
}
#endif

os__Dirlist* os__Dirlist__make(const char *path, os__ebuf *ebuf)
{
#if OS_DEBUG
	++os__dirlist__made;
	Log() << "dirlists created" << std::endl;
#endif
	auto ret = new os__Dirlist{};
	if (ret)
	{
		const wchar_t wild[] = {'/', '*', 0};
		ret->handle = FindFirstFileW(
			(strcvt.from_bytes(path) + wild).c_str(),
			&ret->info);
		
		if (ret->handle == INVALID_HANDLE_VALUE)
		{ ret->code = GetLastError(); }
	}
	else
	{
		if (ebuf) { std::strncpy(ebuf->buf, "failed allocation", ebuf->size); }
	}
	return ret;
}

int os__Dirlist__free(os__Dirlist *dl, os__ebuf *ebuf)
{
#if OS_DEBUG
	++os__dirlist__freed;
	Log() << "dirlist: "
		<< os__dirlist__made << " / " << os__dirlist__freed << std::endl;
#endif
	if (dl->handle != INVALID_HANDLE_VALUE)
	{
		if (!FindClose(dl->handle))
		{
			if (ebuf) { *ebuf << GetLastError(); }
			return -1;
		}
	}
	delete dl;
	return 0;
}
namespace
{
	const char* Dirlist__next(os__Dirlist *dl, os__ebuf *ebuf)
	{
		if (dl->code == ERROR_SUCCESS)
		{
			dl->buf = strcvt.to_bytes(dl->info.cFileName);
			if (!FindNextFileW(dl->handle, &dl->info))
			{ dl->code = GetLastError(); }
			return dl->buf.c_str();
		}
		else
		{
			if (dl->code == ERROR_NO_MORE_FILES || dl->code == ERROR_FILE_NOT_FOUND) { return ""; }
			if (ebuf) { *ebuf << dl->code; }
			return nullptr;
		}
	}
}

const char* os__Dirlist__next(os__Dirlist *dl, os__ebuf *ebuf)
{
	const char *ret;
	while (
		(ret = Dirlist__next(dl, ebuf))
		&& (
			!strcmp(ret, ".")
			|| !strcmp(ret, "..")))
	{}
	return ret;
}

//------------------------------
// file node
//------------------------------
//GetFileAttributesW
//https://docs.microsoft.com/en-us/windows/win32/fileio/file-attribute-constants
//https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-getfileattributesw

os__Filenode os__Filenode__make(const char *path, os__ebuf *buf)
{
	DWORD attrs = GetFileAttributesW(strcvt.from_bytes(path).c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES)
	{
		DWORD code = GetLastError();
		if (code != ERROR_FILE_NOT_FOUND && code != ERROR_PATH_NOT_FOUND)
		{
			if (buf) { *buf << code; }
			return -1;
		}
		else { return 0; }
	}
	else
	{ return attrs; }
}
bool os__Filenode__exists(os__Filenode node) { return node > 0; }
bool os__Filenode__is_dir_nc(os__Filenode node)
{ return node & FILE_ATTRIBUTE_DIRECTORY; }
bool os__Filenode__is_file_nc(os__Filenode node)
{ return !os__Filenode__is_dir_nc(node); }

//------------------------------
//fs modification
//------------------------------
int os__mkdir(const char *path, os__ebuf *buf)
{
	if (!CreateDirectoryW(strcvt.from_bytes(path).c_str(), NULL))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	return 0;
}

#include "normdst_incl.cpp"
//works for directories and files
int os__rename(const char *before, const char *after, os__ebuf *buf)
{
	try
	{
		std::string dst = normalize_dst(before, after, buf);
		if (!dst.size()) { return -1; }
		if (!MoveFileExW(
			strcvt.from_bytes(before).c_str(),
			strcvt.from_bytes(dst).c_str(),
			MOVEFILE_COPY_ALLOWED))
		{
			if (buf) { *buf << GetLastError(); }
			return -1;
		}
		return 0;
	}
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return -1;
	}
}
int os__remove(const char *path, os__ebuf *buf)
{
	try
	{
		os__Filenode node = os__Filenode__make(path, buf);
		if (node < 0) { return -1; }
		if (os__Filenode__exists(node))
		{
			bool succ = 0;
			if (os__Filenode__is_file_nc(node))
			{ succ = DeleteFileW(strcvt.from_bytes(path).c_str()); }
			else
			{ succ = RemoveDirectoryW(strcvt.from_bytes(path).c_str()); }
			if (!succ)
			{
				DWORD code = GetLastError();
				if (code != ERROR_FILE_NOT_FOUND)
				{
					if (buf) { *buf << code; }
					return -1;
				}
			}
		}
		return 0;
	}
	catch (std::exception &exc)
	{
		if (buf) { *buf << exc; }
		return -1;
	}
}

const char* os__get_sysdir(const char *name)
{
	static std::map<std::string, std::string> rets;
	auto it = rets.find(name);
	if (it != rets.end()) { return it->second.c_str(); }
#if _WIN32_WINNT >= _WIN32_WINNT_VISTA
	static std::map<std::string, REFKNOWNFOLDERID> ids
	{
		{"accountpictures", FOLDERID_AccountPictures},
		{"admintools", FOLDERID_AdminTools},
		{"appdatadesktop", FOLDERID_AppDataDesktop},
		{"appdatadocuments", FOLDERID_AppDataDocuments},
		{"appdatafavorites", FOLDERID_AppDataFavorites},
		{"appdataprogramdata", FOLDERID_AppDataProgramData},
		{"applicationshortcuts", FOLDERID_ApplicationShortcuts},
		{"cameraroll", FOLDERID_CameraRoll},
		{"cdburning", FOLDERID_CDBurning},
		{"commonadmintools", FOLDERID_CommonAdminTools},
		{"commonoemlinks", FOLDERID_CommonOEMLinks},
		{"commonprograms", FOLDERID_CommonPrograms},
		{"commonstartmenu", FOLDERID_CommonStartMenu},
		{"commonstartup", FOLDERID_CommonStartup},
		{"commontemplates", FOLDERID_CommonTemplates},
		{"contacts", FOLDERID_Contacts},
		{"cookies", FOLDERID_Cookies},
		{"desktop", FOLDERID_Desktop},
		{"devicemetadatastore", FOLDERID_DeviceMetadataStore},
		{"documents", FOLDERID_Documents},
		{"documentslibrary", FOLDERID_DocumentsLibrary},
		{"downloads", FOLDERID_Downloads},
		{"favorites", FOLDERID_Favorites},
		{"fonts", FOLDERID_Fonts},
		{"gametasks", FOLDERID_GameTasks},
		{"history", FOLDERID_History},
		{"implicitappshortcuts", FOLDERID_ImplicitAppShortcuts},
		{"internetcache", FOLDERID_InternetCache},
		{"libraries", FOLDERID_Libraries},
		{"links", FOLDERID_Links},
		{"localappdata", FOLDERID_LocalAppData},
		{"localappdatalow", FOLDERID_LocalAppDataLow},
		{"localizedresourcesdir", FOLDERID_LocalizedResourcesDir},
		{"music", FOLDERID_Music},
		{"musiclibrary", FOLDERID_MusicLibrary},
		{"nethood", FOLDERID_NetHood},
		{"objects3d", FOLDERID_Objects3D},
		{"originalimages", FOLDERID_OriginalImages},
		{"photoalbums", FOLDERID_PhotoAlbums},
		{"pictureslibrary", FOLDERID_PicturesLibrary},
		{"pictures", FOLDERID_Pictures},
		{"playlists", FOLDERID_Playlists},
		{"printhood", FOLDERID_PrintHood},
		{"profile", FOLDERID_Profile},
		{"programdata", FOLDERID_ProgramData},
		{"programfiles", FOLDERID_ProgramFiles},
		{"programfilesx64", FOLDERID_ProgramFilesX64},
		{"programfilesx86", FOLDERID_ProgramFilesX86},
		{"programfilescommon", FOLDERID_ProgramFilesCommon},
		{"programfilescommonx64", FOLDERID_ProgramFilesCommonX64},
		{"programfilescommonx86", FOLDERID_ProgramFilesCommonX86},
		{"programs", FOLDERID_Programs},
		{"public", FOLDERID_Public},
		{"publicdesktop", FOLDERID_PublicDesktop},
		{"publicdocuments", FOLDERID_PublicDocuments},
		{"publicdownloads", FOLDERID_PublicDownloads},
		{"publicgametasks", FOLDERID_PublicGameTasks},
		{"publiclibraries", FOLDERID_PublicLibraries},
		{"publicmusic", FOLDERID_PublicMusic},
		{"publicpictures", FOLDERID_PublicPictures},
		{"publicringtones", FOLDERID_PublicRingtones},
		{"publicusertiles", FOLDERID_PublicUserTiles},
		{"publicvideos", FOLDERID_PublicVideos},
		{"quicklaunch", FOLDERID_QuickLaunch},
		{"recent", FOLDERID_Recent},
		{"recordedtvlibrary", FOLDERID_RecordedTVLibrary},
		{"recyclebinfolder", FOLDERID_RecycleBinFolder},
		{"resourcedir", FOLDERID_ResourceDir},
		{"ringtones", FOLDERID_Ringtones},
		{"roamingappdata", FOLDERID_RoamingAppData},
		{"roamedtileimages", FOLDERID_RoamedTileImages},
		{"roamingtiles", FOLDERID_RoamingTiles},
		{"samplemusic", FOLDERID_SampleMusic},
		{"samplepictures", FOLDERID_SamplePictures},
		{"sampleplaylists", FOLDERID_SamplePlaylists},
		{"samplevideos", FOLDERID_SampleVideos},
		{"savedgames", FOLDERID_SavedGames},
		{"savedpictures", FOLDERID_SavedPictures},
		{"savedpictureslibrary", FOLDERID_SavedPicturesLibrary},
		{"savedsearches", FOLDERID_SavedSearches},
		{"screenshots", FOLDERID_Screenshots},
		{"searchhistory", FOLDERID_SearchHistory},
		{"searchtemplates", FOLDERID_SearchTemplates},
		{"sendto", FOLDERID_SendTo},
		{"sidebardefaultparts", FOLDERID_SidebarDefaultParts},
		{"sidebarparts", FOLDERID_SidebarParts},
		{"skydrive", FOLDERID_SkyDrive},
		{"skydrivecameraroll", FOLDERID_SkyDriveCameraRoll},
		{"skydrivedocuments", FOLDERID_SkyDriveDocuments},
		{"skydrivepictures", FOLDERID_SkyDrivePictures},
		{"startmenu", FOLDERID_StartMenu},
		{"startup", FOLDERID_Startup},
		{"system", FOLDERID_System},
		{"systemx86", FOLDERID_SystemX86},
		{"templates", FOLDERID_Templates},
		{"userpinned", FOLDERID_UserPinned},
		{"userprofiles", FOLDERID_UserProfiles},
		{"userprogramfiles", FOLDERID_UserProgramFiles},
		{"userprogramfilescommon", FOLDERID_UserProgramFilesCommon},
		{"videos", FOLDERID_Videos},
		{"videoslibrary", FOLDERID_VideosLibrary},
		{"windows", FOLDERID_Windows}
	};
	auto refit = ids.find(name);
	if (refit == ids.end()) { return ""; }
	PWSTR wstr;
	HRESULT res = SHGetKnownFolderPath(refit->second, 0, NULL, &wstr);
	const char *ret = "";
	if (res == S_OK)
	{
		rets[name] = strcvt.to_bytes(wstr);
		ret = rets.at(name).c_str();
	}
	CoTaskMemFree(wstr);
	return ret;
#else
	std::map<std::string, int> ids = 
	{
		{"admintools", CSIDL_ADMINTOOLS},
		{"cdburning", CSIDL_CDBURN_AREA},
		{"commonadmintools", CSIDL_COMMON_ADMINTOOLS},
		{"commonoemlinks", CSIDL_COMMON_OEM_LINKS},
		{"commonprograms", CSIDL_COMMON_PROGRAMS},
		{"commonstartmenu", CSIDL_COMMON_STARTMENU},
		{"commonstartup", CSIDL_COMMON_STARTUP},
		{"commontemplates", CSIDL_COMMON_TEMPLATES},
		{"cookies", CSIDL_COOKIES},
		{"desktop", CSIDL_DESKTOP},
		{"documents", CSIDL_MYDOCUMENTS},
		{"favorites", CSIDL_FAVORITES},
		{"fonts", CSIDL_FONTS},
		{"history", CSIDL_HISTORY},
		{"internetcache", CSIDL_INTERNET_CACHE},
		{"localappdata", CSIDL_LOCAL_APPDATA},
		{"localizedresourcesdir", CSIDL_RESOURCES_LOCALIZED},
		{"music", CSIDL_MYMUSIC},
		{"nethood", CSIDL_NETHOOD},
		{"pictures", CSIDL_MYPICTURES},
		{"printhood", CSIDL_PRINTHOOD},
		{"profile", CSIDL_PROFILE},
		{"home", CSIDL_PROFILE},
		{"programdata", CSIDL_COMMON_APPDATA},
		{"programfiles", CSIDL_PROGRAM_FILES},
		{"programfilesx86", CSIDL_PROGRAM_FILESX86},
		{"programfilescommon", CSIDL_PROGRAM_FILES_COMMON},
		{"programfilescommonx86", CSIDL_PROGRAM_FILES_COMMONX86},
		{"programs", CSIDL_PROGRAMS},
		{"publicdesktop", CSIDL_COMMON_DESKTOPDIRECTORY},
		{"publicdocuments", CSIDL_COMMON_DOCUMENTS},
		{"publicmusic", CSIDL_COMMON_MUSIC},
		{"publicpictures", CSIDL_COMMON_PICTURES},
		{"publicvideos", CSIDL_COMMON_VIDEO},
		{"recent", CSIDL_RECENT},
		{"recyclebinfolder", CSIDL_BITBUCKET},
		{"resourcedir", CSIDL_RESOURCES},
		{"roamingappdata", CSIDL_APPDATA},
		{"sendto", CSIDL_SENDTO},
		{"startmenu", CSIDL_STARTMENU},
		{"startup", CSIDL_STARTUP},
		{"system", CSIDL_SYSTEM},
		{"systemx86", CSIDL_SYSTEMX86},
		{"templates", CSIDL_TEMPLATES},
		{"videos", CSIDL_MYVIDEO},
		{"windows", CSIDL_WINDOWS}
	};

	auto csidlit = ids.find(name);
	if (csidlit == ids.end()) { return ""; }

	wchar_t res[MAX_PATH];
	if (
		SHGetFolderPathW(
			NULL, csidlit->second, NULL, SHGFP_TYPE_CURRENT, res) == S_OK)
	{
		rets[name] = strcvt.to_bytes(res);
		return rets.at(name).c_str();
	}
	return "";
#endif
}

OS_API void os__hide_window()
{
	HWND handle = GetConsoleWindow();
	if (handle != NULL)
	{ ShowWindow(handle, SW_HIDE); }
}
OS_API void os__show_window()
{
	HWND handle = GetConsoleWindow();
	if (handle != NULL)
	{ ShowWindow(handle, SW_SHOW); }
}

FILE* os__term_open(const char *mode)
{
	std::string smode(mode);
	bool read = smode.find_first_of('r') != std::string::npos;
	bool write = smode.find_first_of("wa") != std::string::npos;
	std::wstring name;
	//?? can't really find documentation on CONIN/CONOUT/CON
	name = strcvt.from_bytes("CONIN$");
	HANDLE h = CreateFileW(
		name.c_str(),
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (h != INVALID_HANDLE_VALUE)
	{
		int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h), 0);
		if (fd >= 0)
		{
			FILE *f = _fdopen(fd, mode);
			if (f)
			{ return f; }
			else
			{ _close(fd); }
		}
		else
		{ CloseHandle(h); }
	}
	FILE *f = fopen("CONIN$", mode);
	return f;
}

int os__hide_input(FILE *f, os__ebuf *buf)
{
	int fd = ::_fileno(f);
	if (fd == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	else if (fd == -2)
	{
		if (buf) { *buf << static_cast<int>(std::errc::inappropriate_io_control_operation); }
		return -1;
	}
	HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
	if (h == INVALID_HANDLE_VALUE)
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	DWORD mode;
	if (!GetConsoleMode(h, &mode))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	mode &= ~ENABLE_ECHO_INPUT;
	if (!SetConsoleMode(h, mode))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	return 0;
}
int os__show_input(FILE *f, os__ebuf *buf)
{
	int fd = ::_fileno(f);
	if (fd == -1)
	{
		if (buf) { *buf << errno; }
		return -1;
	}
	else if (fd == -2)
	{
		if (buf) { *buf << static_cast<int>(std::errc::inappropriate_io_control_operation); }
		return -1;
	}
	HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
	if (h == INVALID_HANDLE_VALUE)
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	DWORD mode;
	if (!GetConsoleMode(h, &mode))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT;
	if (!SetConsoleMode(h, mode))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	return 0;
}

int os__set_env(const char *val, const char *var, os__ebuf *buf)
{
	std::wstring wvar = strcvt.from_bytes(var);
	const wchar_t *wval = nullptr;
	std::wstring value;
	if (val)
	{
		value = strcvt.from_bytes(val);
		wval = value.c_str();
	}
	if (SetEnvironmentVariableW(wvar.c_str(), wval))
	{
		if (buf) { *buf << GetLastError(); }
		return -1;
	}
	return 0;
}
int os__get_env(char *buf, const char *var, size_t bufsize, os__ebuf *ebuf)
{
	std::wstring wbuf(bufsize, '\0');
	DWORD ret = GetEnvironmentVariableW(
		strcvt.from_bytes(var).c_str(),
		&wbuf[0],
		bufsize);
	if (ret == 0)
	{
		DWORD code = GetLastError();
		if (code == ERROR_ENVVAR_NOT_FOUND)
		{ return -1; }
		else if (code == ERROR_SUCCESS)
		{ return 0; }
		else
		{
			if (buf) { *buf << code; }
			return -2;
		}
	}
	else if (ret <= bufsize)
	{
		wbuf.resize(ret);
		std::string result = strcvt.to_bytes(wbuf);
		if (result.size() <= bufsize)
		{ std::strncpy(buf, result.c_str(), bufsize); }
		return static_cast<int>(result.size());
	}
	else
	{
		return static_cast<int>(ret);
	}
}
#include "common_incl.cpp"
