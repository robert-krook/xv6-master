/*
 *  acpi.c - ACPI (Advanced Configuration and Power Interface)
 *  
 *  Is an open industry standard that lets your computer's operating system
 *  directly manage power savings and hardware configuration, replacing
 *  older BIOS-controlled methods. It controls sleep modes,
 *  processor speed, and device power levels.
 */

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mp.h"
#include "x86.h"
#include "mmu.h"
#include "proc.h"
#include "acpi.h"

extern struct cpu cpus[NCPU];
extern int ismp;
extern int ncpu;
extern uchar ioapicid;

static struct acpi_rdsp *
scan_rdsp (uint base, uint len) 
{
    uchar *p;

    for (p = p2v(base); len >= sizeof(struct acpi_rdsp); len -= 4, p += 4) {
        if (memcmp(p, SIG_RDSP, 8) == 0) {
            uint sum, n;
        for (sum = 0, n = 0; n < 20; n++)
            sum += p[n];
        if ((sum & 0xff) == 0)
            return (struct acpi_rdsp *) p;
        }
    }
    return (struct acpi_rdsp *) 0;  
}

static struct acpi_rdsp *
find_rdsp (void) 
{
    struct acpi_rdsp *rdsp;
    uintp pa;

    pa = *((ushort*) P2V(0x40E)) << 4; // EBDA
    if (pa && (rdsp = scan_rdsp(pa, 1024)))
        return rdsp;
    return scan_rdsp(0xE0000, 0x20000);
}

static int acpi_config_smp(struct acpi_madt *madt) 
{
    uint32 lapic_addr;
    uint nioapic = 0;
    uchar *p, *e;

    if (!madt)
        return -1;

    if (madt->header.length < sizeof(struct acpi_madt))
        return -1;

    lapic_addr = madt->lapic_addr_phys;

    p = madt->table;
    e = p + madt->header.length - sizeof(struct acpi_madt);

    while (p < e) {

        uint len;

        if ((e - p) < 2)
            break;

        len = p[1];

        if ((e - p) < len)
            break;

        switch (p[0]) {

            case TYPE_LAPIC: {
                struct madt_lapic *lapic = (void*) p;
                if (len < sizeof(*lapic))
                    break;
                if (!(lapic->flags & APIC_LAPIC_ENABLED))
                    break;
                cprintf("acpi: cpu#%d apicid %d\n", ncpu, lapic->apic_id);
                cpus[ncpu].id = ncpu;
                cpus[ncpu].apicid = lapic->apic_id;
                ncpu++;
                break;
            }

            case TYPE_IOAPIC: {
                struct madt_ioapic *ioapic = (void*) p;
                if (len < sizeof(*ioapic))
                    break;
                cprintf("acpi: ioapic#%d @%x id=%d base=%d\n",
                nioapic,  ioapic->addr, ioapic->id, ioapic->interrupt_base);

                if (nioapic) {
                    cprintf("warning: multiple ioapics are not supported");
                } else {
                    ioapicid = ioapic->id;
                }
                nioapic++;
                break;
            }
        }
        p += len;
    }

     if (ncpu) {
        ismp = 1;
        lapic = IO2V(((uintp)lapic_addr));
        return 0;
    }

    return -1;
}

#if X64
#define PHYSLIMIT 0x80000000
#else
#define PHYSLIMIT 0x0E000000
#endif

int acpiinit(void) 
{
    unsigned n, count;
    struct acpi_rdsp *rdsp;
    struct acpi_rsdt *rsdt;
    struct acpi_madt *madt = 0;

    rdsp = find_rdsp();

    if (rdsp->rsdt_addr_phys > PHYSLIMIT)
        goto notmapped;

    rsdt = p2v(rdsp->rsdt_addr_phys);
    count = (rsdt->header.length - sizeof(*rsdt)) / 4;

    for (n = 0; n < count; n++) {
        struct acpi_desc_header *hdr = p2v(rsdt->entry[n]);
        if (rsdt->entry[n] > PHYSLIMIT)
            goto notmapped;
#if DEBUG
    uchar sig[5], id[7], tableid[9], creator[5];
    memmove(sig, hdr->signature, 4); sig[4] = 0;
    memmove(id, hdr->oem_id, 6); id[6] = 0;
    memmove(tableid, hdr->oem_tableid, 8); tableid[8] = 0;
    memmove(creator, hdr->creator_id, 4); creator[4] = 0;
    cprintf("acpi: %s %s %s %x %s %x\n",
      sig, id, tableid, hdr->oem_revision,
      creator, hdr->creator_revision);
#endif
        if (!memcmp(hdr->signature, SIG_MADT, 4))
            madt = (void*) hdr;
    }

    return acpi_config_smp(madt);

notmapped:
    cprintf("acpi: tables above 0x%x not mapped.\n", PHYSLIMIT);
    return -1;
}
