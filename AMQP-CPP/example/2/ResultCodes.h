#ifndef RESULT_CODES_H
#define RESULT_CODES_H

#include <memory>
#include <future>

typedef std::future< bool >                     DeferedResult;
typedef std::shared_ptr< std::promise< bool > > DeferedResultSetter;
#define dummyResultSetter nullptr


#endif
