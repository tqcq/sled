/**
 * @file     : reflect
 * @created  : 星期一  2 26, 2024 09:55:19 CST
 * @license  : MIT
 **/

#pragma
#ifndef SLED_REFLECT_REFLECT_H
#define SLED_REFLECT_REFLECT_H

#if !defined(__NO_META_PARSER__) && !defined(__META_PARSER__)
#define __META_PARSER__
#endif

#if defined(__META_PARSER__)
#define CLASS(...) class __attribute__((annotate("reflect-class"))) #__VA_ARGS__
#define PROPERTY() __attribute__((annotate("reflect-property")))
#define METHOD() __attribute__((annotate("reflect-method")))
#else
#define REFLECT_CLASS
#define PROPERTY()
#define METHOD()
#endif

#endif// SLED_REFLECT_REFLECT_H
