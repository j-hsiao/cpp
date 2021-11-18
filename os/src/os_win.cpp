#ifndef _CRT_SECURE_NO_WARNINGS
# define _CRT_SECURE_NO_WARNINGS 1
#endif

#include "os/os.hpp"
#include "os/tlog.hpp"

#include <cwchar>
#include <locale>
#include <memory>
#include <system_error>
#include <string>
#include <utility>
#include <stdio.h>
#include <io.h>
#include <map>
#include <climits>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlobj.h>
#include <Objbase.h>
#define INITGUID
#define INITKNOWNFOLDERS
#include <Knownfolders.h>
#include <accctrl.h>
#include <fcntl.h>
#undef WIN32_LEAN_AND_MEAN

//new process with piped in/out
//https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
namespace
{
#ifdef __MINGW32__
//mingw duplicates generic_category() into system_category()
//instead of using windows errors.

	struct Wincat: public std::error_category
	{
		std::map<int, std::string> msgs;
		Wincat(): msgs{
			{ERROR_FILE_NOT_FOUND, "file not found"},
			{ERROR_PATH_NOT_FOUND, "path not found"},
			{ERROR_FILE_EXISTS, "file exists"},
		}{}

		virtual const char* name() const noexcept
		{ return "Windows subcategory"; }

		virtual std::string message( int condition ) const
		{
			auto it = msgs.find(condition);
			if (it == msgs.end())
			{ return "unknown error " + std::to_string(condition); }
			else { return it->second; }
		}
		virtual std::error_condition default_error_condition(int code) const noexcept
		{
			if (
				code == ERROR_FILE_NOT_FOUND
				|| code == ERROR_PATH_NOT_FOUND)
			{
				return std::make_error_condition(
					std::errc::no_such_file_or_directory);
			}
			else if (code == ERROR_FILE_EXISTS)
			{ return std::make_error_condition(std::errc::file_exists); }
			else
			{ return {code, *this}; }
		}
		virtual bool equivalent(
			const std::error_code& code, int condition ) const noexcept
		{
			return (
				(code.category() == std::system_category())
				&& (
					((code.value() == ERROR_FILE_NOT_FOUND
						|| code.value() == ERROR_PATH_NOT_FOUND)
						&& condition == static_cast<int>(
							std::errc::no_such_file_or_directory))
					|| (
						code.value() == ERROR_FILE_EXISTS
						&& condition == static_cast<int>(
							std::errc::file_exists))
				));
		}
		virtual bool equivalent(
			int code, const std::error_condition& condition) const noexcept
		{
			return 
				(code == ERROR_FILE_EXISTS
					&& condition == std::make_error_condition(std::errc::file_exists))
				|| (
					(code == ERROR_PATH_NOT_FOUND || code == ERROR_FILE_NOT_FOUND)
					&& condition == std::make_error_condition(
						std::errc::no_such_file_or_directory));
		}
	};
	Wincat wincat;

	const std::error_condition noent(
		static_cast<int>(std::errc::no_such_file_or_directory), wincat);
	const std::error_condition exists(
		static_cast<int>(std::errc::file_exists), wincat);
#else
	const std::error_condition noent = std::make_error_condition(
		std::errc::no_such_file_or_directory);
	const std::error_condition exists = std::make_error_condition(
		std::errc::file_exists);
#endif

	std::wstring tows(const std::string &s)
	{
		if (s.size() > INT_MAX)
		{
			throw std::runtime_error(
				"strings > INT_MAX(" + std::to_string(INT_MAX) + ") not supported");
		}
		if (!s.size()) { return {}; }
		int sz = MultiByteToWideChar(
			CP_UTF8, 0, &s[0], static_cast<int>(s.size()), nullptr, 0);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "mbs->wcs size");
		}
		std::wstring out(sz, 0);
		sz = MultiByteToWideChar(
			CP_UTF8, 0, &s[0], static_cast<int>(s.size()), &out[0], sz);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "mbs->wcs conversion");
		}
		return out;
	}
	std::string tos(const std::wstring &ws)
	{
		if (ws.size() > INT_MAX)
		{
			throw std::runtime_error(
				"strings > INT_MAX(" + std::to_string(INT_MAX) + ") not supported");
		}
		int sz = WideCharToMultiByte(
			CP_UTF8, 0, &ws[0], static_cast<int>(ws.size()),
			nullptr, 0, NULL, nullptr);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "wcs->mbs size");
		}
		std::string out(sz, 0);
		sz = WideCharToMultiByte(
			CP_UTF8, 0, &ws[0], static_cast<int>(ws.size()),
			&out[0], sz, NULL, nullptr);
		if (sz == 0)
		{
			throw std::system_error(
				GetLastError(), std::system_category(), "wcs->mbs conversion");
		}
		return out;
	}

