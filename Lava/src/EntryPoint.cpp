#define TINYOBJLOADER_IMPLEMENTATION

#include	"Headers/Application.h"
#include    <fcntl.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, _FILE_, _LINE_)
#define new DEBUG_NEW

int main() {
	AllocConsole();

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	setvbuf(hf_out, NULL, _IONBF, 1);
	*stdout = *hf_out;

	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	setvbuf(hf_in, NULL, _IONBF, 128);
	*stdin = *hf_in;

	try {
		Application.Init();
		Application.Execute();
		Application.Clear();
	}
	catch (const std::exception & e) {
		spdlog::error(e.what());
		
	}
	
	_CrtDumpMemoryLeaks();

	return 0;
}



