#ifndef DEBUG_H
#define DEBUG_H

#define DEBUG 1

#if DEBUG == 0
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define debugflush(x) Serial.flush(x)

#else
//Set debug statements to nothing
#define debug(x)
#define debugln(x)
#define debugflush(x)

#endif


#endif