//wstring_convert is deprecated, try to use codecvt and locales directly
// 	std::locale getlocale()
// 	{
// 		try
// 		{
// 			os::cerr << "using en.us_utf-8" << os::endl;
// 			return std::locale("en.US_UTF-8");
// 		}
// 		catch (std::exception &e)
// 		{
// 			os::cerr << "falling back:" << e.what() << os::endl;
// 			return std::locale("C");
// 		}
// 	};
// 	//cannot convert directly to wchar_t because the max_length = 1
// 	//and converting unicode fails. Windows apis however require wchar_t
// 	//which, from research, seems to be same as char16_t (though diff signedness)
// 	typedef std::codecvt<char16_t, char, std::mbstate_t> Wcvt;
// 	static_assert(sizeof(char16_t) == sizeof(wchar_t), "windows code assumes wchar_t == char16_t");
// 	std::wstring tows(const std::string &s)
// 	{
// 		auto loc = getlocale();
// 		auto &converter = std::use_facet<Wcvt>(loc);
// 
// 		std::mbstate_t state;
// 		std::u16string us(s.size(), 0);
// 
// 		const char *src;
// 		std::u16string::value_type *dst;
// 		auto result = converter.in(
// 			state, &s[0], &s[0] + s.size(), src,
// 			&us[0], &us[0] + us.size(), dst);
// 		if (result != converter.ok)
// 		{ throw std::runtime_error("char->wchar conversion failed"); }
// 		return std::wstring(&us[0], dst);
// 	}
// 	std::string tos(const std::wstring &ws)
// 	{
// 		auto loc = getlocale();
// 		auto &converter = std::use_facet<Wcvt>(loc);
// 
// 		std::mbstate_t state;
// 		std::u16string us(ws.begin(), ws.end());
// 		std::string s(converter.max_length() * us.size(), 0);
// 
// 		const std::u16string::value_type *src;
// 		char *dst;
// 		auto result = converter.out(
// 			state,
// 			&us[0], &us[0] + us.size(), src,
// 			&s[0], &s[0] + s.size(), dst);
// 		if (result != converter.ok)
// 		{ throw std::runtime_error("wchar->char conversion failed"); }
// 		s.resize(dst - &s[0]);
// 		return s;
// 	}
}

