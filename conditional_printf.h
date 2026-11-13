#define ON_DEBUG

#ifdef  ON_DEBUG
    #define PRINTF(...) printf(__VA_ARGS__)
    #define GETCHAR()   getchar()
#endif

#ifndef ON_DEBUG
    #define PRINTF(...) (void(0))
    #define GETCHAR()   (void(0))
#endif