#define LIBRARY_PATH "./libaids.so"
#define FUNC_NAME "hore"
#define CONFIG_PATH "./config"
#include <dlfcn.h>

void (*open_plugin())()
{
	void (*funcp)();
	void *handle = dlopen(LIBRARY_PATH, RTLD_NOW);

	if (!handle) {
		printf("%s\n", dlerror());
		return NULL;
	}

	*(void**)(&funcp) = dlsym(handle, FUNC_NAME);

	if (!funcp) {
		printf("%s\n", dlerror());
		return NULL;
	}

	return funcp;
}

