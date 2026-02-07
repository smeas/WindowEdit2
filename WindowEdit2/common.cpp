#include "common.h"
#include <ShlObj.h>
#pragma comment(lib, "shell32.lib")

bool ShellOpenFolderAndSelectFile(const wchar_t* filePath)
{
	PIDLIST_ABSOLUTE pidl = ILCreateFromPath(filePath);
	if (!pidl)
		return false;

	HRESULT hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);

	ILFree(pidl);

	return SUCCEEDED(hr);
}
