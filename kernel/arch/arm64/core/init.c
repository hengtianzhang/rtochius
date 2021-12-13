/*
 * Based on arch/arm/mm/init.c
 *
 * Copyright (C) 1995-2005 Russell King
 * Copyright (C) 2012 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <base/types.h>
#include <base/cache.h>

#include <rtochius/of_fdt.h>
#include <rtochius/memory.h>
#include <rtochius/page.h>
#include <rtochius/param.h>
#include <rtochius/mm.h>

#include <asm/kernel-pgtable.h>
#include <asm/sections.h>

struct memblock memblock_kernel;

/*
 * We need to be able to catch inadvertent references to memstart_addr
 * that occur (potentially in generic code) before arm64_memblock_init()
 * executes, which assigns it its actual value. So use a default value
 * that cannot be mistaken for a real physical address.
 */
s64 memstart_addr __ro_after_init = -1;

int pfn_valid(unsigned long pfn)
{
	phys_addr_t addr = pfn << PAGE_SHIFT;

	if ((addr >> PAGE_SHIFT) != pfn)
		return 0;

	return memblock_is_map_memory(&memblock_kernel, addr);
}

static int __init early_init_dt_scan_usablemem(unsigned long node,
		const char *uname, int depth, void *data)
{
	struct memblock_region *usablemem = data;
	const __be32 *reg;
	int len;

	if (depth != 1 || strcmp(uname, "chosen") != 0)
		return 0;

	reg = of_get_flat_dt_prop(node, "linux,usable-memory-range", &len);
	if (!reg || (len < (dt_root_addr_cells + dt_root_size_cells)))
		return 1;

	usablemem->base = dt_mem_next_cell(dt_root_addr_cells, &reg);
	usablemem->size = dt_mem_next_cell(dt_root_size_cells, &reg);

	return 1;
}

static void __init fdt_enforce_memory_region(void)
{
	struct memblock_region reg = {
		.size = 0,
	};

	of_scan_flat_dt(early_init_dt_scan_usablemem, &reg);

	if (reg.size)
		memblock_cap_memory_range(&memblock_kernel, reg.base, reg.size);
}

void __init arm64_memblock_init(void)
{
	const s64 linear_region_size = -(s64)PAGE_OFFSET;

	/* Handle linux,usable-memory-range property */
	fdt_enforce_memory_region();

	/* Remove memory above our supported physical address size */
	memblock_remove(&memblock_kernel, 1ULL << PHYS_MASK_SHIFT, ULLONG_MAX);

	/*
	 * Ensure that the linear region takes up exactly half of the kernel
	 * virtual address space. This way, we can distinguish a linear address
	 * from a kernel/module/vmalloc address by testing a single bit.
	 */
	BUILD_BUG_ON(linear_region_size != BIT(VA_BITS - 1));

	/*
	 * Select a suitable value for the base of physical memory.
	 */
	memstart_addr = round_down(memblock_start_of_DRAM(&memblock_kernel),
				   ARM64_MEMSTART_ALIGN);

	/*
	 * Remove the memory that we will not be able to cover with the
	 * linear mapping. Take care not to clip the kernel which may be
	 * high in memory.
	 */
	memblock_remove(&memblock_kernel, max_t(u64, memstart_addr + linear_region_size,
			__pa_symbol(_end)), ULLONG_MAX);
	if (memstart_addr + linear_region_size < memblock_end_of_DRAM(&memblock_kernel)) {
		/* ensure that memstart_addr remains sufficiently aligned */
		memstart_addr = round_up(memblock_end_of_DRAM(&memblock_kernel) - linear_region_size,
					 ARM64_MEMSTART_ALIGN);
		memblock_remove(&memblock_kernel, 0, memstart_addr);
	}

	if (phys_initrd_size) {
		/*
		 * Add back the memory we just removed if it results in the
		 * initrd to become inaccessible via the linear mapping.
		 * Otherwise, this is a no-op
		 */
		u64 base = phys_initrd_start & PAGE_MASK;
		u64 size = PAGE_ALIGN(phys_initrd_size);

		/*
		 * We can only add back the initrd memory if we don't end up
		 * with more memory than we can address via the linear mapping.
		 * It is up to the bootloader to position the kernel and the
		 * initrd reasonably close to each other (i.e., within 32 GB of
		 * each other) so that all granule/#levels combinations can
		 * always access both.
		 */
		if (WARN(base < memblock_start_of_DRAM(&memblock_kernel) ||
			 base + size > memblock_start_of_DRAM(&memblock_kernel) +
				       linear_region_size,
			"initrd not fully accessible via the linear mapping -- please check your bootloader ...\n")) {
				;
		} else {
			memblock_remove(&memblock_kernel, base, size); /* clear MEMBLOCK_ flags */
			memblock_add(&memblock_kernel, base, size);
			memblock_reserve(&memblock_kernel, base, size);
		}
	}

	/*
	 * Register the kernel text, kernel data, initrd, and initial
	 * pagetables with memblock.
	 */
	memblock_reserve(&memblock_kernel, __pa_symbol(_text), _end - _text);

	early_init_fdt_scan_reserved_mem();
}

static void __init vmemmap_init(void)
{
	int i;
	size_t size;
	phys_addr_t phys_addr, start_pfn, end_pfn;

	for_each_mem_pfn_range(&memblock_kernel, i, &start_pfn, &end_pfn) {
		size = round_up(((end_pfn - start_pfn) * sizeof (struct page)), PAGE_SIZE);
		phys_addr = memblock_alloc(&memblock_kernel, size, PAGE_SIZE);

		vmemmap_populate(phys_addr, (unsigned long)pfn_to_page(start_pfn), size);
		memset(pfn_to_page(start_pfn), 0, size);
	}
}

static int __init early_memblock(char *p)
{
	if (p && strstr(p, "debug"))
		memblock_debug_enable();
	return 0;
}
early_param("memblock", early_memblock);

void __init bootmem_init(void)
{
	unsigned long min, max;

	min = PFN_UP(memblock_start_of_DRAM(&memblock_kernel));
	max = PFN_DOWN(memblock_end_of_DRAM(&memblock_kernel));

	early_memtest(min << PAGE_SHIFT, max << PAGE_SHIFT);

	vmemmap_init();

	memblock_dump_all(&memblock_kernel);
}

void free_initmem(void)
{
	free_reserved_area(lm_alias(__init_begin),
			   lm_alias(__init_end),
			   0, "unused kernel");
	/*
	 * Unmap the __init region but leave the VM area in place. This
	 * prevents the region from being reused for kernel modules, which
	 * is not supported by kallsyms.
	 */
	unmap_kernel_range((u64)__init_begin, (u64)(__init_end - __init_begin));
}
