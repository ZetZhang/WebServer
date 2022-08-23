#include "src/server/webserver.h"

int main(int argc, char *argv[])
{
    wsv::WebServer server(12309, 3, 60000, false, 3306, "root", "zjt152445", "yourdb", 12, 6, true, 1, 1024);
    server.start();
}