namespace os
{
/*
Paths
*/
	Path Path::sysdir(const std::string &id)
	{
		if (id == ".")
		{ return Path(".").abs(); }
#		if _WIN32_WINNT >= _WIN32_WINNT_VISTA
		static const std::map<std::string, REFKNOWNFOLDERID> ids
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
			{"home", FOLDERID_Profile},
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
		auto it = ids.find(id);
		if (it != ids.end())
		{
			PWSTR wstr;
			HRESULT res = SHGetKnownFolderPath(it->second, 0, NULL, &wstr);
			if (res == S_OK)
			{
				auto ret = tos(wstr);
				CoTaskMemFree(wstr);
				return ret;
			}
			CoTaskMemFree(wstr);
		}
		throw std::system_error(noent.value(), noent.category(), id);
#		else
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
		auto it = ids.find(id);
		if (it != ids.end())
		{
			wchar_t res[MAX_PATH];
			if (
				SHGetFolderPathW(NULL, it->second, NULL, SHGFP_TYPE_CURRENT, res)==S_OK)
			{ return tos(res); }
		}
		throw std::system_error(noent.value(), noent.category(), id);
#		endif
	}
	Path& Path::abs()
	{
		std::wstring buf;
		auto wpath = tows(path);
		auto ret = GetFullPathNameW(wpath.c_str(), 0, &buf[0], nullptr);
		if (ret)
		{
			buf.resize(ret);
			ret = GetFullPathNameW(wpath.c_str(), ret, &buf[0], nullptr);
			if (ret)
			{
				buf.resize(ret);
				path = tos(buf);
				return *this;
			}
		}
		throw std::system_error(GetLastError(), std::system_category(), path);
	}
/*
Env
*/
	std::string getenv(const std::string &name)
	{
		std::wstring wbuf;
		auto wname = tows(name);
		DWORD ret = GetEnvironmentVariableW(
			wname.c_str(), &wbuf[0], 0);
		if (ret)
		{
			wbuf.resize(ret);
			ret = GetEnvironmentVariableW(
				wname.c_str(), &wbuf[0], ret);
		}
		if (ret)
		{
			wbuf.resize(ret);
			return tos(wbuf);
		}
		else
		{
			DWORD e = GetLastError();
			if (e == ERROR_SUCCESS)
			{ return ""; }
			else
			{
				if (e == ERROR_ENVVAR_NOT_FOUND)
				{
					throw std::system_error(
						std::make_error_code(std::errc::no_such_file_or_directory)); }
				else
				{ throw std::system_error(e, std::system_category(), name); }
			}
		}
	}
	void setenv(const std::string &name, const char *val)
	{
		std::wstring wname = tows(name);
		const wchar_t *wval = nullptr;
		std::wstring wvalue;
		if (val)
		{
			wvalue = tows(val);
			wval = wvalue.c_str();
		}
		if (SetEnvironmentVariableW(wname.c_str(), wval) == 0)
		{ throw std::system_error(GetLastError(), std::system_category(), "setting env var "+name); }
	}
/*
Dirlist
*/
	struct Dirlist::iterator_type::info
	{
		info(const std::string &dname):
			winfo{},
			handle{
				FindFirstFileW(
//for some reason, the \\?\ prefix makes R: (ramdisk) unable to be searched
					// (L"\\\\?\\" + tows((Path(dname).abs()+="*").path)).c_str(),
					tows((Path(dname)+="*").path).c_str(),
					&winfo)}
		{
			if (handle == INVALID_HANDLE_VALUE)
			{
				throw std::system_error(GetLastError(), std::system_category(), dname);
			}
			val = tos(winfo.cFileName);
			if (val == "." || val == "..")
			{ next(); }
		}
		void next()
		{
			if (FindNextFileW(handle, &winfo))
			{
				val = tos(winfo.cFileName);
				if (val == "." || val == "..")
				{ next(); }
			}
			else
			{
				DWORD e = GetLastError();
				if (e == ERROR_NO_MORE_FILES)
				{ val = ""; }
				else
				{ throw std::system_error(e, std::system_category()); }
			}
		}
		WIN32_FIND_DATAW winfo;
		HANDLE handle;
		std::string val;
		~info()
		{
			if (handle != INVALID_HANDLE_VALUE)
			{ FindClose(handle); }
		}
	};


	Dirlist::iterator_type::iterator_type(): data{} {}
	Dirlist::iterator_type::iterator_type(const std::string &dname):
		data{new info{dname}}
	{}
	Dirlist::iterator_type::iterator_type(iterator_type &&o)
	{ data = std::move(o.data); }
	Dirlist::iterator_type::~iterator_type()
	{ data.reset(nullptr); }

	std::string Dirlist::iterator_type::operator*() const
	{ return data ? data->val : ""; }
	Dirlist::iterator_type& Dirlist::iterator_type::operator++()
	{
		data->next();
		if (data->val == "")
		{ data.reset(nullptr); }
		return *this;
	}
	bool Dirlist::iterator_type::operator!=(const iterator_type &o)
	{ return !data != !o.data; }

/*
Filenode
*/
	Filenode::Filenode(const std::string &name):
		flags(GetFileAttributesW(tows(name).c_str()))
			//(L"\\\\?\\"+tows(Path(name).abs().path)).c_str())
	{
		if (flags == INVALID_FILE_ATTRIBUTES)
		{
			std::error_code code(GetLastError(), std::system_category());
			if (code != noent) { throw std::system_error(code, name); }
			else { flags = 0; }
		}
	}

	Filenode::operator bool() const
	{ return flags; }
	bool Filenode::isdir() const
	{ return flags & FILE_ATTRIBUTE_DIRECTORY; }
	//...
	bool Filenode::isfile() const
	{
		//there's a GetFileType but need to open a handle,
		//this is probably sufficient?
		return !isdir() && flags & (
			FILE_ATTRIBUTE_ARCHIVE
			| FILE_ATTRIBUTE_COMPRESSED
			| FILE_ATTRIBUTE_ENCRYPTED
			| FILE_ATTRIBUTE_HIDDEN
			| FILE_ATTRIBUTE_NORMAL
			| FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
			| FILE_ATTRIBUTE_OFFLINE
			| FILE_ATTRIBUTE_READONLY
			// | FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
			// | FILE_ATTRIBUTE_RECALL_ON_OPEN
			| FILE_ATTRIBUTE_REPARSE_POINT
			| FILE_ATTRIBUTE_SPARSE_FILE
			| FILE_ATTRIBUTE_SYSTEM
			| FILE_ATTRIBUTE_TEMPORARY);
	}

