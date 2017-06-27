#include <truth/log.h>
#include <truth/types.h>

constructor enum status init() {
    log(Log_Debug, "Hello world!");
    return Ok;
}

destructor enum status fini() {
    log(Log_Debug, "Bye bye!");
    return Ok;
}
