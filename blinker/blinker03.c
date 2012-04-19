
void dummy ( unsigned int );
void PUT32 ( unsigned int, unsigned int );
unsigned int GET32 ( unsigned int );

#define PORTECLR 0xBF886114
#define PORTESET 0xBF886118

void dowait ( void )
{
    unsigned int ra;

    for(ra=0;ra<20;ra++) dummy(ra);
}

void notmain ( void )
{
    while(1)
    {
        PUT32(PORTECLR,0x1);
        dowait();
        PUT32(PORTESET,0x1);
        dowait();
    }
}


