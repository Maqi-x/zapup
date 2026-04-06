#include <zapup/app.h>

int main(int argc, const char* const* argv) {
    ZapupApp app; 

    zapup_init(&app);
    int code = zapup_run(&app, argc, argv);
    zapup_destroy(&app);

    return code;
}
