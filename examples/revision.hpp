// This file was generated by VersionString.cmake

#pragma once

#include <ostream>

constexpr const char kProjectName[] = "mgui-gtk4-examples";
constexpr const char kVersionNumber[] = "1.0.0";
constexpr int kBuildNumber = 150;
constexpr const char kRevisionGitTag[] = "9df9f95*";
constexpr const char kRevisionDate[] = "2024-04-07T15:33:08Z";

#ifndef VERSION_INFO_DEFINED
#define VERSION_INFO_DEFINED 1

namespace version_info_v1_2
{

class version_info_base
{
  public:
	static void write_version_string(std::ostream &os, bool verbose)
	{
		auto s_main = registered_main();
		if (s_main != nullptr)
			s_main->write(os, verbose);

		if (verbose)
		{
			for (auto lib = registered_libraries(); lib != nullptr; lib = lib->m_next)
			{
				os << "-\n";
				lib->write(os, verbose);
			}
		}
	}

  protected:
	version_info_base(const char *name, const char *version, int build_number, const char *git_tag, const char *revision_date, bool is_main)
		: m_name(name)
		, m_version(version)
		, m_build_number(build_number)
		, m_git_tag(git_tag)
		, m_revision_date(revision_date)
	{
		if (is_main)
			registered_main() = this;
		else
		{
			auto &s_head = registered_libraries();
			m_next = s_head;
			s_head = this;
		}
	}

	void write(std::ostream &os, bool verbose)
	{
		os << m_name << " version " << m_version << '\n';

		if (verbose)
		{
			if (m_build_number != 0)
			{
				os << "build: " << m_build_number << ' ' << m_revision_date << '\n';
				if (m_git_tag[0] != 0)
					os << "git tag: " << m_git_tag << '\n';
			}
		}
	}

	using version_info_ptr = version_info_base *;

	static version_info_ptr &registered_main()
	{
		static version_info_ptr s_main = nullptr;
		return s_main;
	}

	static version_info_ptr &registered_libraries()
	{
		static version_info_ptr s_head = nullptr;
		return s_head;
	}

	const char *m_name;
	const char *m_version;
	int m_build_number;
	const char *m_git_tag;
	const char *m_revision_date;
	version_info_base *m_next = nullptr;
};

template <typename T>
class version_info : public version_info_base
{
  public:
	using implementation_type = T;

	version_info(const char *name, const char *version, int build_number, const char *git_tag, const char *revision_date, bool is_main)
		: version_info_base(name, version, build_number, git_tag, revision_date, is_main)
	{
	}
};

} // namespace version_info_v1_2

inline void write_version_string(std::ostream &os, bool verbose)
{
	version_info_v1_2::version_info_base::write_version_string(os, verbose);
}

#endif

const class version_info_impl : public version_info_v1_2::version_info<version_info_impl>
{
  public:
	version_info_impl()
		: version_info(kProjectName, kVersionNumber, kBuildNumber, kRevisionGitTag, kRevisionDate, true)
	{
	}
} s_version_info_instance;
