// Multiprocessor support
// Search memory for MP description structures.
// http://developer.intel.com/design/pentium/datashts/24201606.pdf

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"

struct cpu cpus[NCPU];
static struct cpu *bcpu;
int ismp;
int ncpu;
uchar ioapicid;


// // Validate ACPI checksum
// int 
// check_rsdp (uint8 *ptr, int length) {
//     uint8 sum = 0;
//     for (int i = 0; i < length; i++) {
//         sum += ptr[i];
//     }
//     return (sum == 0);
// }

// // Search a specific memory block on 16-byte boundaries
// rsdp_t * 
// scan_memory(void *start, int length) 
// {
//     for (void *addr = start; addr < start + length; addr += 16) {
//         rsdp_t *rsdp = (rsdp_t*)addr;
//         if (memcmp(rsdp->signature, "RSD PTR ", 8) == 0) {
//             if (check_rsdp((uint8 *)rsdp, rsdp->revision == 0 ? 20 : rsdp->length)) {
//                 return rsdp;
//             }
//         }
//     }
//     return nullptr;
// }

// Locate RSDP main entry point
// rsdp_t * find_rsdp (void) 
// {
//     rsdp_t *rsdp = nullptr;

// cprintf ("find_rsdp\n");

//     // 1. Search EBDA (Extended BIOS Data Area) segment pointer at 0x40E
//     uint ebda_segment = *(uint *)0x040E;

// cprintf ("find_rsdp-2\n");

//     if (ebda_segment != 0) {
//         cprintf ("find_rsdp-3\n");
//         rsdp = scan_memory((unsigned int *)(ebda_segment << 4), 1024);
//         if (rsdp) return rsdp;
//     }

// cprintf ("find_rsdp-9\n");

//     // 2. Search main BIOS memory area (0x000E0000 - 0x000FFFFF)
//     rsdp = scan_memory((void *) 0x000E0000, 0x20000);
//     return rsdp;
// }






int
mpbcpu (void)
{
    return bcpu-cpus;
}

static uchar
sum (uchar *addr, int len)
{
    int i, sum;
  
    sum = 0;
    for(i=0; i<len; i++)
        sum += addr[i];

    return sum;
}

// Look for an MP structure in the len bytes at addr.
static struct mp *
mpsearch1 (uint a, int len)
{
    uchar *e, *p, *addr;

    addr = p2v (a);
    
    e = addr+len;

    for(p = addr; p < e; p += sizeof(struct mp))
        if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
            return (struct mp *) p;
    
    return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp *
mpsearch (void)
{
    uchar *bda;
    uint p;
    struct mp *mp;

    bda = (uchar *) P2V (0x400);

    if ((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)) {
        
        if((mp = mpsearch1 (p, 1024)))
            return mp;
    } else {
        p = ((bda[0x14]<<8)|bda[0x13])*1024;
        if ((mp = mpsearch1(p-1024, 1024)))
            return mp;
    }
    return mpsearch1 (0xF0000, 0x10000);
}

// void search_madt ()
// {

// rsdp_t *o;

// o = find_rsdp ();
// if (o!=nullptr)
// {
//     cprintf ("found\n");
// }

// }

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mpconf *
mpconfig (struct mp **pmp)
{
    struct mpconf *conf;
    struct mp *mp;

    if( (mp = mpsearch ()) == 0 || mp->physaddr == 0)
        return 0;
    
    conf = (struct mpconf*) p2v((uintp) mp->physaddr);

    if(memcmp(conf, "PCMP", 4) != 0)
        return 0;

    if (conf->version != 1 && conf->version != 4)
        return 0;

    if (sum ((uchar *) conf, conf->length) != 0)
        return 0;

    *pmp = mp;

    return conf;
}

void
mpinit (void)
{
    uchar *p, *e;
    struct mp *mp;
    struct mpconf *conf;
    struct mpproc *proc;
    struct mpioapic *ioapic;

    bcpu = &cpus[0];

    if ((conf = mpconfig (&mp)) == 0)
        return;

    ismp = 1;

    lapic = IO2V ((uintp) conf->lapicaddr);

    //cprintf ("config length %d\n", conf->length);

    for (p= (uchar *)(conf+1), e=(uchar *) conf + conf->length; p<e; ) {

        switch(*p) {

        case MPPROC:
//        cprintf ("MPPROC\n");
            proc = (struct mpproc*)p;
            cprintf("mpinit ncpu=%d apicid=%d\n", ncpu, proc->apicid);
            if(proc->flags & MPBOOT)
                bcpu = &cpus[ncpu];
            cpus[ncpu].id = ncpu;
            cpus[ncpu].apicid = proc->apicid;
            ncpu++;
            p += sizeof(struct mpproc);
            continue;
                
        case MPIOAPIC:
//cprintf ("MPIOAPIC\n");        
            ioapic = (struct mpioapic*)p;
            ioapicid = ioapic->apicno;
            p += sizeof(struct mpioapic);
            continue;

        case MPBUS:
//cprintf ("MPBUS\n");

        case MPIOINTR:
//cprintf ("MPIOINTR\n");

        case MPLINTR:
//cprintf ("MPLINTR\n");

            p += 8;
            continue;

        default:
            cprintf("mpinit: unknown config type %x\n", *p);
            ismp = 0;
        }
    }

    if(!ismp) {
        cprintf("mpinit: fallback\n");
        // Didn't like what we found; fall back to no MP.
        ncpu = 1;
        lapic = 0;
        ioapicid = 0;
        return;
    }

    if(mp->imcrp) {
        // Bochs doesn't support IMCR, so this doesn't run on Bochs.
        // But it would on real hardware.
        outb(0x22, 0x70);   // Select IMCR
        outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
    }
}
