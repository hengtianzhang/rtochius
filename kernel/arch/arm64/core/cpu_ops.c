/*
 * CPU kernel entry/exit control
 *
 * Copyright (C) 2013 ARM Ltd.
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
#include <base/string.h>

#include <rtochius/of.h>

#include <asm/cache.h>
#include <asm/cpu_ops.h>
#include <asm/smp_plat.h>

extern const struct cpu_operations cpu_psci_ops;

const struct cpu_operations *cpu_ops[NR_CPUS] __ro_after_init;

static const struct cpu_operations *const dt_supported_cpu_ops[] __initconst = {
	&cpu_psci_ops,
	NULL,
};

static const struct cpu_operations * __init cpu_get_ops(const char *name)
{
	const struct cpu_operations *const *ops;

	ops = dt_supported_cpu_ops;

	while (*ops) {
		if (!strcmp(name, (*ops)->name))
			return *ops;

		ops++;
	}

	return NULL;
}

static const char *__init cpu_read_enable_method(int cpu)
{
	const char *enable_method;

	struct device_node *dn = of_get_cpu_node(cpu, NULL);

	if (!dn) {
		if (!cpu)
			pr_err("Failed to find device node for boot cpu\n");
		return NULL;
	}

	enable_method = of_get_property(dn, "enable-method", NULL);
	if (!enable_method) {
		/*
			* The boot CPU may not have an enable method (e.g.
			* when spin-table is used for secondaries).
			* Don't warn spuriously.
			*/
		if (cpu != 0)
			pr_err("%pOF: missing enable-method property\n",
				dn);
	}

	return enable_method;
}

/*
 * Read a cpu's enable method and record it in cpu_ops.
 */
int __init cpu_read_ops(int cpu)
{
	const char *enable_method = cpu_read_enable_method(cpu);

	if (!enable_method)
		return -ENODEV;

	cpu_ops[cpu] = cpu_get_ops(enable_method);
	if (!cpu_ops[cpu]) {
		pr_warn("Unsupported enable-method: %s\n", enable_method);
		return -EOPNOTSUPP;
	}

	return 0;
}
