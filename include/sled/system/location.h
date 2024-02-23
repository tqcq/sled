/**
 * @file     : location
 * @created  : Sunday Feb 04, 2024 20:58:26 CST
 * @license  : MIT
 **/

#ifndef LOCATION_H
#define LOCATION_H

namespace sled {

class Location {
public:
    static Location Current() { return Location(); }
};

}// namespace sled

#endif// LOCATION_H
