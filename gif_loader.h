
#ifndef GIF_LOADER_H
#define GIF_LOADER_H

#include "core/object/reference.h"

#include "core/io/image.h"

class GIFLoader : public Reference {
    GDCLASS(GIFLoader, Reference);

public:
	Array load_gif(const String &file);

	GIFLoader();
	~GIFLoader();

private:
	static void _bind_methods();
};

#endif 
