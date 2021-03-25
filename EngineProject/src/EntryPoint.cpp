#define TINYOBJLOADER_IMPLEMENTATION
#include	"Headers/Application.h"

#define _CRTDBG_MAP_ALLOC
#ifdef _DEBUG
	#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
	#define new DEBUG_NEW
#endif
#include <stdlib.h>
#include <crtdbg.h>


int main() {
	try {
		Engine::Globals::App.Init();
		Engine::Globals::App.Execute();
		Engine::Globals::App.Clear();
	}
	catch (const std::exception & e) {
		spdlog::error(e.what());
	}
	//
	_CrtDumpMemoryLeaks();

	return 0;
}



