#include "Galil.h"
#include "EmbeddedFunctions.h"

int main() {
    EmbeddedFunctions a(true);
    EmbeddedFunctions b(a);

    return 0;
}

//int main(void) {
//
//	return 0;
//}