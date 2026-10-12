#include "Galil.h"
#include "EmbeddedFunctions.h"

int main(void) {
    EmbeddedFunctions funcs;
    Galil galil(&funcs, "192.168.0.120 -d");
}
