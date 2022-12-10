#include "register_types.h"

#include "gif_loader.h"

void register_gif_loader_types() {
    ClassDB::register_class<GIFLoader>();
}

void unregister_gif_loader_types() {
}