	void mkdir(const std::string &path)
	{
		if (!CreateDirectoryW(tows(path).c_str(), NULL))
		{
			std::error_code code(GetLastError(), std::system_category());
			if (code != exists)
			{ throw std::system_error(code, path); }
		}
	}

	void rename(const std::string &name, const std::string &to)
	{
		std::string out;
		Filenode dst(to);
		if (dst)
		{
			out = (Path(to) + Path(name).basename()).path;
			if (!dst.isdir() || Filenode(out))
			{ throw std::system_error(exists.value(), exists.category(), "rename "+name+"->"+to); }
		}
		else { out = to; }
		if (
			!MoveFileExW(
				tows(name).c_str(),
				tows(out).c_str(),
				MOVEFILE_COPY_ALLOWED))
		{ throw std::system_error(GetLastError(), std::system_category(), "rename "+name+"->"+to); }
	}
	void remove(const std::string &path)
	{
		Filenode node(path);
		if (node)
		{
			bool success;
			if (node.isdir())
			{ success = RemoveDirectoryW(tows(path).c_str()); }
			else
			{ success = DeleteFileW(tows(path).c_str()); }
			if (!success)
			{
				std::error_code code(GetLastError(), std::system_category());
				if (code != noent) { throw std::system_error(code, path); }
			}
		}
	}
	void hide_window()
	{
		HWND handle = GetConsoleWindow();
		if (handle != NULL)
		{ ShowWindow(handle, SW_HIDE); }
	}
	void show_window()
	{
		HWND handle = GetConsoleWindow();
		if (handle != NULL)
		{ ShowWindow(handle, SW_SHOW); }
	}

	Term::Term(const std::string &mode): tptr{nullptr}
	{
		//?? can't really find documentation on CONIN/CONOUT/CON
		std::wstring wname = tows("CONIN$");
		//GENERIC_WRITE is necessary because otherwise, SetConsoleMode to hide
		//input will fail. GENERIC_READ is necessary to read data/mode, etc
		HANDLE h = CreateFileW(
			wname.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (h == INVALID_HANDLE_VALUE)
		{ throw std::system_error(GetLastError(), std::system_category()); }
		int fd = _open_osfhandle(reinterpret_cast<intptr_t>(h), 0);
		if (fd != -1)
		{
			tptr = _fdopen(fd, mode.c_str());
			if (tptr) { return; }
			else
			{
				_close(fd);
				throw std::system_error(errno, std::generic_category());
			}
		}
		else
		{
			CloseHandle(h);
			throw std::system_error(
				std::make_error_code(std::errc::bad_file_descriptor));
		}
		//FILE* obtained via fopen always fail with SetConsoleMode in testing.
		// tptr = fopen("CONIN$", mode.c_str());
		// if (!tptr)
		// {
			// throw std::system_error(
				// errno, std::generic_category(), "could not open terminal");
		// }
	}
	Term::~Term() { if (tptr) { ::fclose(tptr); } }
	bool Term::show_input(bool show)
	{
		int fd = ::_fileno(tptr);
		if (fd == -1)
		{ throw std::system_error(errno, std::generic_category(), "get console fd"); }
		else if (fd == -2) //no console window, no input/output to show/hide
		{ return 0; }
		HANDLE h = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
		if (h == INVALID_HANDLE_VALUE)
		{ throw std::system_error(GetLastError(), std::system_category(), "get console handle"); }
		DWORD mode;
		if (!GetConsoleMode(h, &mode))
		{ throw std::system_error(GetLastError(), std::system_category(), "get console mode"); }
		bool ret = mode & ENABLE_ECHO_INPUT;
		if (show)
		{ mode |= ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT; }
		else
		{ mode &= ~ENABLE_ECHO_INPUT; }
		if (!SetConsoleMode(h, mode))
		{ throw std::system_error(GetLastError(), std::system_category(), "set console mode"); }
		return ret;
	}
}
