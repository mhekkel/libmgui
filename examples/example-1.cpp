#include "MApplication.hpp"
#include "MWindow.hpp"

#include <iostream>

const char kAppName[] = "ExampleApp";

class ExampleApp : public MApplication
{
  public:
	ExampleApp(MApplicationImpl *impl)
		: MApplication(impl)
	{

	}

	void Initialise() override
	{
		MApplication::Initialise();

		std::cout << "Initialize\n";
	}

	void Execute(const std::string &command, const std::vector<std::string> &arguments)
	{
		if (command == "New")
			DoNew();
	}

	void DoNew() override
	{
		std::cout << "DoNew\n";
	}
};

MApplication *MApplication::Create(MApplicationImpl *inImpl)
{
	return new ExampleApp(inImpl);
}

// --------------------------------------------------------------------

int main(int argc, char *argv[])
{
	ExampleApp::Main("New", {});

	return 0;
}
