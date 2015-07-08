/******************************************************************* 
FileName: macro.h
Author  : vee
Date    : 2015-07-01 00:59:06
Desc    : useful macros for c++ programming
*******************************************************************/ 

#ifndef _VEE_MACRO_H_
#define _VEE_MACRO_H_

#define _VEE_BEGIN	namespace vee {
#define _VEE_END	}
#define _VEE	    ::vee::
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
TypeName(const TypeName&) = delete; \
void operator=(const TypeName&) = delete
#define DISALLOW_MOVE_AND_ASSIGN(TypeName) \
TypeName(const TypeName&&) = delete; \
void operator=(const TypeName&&) = delete
#define VEE_EMPTY_FUNCTION {}

#endif // _VEE_MACRO_H_