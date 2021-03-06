/*
 *  linux/arch/arm/mm/nommu.c
 *
 * ARM uCLinux supporting functions.
 */
#include <linux/module.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/bootmem.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/sections.h>
#include <asm/page.h>
#include <asm/setup.h>
#include <asm/mpu.h>
#include <asm/mach/arch.h>

#include "mm.h"

/*
 * Reserve the various regions of node 0
 */
void __init reserve_node_zero(pg_data_t *pgdat)
{
	/*
	 * Register the kernel text and data with bootmem.
	 * Note that this can only be in node 0.
	 */
#ifdef CONFIG_XIP_KERNEL
	reserve_bootmem_node(pgdat, __pa(_data), _end - _data,
			BOOTMEM_DEFAULT);
#else
#if defined(PHYS_ALIAS_OFFSET)
	reserve_bootmem_node(pgdat, PHYS_ALIAS_ADDR(_stext), _end - _stext,
			BOOTMEM_DEFAULT);
#else
	reserve_bootmem_node(pgdat, __pa(_stext), _end - _stext,
			BOOTMEM_DEFAULT);
#endif
#endif

#ifndef CONFIG_CPU_V7M
	/*
	 * Register the exception vector page.
	 * some architectures which the DRAM is the exception vector to trap,
	 * alloc_page breaks with error, although it is not NULL, but "0."
	 */
	reserve_bootmem_node(pgdat, CONFIG_VECTORS_BASE, PAGE_SIZE,
			BOOTMEM_DEFAULT);
#endif

#if defined(CONFIG_MTD_UCLINUX) && defined(CONFIG_MTD_UCLINUX_RELOCATE)
{		
	/*
	 * If using a romfs in ram, reserve this memory.
	 */
	unsigned char *p;
	unsigned romfslen;
	unsigned ressiz;
	p = (unsigned char *)_end;
	if (!strncmp(p, "-rom1fs-", 8))
	{
		romfslen = p[8];
		romfslen = (romfslen << 8) + p[9];
		romfslen = (romfslen << 8) + p[10];
		romfslen = (romfslen << 8) + p[11];
		ressiz   = PAGE_ALIGN((unsigned)p + romfslen);
		reserve_bootmem_node(pgdat, __pa(p), ressiz - (unsigned)p,
				BOOTMEM_DEFAULT);
	}//if
}
#endif
}

/*
 * paging_init() sets up the page tables, initialises the zone memory
 * maps, and sets up the zero page, bad page and bad page tables.
 */
void __init paging_init(struct machine_desc *mdesc)
{
	bootmem_init();
#ifdef CONFIG_MPU
	mpu_init();
#endif
}

/*
 * We don't need to do anything here for nommu machines.
 */
void setup_mm_for_reboot(char mode)
{
}

void flush_dcache_page(struct page *page)
{
	__cpuc_flush_dcache_area(page_address(page), PAGE_SIZE);
}
EXPORT_SYMBOL(flush_dcache_page);

void flush_ptrace_access(struct vm_area_struct *vma, struct page *page,
			 unsigned long uaddr, void *kaddr,
			 unsigned long len, int write)
{
	if (vma->vm_flags & VM_EXEC)
		__cpuc_coherent_user_range(uaddr, uaddr + len);
}

void __iomem *__arm_ioremap_pfn(unsigned long pfn, unsigned long offset,
				size_t size, unsigned int mtype)
{
	if (pfn >= (0x100000000ULL >> PAGE_SHIFT))
		return NULL;
	return (void __iomem *) (offset + (pfn << PAGE_SHIFT));
}
EXPORT_SYMBOL(__arm_ioremap_pfn);

void __iomem *__arm_ioremap(unsigned long phys_addr, size_t size,
			    unsigned int mtype)
{
	return (void __iomem *)phys_addr;
}
EXPORT_SYMBOL(__arm_ioremap);

void __iounmap(volatile void __iomem *addr)
{
}
EXPORT_SYMBOL(__iounmap);
