# simple-windows-service

This repo contains simple Windows Service, that can be extended with your own code. There are 2 source files that you may
be interested in:

* **service.cpp** - plain skeleton for your own usage,
* **service_sample_write.cpp** - same as above, but with a simple mechanism that writes current date to C:\file.txt every 10
seconds.

For more information about this code, visit: http://itachi.pl/teeny-tiny-windows-service-w-cpp/