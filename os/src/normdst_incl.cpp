namespace
{
	std::string normalize_dst(const char *src, const char *dst, os__ebuf *buf)
	{
		try
		{
			std::string ret(dst);
			if (!ret.size())
			{
				if (buf) { *buf << std::errc::invalid_argument; }
				return "";
			}
			os__Filenode node = os__Filenode__make(dst, buf);
			if (node < 0) { return ""; }
			if (os__Filenode__exists(node))
			{
				if (os__Filenode__is_dir_nc(node))
				{
					os__Path srcp(src);
					os__Path dstp(dst);
					dstp.add(srcp.base());
					ret = dstp.c_str();
				}
				else
				{
					if (buf) { *buf << std::errc::file_exists; }
					return "";
				}
			}
			return ret;
		}
		catch (std::exception &exc)
		{
			if (buf) { *buf << exc; }
			return "";
		}
	}
}
