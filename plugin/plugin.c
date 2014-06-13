#include "plugin.h"
inline void *open_plugin()
{
	void *handle = dlopen(LIBRARY_PATH, RTLD_NOW);

	if (!handle) {
		//printf("%s\n", dlerror());
		return handle;
	}

	return dlsym(handle, FUNC_NAME);
}
