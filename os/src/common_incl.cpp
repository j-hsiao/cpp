#include <string>
#include <vector>
int os__makedirs(const char *path, os__ebuf *buf)
{
	try
	{
		os__Path p(path);
		int code = 0;
		std::vector<std::string> stack;
		os__Filenode node;
		while (
			(node = os__Filenode__make(p.c_str(), buf)) >= 0
			&& !os__Filenode__exists(node)
			&& p.inds.size())
		{
			Log() << "need to make " << p.c_str() << std::endl;
			stack.push_back(p.base());
			p.dirname();
		}
		if (node < 0) { return -1; }
		if (!os__Filenode__is_dir(node))
		{
			Log() << "exists?" << os__Filenode__exists(node) << std::endl;
			code = -1;
			stack.clear();
			if (buf) { *buf << std::errc::not_a_directory; }
		}
		while (stack.size() && code == 0)
		{
			p += stack.back();
			code = os__mkdir(p.c_str(), buf);
			stack.pop_back();
		}
		if (code)
		{
			Log() << "error at " << p.c_str() << std::endl;
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
