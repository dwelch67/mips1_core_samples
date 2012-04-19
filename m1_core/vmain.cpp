
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#include "Vm1_core.h"

Vm1_core *top;
#if VM_TRACE
VerilatedVcdC *trace;
#endif

char gstring[80];
char newline[1024];
unsigned char hexline[1024];

//16Kbytes
#define RAMMASK 0x00FFFFFF
unsigned int ram[(RAMMASK+1)>>2];

//-----------------------------------------------------------------------------
int readhex ( FILE *fp )
{
//unsigned int errors;

unsigned int addhigh;
unsigned int add;

unsigned int ra;
//unsigned int rb;

//unsigned int pages;
//unsigned int page;


unsigned int line;

unsigned char checksum;

unsigned int len;
unsigned int hexlen;
unsigned int maxadd;

unsigned char t;

    maxadd=0;

    addhigh=0;
    memset(ram,0xFF,sizeof(ram));

    line=0;
    while(fgets(newline,sizeof(newline)-1,fp))
    {
        line++;
        //printf("%s",newline);
        if(newline[0]!=':')
        {
            printf("Syntax error <%u> no colon\n",line);
            continue;
        }
        gstring[0]=newline[1];
        gstring[1]=newline[2];
        gstring[2]=0;
        len=strtoul(gstring,NULL,16);
        for(ra=0;ra<(len+5);ra++)
        {
            gstring[0]=newline[(ra<<1)+1];
            gstring[1]=newline[(ra<<1)+2];
            gstring[2]=0;
            hexline[ra]=(unsigned char)strtoul(gstring,NULL,16);
        }
        checksum=0;
        for(ra=0;ra<(len+5);ra++) checksum+=hexline[ra];
        //checksum&=0xFF;
        if(checksum)
        {
            printf("checksum error <%u>\n",line);
        }
        add=hexline[1]; add<<=8;
        add|=hexline[2];
        add|=addhigh;
        if(add>RAMMASK)
        {
            printf("address too big 0x%08X\n",add);
            //return(1);
            continue;
        }
        if(add&3)
        {
            printf("bad address 0x%08X\n",add);
            return(1);
        }
        t=hexline[3];
        if(t!=0x02)
        {
            if(len&3)
            {
                printf("bad length\n");
                return(1);
            }
        }

        //:0011223344
        //;llaaaattdddddd
        //01234567890
        switch(t)
        {
            default:
                printf("UNKOWN type %02X <%u>\n",t,line);
                break;
            case 0x00:
                for(ra=0;ra<len;ra+=4)
                {
                    if(add>RAMMASK)
                    {
                        printf("address too big 0x%08X\n",add);
                        break;
                    }
                    ram[add>>2]                  =hexline[ra+4+3];
                    ram[add>>2]<<=8; ram[add>>2]|=hexline[ra+4+2];
                    ram[add>>2]<<=8; ram[add>>2]|=hexline[ra+4+1];
                    ram[add>>2]<<=8; ram[add>>2]|=hexline[ra+4+0];
                    //printf("%08X: %02X\n",add,t);
                    add+=4;
                    if(add>maxadd) maxadd=add;
                }
                break;
            case 0x01:
//                printf("End of data\n");
                break;
            case 0x02:
                addhigh=hexline[5];
                addhigh<<=8;
                addhigh|=hexline[4];
                addhigh<<=16;
                break;
        }
    }

    //printf("%u lines processed\n",line);
    //printf("%08X\n",maxadd);

    //for(ra=0;ra<maxadd;ra+=4)
    //{
        //printf("0x%08X: 0x%08X\n",ra,ram[ra>>2]);
    //}

    return(0);


}

int main(int argc, char *argv[])
{
    unsigned int lasttick;
    unsigned int tick;
    unsigned int addr;
    unsigned int mask;
    unsigned int simhalt;
    unsigned int did_reset;

    FILE *fp;

//    Verilated::commandArgs(argc, argv);

    if(argc<2)
    {
        fprintf(stderr,".hex file not specified\n");
        return(1);
    }
    fp=fopen(argv[1],"rt");
    if(fp==NULL)
    {
        fprintf(stderr,"Error opening file [%s]\n",argv[1]);
        return(1);
    }
    if(readhex(fp)) return(1);
    fclose(fp);


#if VM_TRACE
    Verilated::traceEverOn(true);
#endif

    top = new Vm1_core;

#if VM_TRACE
    trace = new VerilatedVcdC;
    top->trace(trace, 99);
    trace->open("test.vcd");
#endif

    top->sys_clock_i = 0;
    top->sys_reset_i = 1;
    top->sys_irq_i = 1;

    simhalt=0;
    did_reset=0;
    tick=0;
    lasttick=tick;
    while (!Verilated::gotFinish())
    {

        tick++;
        if(tick<lasttick) printf("tick rollover\n");
        lasttick=tick;

        if((tick&1)==0)
        {
        }
        if(did_reset)
        {
            if(top->imem_read)
            {
                top->imem_data = ram[(top->imem_addr&RAMMASK)>>2];
            }
            top->imem_done = top->imem_read;
            if(top->dmem_write)
            {
                if(top->dmem_addr&0x80000000)
                {
                    if((tick&1)==0)
                    {
                        //if(top->dmem_sel==0x0F)
                        {
                            //all lanes on, just write
                            if(top->dmem_addr==0xBF886114)
                            {
                                printf("write [0x%X]=0x%08X\n",top->dmem_addr,top->dmem_data_w);
                            }
                            if(top->dmem_addr==0xBF886118)
                            {
                                printf("write [0x%X]=0x%08X\n",top->dmem_addr,top->dmem_data_w);
                            }
                        }
                    }
                }
                else
                {
                    addr=top->dmem_addr&RAMMASK;
                    if((tick&1)==0)
                    {
                        if(top->dmem_sel==0x0F)
                        {
                            //all lanes on, just write
                            ram[addr>>2] = top->dmem_data_w;

                            if(addr==0xBF886114)
                            {
                                printf("write ram[0x%X]=0x%08X\n",addr,top->dmem_data_w);
                            }
                            if(addr==0xBF886118)
                            {
                                printf("write ram[0x%X]=0x%08X\n",addr,top->dmem_data_w);
                            }
                        }
                        else
                        {
                            //read-modify-write
                            mask=0;
                            if(top->dmem_sel&1) mask|=0x000000FF;
                            if(top->dmem_sel&2) mask|=0x0000FF00;
                            if(top->dmem_sel&4) mask|=0x00FF0000;
                            if(top->dmem_sel&8) mask|=0xFF000000;
                            ram[addr>>2]&=~mask;
                            ram[addr>>2]|=top->dmem_data_w&mask;
                            //printf("write ram[0x%X]=0x%08X\n",addr,ram[addr>>2]);
                        }
                    }
                }
            }
            if(top->dmem_read)
            {
                top->dmem_data_r = ram[(top->dmem_addr&RAMMASK)>>2];
            }
            top->dmem_done = top->dmem_read|top->dmem_write;
        }
        else
        {
            if (tick > 111)
            {
                top->sys_reset_i = 0;
                did_reset = 1;
            }
        }

        top->sys_clock_i = (tick & 1);
        top->eval();
#if VM_TRACE
        trace->dump(tick);
        if(tick>20000) break;
#endif
        //if(simhalt) break;
    }
#if VM_TRACE
    trace->close();
#endif
    top->final();

printf("Hello %u\n",tick);

    return 0;
}

