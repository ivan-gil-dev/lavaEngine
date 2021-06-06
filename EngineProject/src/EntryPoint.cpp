#define TINYOBJLOADER_IMPLEMENTATION
#  pragma warning( push )
#  pragma warning( disable: 4099 )

#include	"Headers/Application.h"
#include <spdlog/sinks/basic_file_sink.h>

__declspec(dllexport) int main() {
    try {
        Engine::Globals::App.Init();
        Engine::Globals::App.Execute();
        Engine::Globals::App.Clear();
    }
    catch (const std::exception& e) {
        spdlog::error(e.what());
        Engine::Globals::gLogger->error(e.what());
        std::cin.get();
    }
    return 0;
}

#  pragma warning( pop )