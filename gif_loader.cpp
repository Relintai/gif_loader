
#include "gif_loader.h"

Array GIFLoader::load_gif(const String &file) {
	Array images;

	return images;
}

GIFLoader::GIFLoader() {
}
GIFLoader::~GIFLoader() {
}

void GIFLoader::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load_gif", "file"), &GIFLoader::load_gif);
}