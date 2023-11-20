#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#include <string>

class Injector
{
public:
	static bool Inject(std::string process_name, std::string dll_path);
private:
	static DWORD GetPidFromName(std::string process_name);
};
#endif // INJECTOR_HPP