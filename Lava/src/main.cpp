#define TINYOBJLOADER_IMPLEMENTATION

#include "Window.h"

int main(){
	setlocale(LC_ALL,"rus");
	try {
		window.Init();
		window.Loop();
		window.Clear();
	}
	catch (const std::exception& e) {
		std::cerr<<e.what()<<std::endl;
	}
	
	return 0;
}