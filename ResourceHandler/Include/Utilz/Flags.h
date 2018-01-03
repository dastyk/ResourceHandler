#ifndef _UTILZ_FLAGS_H_
#define _UTILZ_FLAGS_H_

#define ENUM_FLAG_OPERATOR(T,X) inline T operator X (const T& lhs, const T& rhs) { return (T) (static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
#define ENUM_FLAG_OPERATOR2(T,X) inline void operator X= ( T& lhs,const T& rhs) { lhs = (T)(static_cast<std::underlying_type_t <T>>(lhs) X static_cast<std::underlying_type_t <T>>(rhs)); } 
#define ENUM_FLAGS(T) \
inline T operator ~ (const T& t) { return (T) (~static_cast<std::underlying_type_t <T>>(t)); } \
inline bool operator & (const T& lhs, const T& rhs) { return (static_cast<std::underlying_type_t <T>>(lhs) & static_cast<std::underlying_type_t <T>>(rhs));  } \
ENUM_FLAG_OPERATOR2(T,|) \
ENUM_FLAG_OPERATOR2(T,&) \
ENUM_FLAG_OPERATOR(T,|) \
ENUM_FLAG_OPERATOR(T,^)
//enum class T
//ENUM_FLAG_OPERATOR(T,&)

#endif