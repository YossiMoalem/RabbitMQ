#ifndef DEBUG_H
#define DEBUG_H


#include <iostream>
#include <sstream>
#include <ctime>
#include <thread>

#ifndef DEBUG
#define DEBUG 0
#endif

#define PRINT_DEBUG(SEVERITY, MSG) do{\
if(SEVERITY >= DEBUG) {\
    std::stringstream ss;\
    ss<< time(0) <<":" << std::this_thread::get_id() <<":";\
    ss<< MSG <<std::endl;\
    std::cerr << ss.str();}\
}while(0);

#endif
