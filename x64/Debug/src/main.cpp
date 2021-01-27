#define TINYOBJLOADER_IMPLEMENTATION

#include	"Application.h"

int main(){

	try {
		app.Init();
		app.Loop();
		app.Clear();
	}

	 catch (const std::exception& e) {
		std::cerr<<e.what()<<std::endl;
	}
	
	return 0;
}