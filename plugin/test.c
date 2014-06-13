#include <stdio.h>
#include "plugin.h"

int main()
{
	void (*plugin)(int);
	plugin = open_plugin();

	if (plugin != NULL)
		plugin(1);

	return 0;
}
