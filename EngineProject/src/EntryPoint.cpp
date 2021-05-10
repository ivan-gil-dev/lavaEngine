#define TINYOBJLOADER_IMPLEMENTATION
#include	"Headers/Application.h"
#include <spdlog/sinks/basic_file_sink.h>



int main() {

	try {
		Engine::Globals::App.Init();
		Engine::Globals::App.Execute();
		Engine::Globals::App.Clear();
	}
	catch (const std::exception & e) {
		spdlog::error(e.what());
		Engine::Globals::gLogger->error(e.what());
		std::cin.get();

	}
	


	return 0;
}



