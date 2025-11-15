#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#endif


#endif