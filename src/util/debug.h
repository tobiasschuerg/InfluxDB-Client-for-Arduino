/**
 * 
 * debug.h: InfluxDB Client debug macros
 * 
 * MIT License
 * 
 * Copyright (c) 2020 InfluxData
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#ifndef _INFLUXDB_CLIENT_DEBUG_H
#define _INFLUXDB_CLIENT_DEBUG_H

#include <Arduino.h>

// Uncomment bellow in case of a problem and rebuild sketch
//#define INFLUXDB_CLIENT_DEBUG_ENABLE

#ifdef INFLUXDB_CLIENT_DEBUG_ENABLE
# define INFLUXDB_CLIENT_DEBUG(fmt, ...) Serial.printf("%.03f ",millis()/1000.0f);Serial.printf_P( (PGM_P)PSTR(fmt), ## __VA_ARGS__ )
#else
# define INFLUXDB_CLIENT_DEBUG(fmt, ...)
#endif //INFLUXDB_CLIENT_DEBUG

#endif //# _INFLUXDB_CLIENT_DEBUG_H
