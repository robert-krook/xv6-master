#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"







int main(int argc, char *argv[])
{

outpw


    exit();
}

unsigned short BgaReadRegister(unsigned short IndexValue)
{
    outpw(VBE_DISPI_IOPORT_INDEX, IndexValue);
    return inpw(VBE_DISPI_IOPORT_DATA);
}

int BgaIsAvailable(void)
{
    return (BgaReadRegister(VBE_DISPI_INDEX_ID) == VBE_DISPI_ID4);
}
