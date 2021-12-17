// SPDX-License-Identifier: GPL-2.0

#define pr_fmt(fmt)  "irqdomain: " fmt

#include <rtochius/spinlock.h>
#include <rtochius/of.h>
#include <rtochius/of_address.h>
#include <rtochius/mutex.h>
#include <rtochius/irqdomain.h>
#include <rtochius/slab.h>
#include <rtochius/printf.h>

struct irqchip_fwid {
	struct fwnode_handle	fwnode;
	unsigned int		type;
	char			*name;
	void *data;
};

static LIST_HEAD(irq_domain_list);
static DEFINE_MUTEX(irq_domain_mutex);

static struct irq_domain *irq_default_domain;

static void irq_domain_check_hierarchy(struct irq_domain *domain)
{
	/* Hierarchy irq_domains must implement callback alloc() */
	if (domain->ops->alloc)
		domain->flags |= IRQ_DOMAIN_FLAG_HIERARCHY;
}

const struct fwnode_operations irqchip_fwnode_ops;

/**
 * irq_domain_alloc_fwnode - Allocate a fwnode_handle suitable for
 *                           identifying an irq domain
 * @type:	Type of irqchip_fwnode. See linux/irqdomain.h
 * @name:	Optional user provided domain name
 * @id:		Optional user provided id if name != NULL
 * @data:	Optional user-provided data
 *
 * Allocate a struct irqchip_fwid, and return a poiner to the embedded
 * fwnode_handle (or NULL on failure).
 *
 * Note: The types IRQCHIP_FWNODE_NAMED and IRQCHIP_FWNODE_NAMED_ID are
 * solely to transport name information to irqdomain creation code. The
 * node is not stored. For other types the pointer is kept in the irq
 * domain struct.
 */
struct fwnode_handle *__irq_domain_alloc_fwnode(unsigned int type, int id,
							const char *name, void *data)
{
	struct irqchip_fwid *fwid;
	char *n;

	fwid = kzalloc(sizeof (*fwid), GFP_KERNEL);
	if (!fwid)
		goto out;

	switch (type) {
	case IRQCHIP_FWNODE_NAMED:
		n = kasprintf(GFP_KERNEL, "%s", name);
		break;
	case IRQCHIP_FWNODE_NAMED_ID:
		n = kasprintf(GFP_KERNEL, "%s-%d", name, id);
		break;
	default:
		n = kasprintf(GFP_KERNEL, "irqchip@%p", data);
		break;
	}

	if (!n)
		goto free_fwid;

	fwid->type = type;
	fwid->name = n;
	fwid->data = data;
	fwid->fwnode.ops = &irqchip_fwnode_ops;

	return &fwid->fwnode;

free_fwid:
	kfree(fwid);
out:
	return NULL;
}

/**
 * irq_domain_free_fwnode - Free a non-OF-backed fwnode_handle
 *
 * Free a fwnode_handle allocated with irq_domain_alloc_fwnode.
 */
void irq_domain_free_fwnode(struct fwnode_handle *fwnode)
{
	struct irqchip_fwid *fwid;

	if (WARN_ON(!is_fwnode_irqchip(fwnode)))
		return;

	fwid = container_of(fwnode, struct irqchip_fwid, fwnode);
	kfree(fwid->name);
	kfree(fwid);
}

/**
 * __irq_domain_add() - Allocate a new irq_domain data structure
 * @fwnode: firmware node for the interrupt controller
 * @hwirq_max: Maximum number of interrupts supported by controller
 * @ops: domain callbacks
 * @host_data: Controller private data pointer
 *
 * Allocates and initialize and irq_domain structure.
 * Returns pointer to IRQ domain, or NULL on failure.
 */
struct irq_domain *__irq_domain_add(struct fwnode_handle *fwnode,
					irq_hw_number_t hwirq_max,
					const struct irq_domain_ops *ops,
					void *host_data)
{
	struct device_node *of_node = to_of_node(fwnode);
	struct irqchip_fwid *fwid;
	struct irq_domain *domain;

	static atomic_t unknown_domains;

	domain = kzalloc(sizeof (*domain), GFP_KERNEL);
	if (WARN_ON(!domain))
		return NULL;

	if (fwnode && is_fwnode_irqchip(fwnode)) {
		fwid = container_of(fwnode, struct irqchip_fwid, fwnode);

		switch (fwid->type) {
		case IRQCHIP_FWNODE_NAMED:
		case IRQCHIP_FWNODE_NAMED_ID:
			domain->name = kstrdup(fwid->name, GFP_KERNEL);
			if (!domain->name) {
				kfree(domain);
				return NULL;
			}
			domain->flags |= IRQ_DOMAIN_NAME_ALLOCATED;
			break;
		default:
			domain->fwnode = fwnode;
			domain->name = fwid->name;
			break;
		}
	} else if (of_node) {
		char *name;

		/*
		 * DT paths contain '/', which debugfs is legitimately
		 * unhappy about. Replace them with ':', which does
		 * the trick and is not as offensive as '\'...
		 */
		name = kasprintf(GFP_KERNEL, "%s", of_node->full_name);
		if (!name) {
			kfree(domain);
			return NULL;
		}

		strreplace(name, '/', ':');

		domain->name = name;
		domain->fwnode = fwnode;
		domain->flags |= IRQ_DOMAIN_NAME_ALLOCATED;
	}

	if (!domain->name) {
		if (fwnode)
			pr_err("Invalid fwnode type for irqdomain\n");
		domain->name = kasprintf(GFP_KERNEL, "unknown-%d",
					atomic_inc_return(&unknown_domains));
		if (!domain->name) {
			kfree(domain);
			return NULL;
		}
		domain->flags |= IRQ_DOMAIN_NAME_ALLOCATED;
	}

	of_node_get(of_node);
	/* Fill structure */
	INIT_RADIX_TREE(&domain->revmap_tree, GFP_KERNEL);
	mutex_init(&domain->revmap_tree_mutex);
	domain->ops = ops;
	domain->host_data = host_data;
	domain->hwirq_max = hwirq_max;
	irq_domain_check_hierarchy(domain);

	mutex_lock(&irq_domain_mutex);
	list_add(&domain->link, &irq_domain_list);
	mutex_unlock(&irq_domain_mutex);

	pr_debug("Added domain %s\n", domain->name);
	return domain;
}

/**
 * irq_domain_remove() - Remove an irq domain.
 * @domain: domain to remove
 *
 * This routine is used to remove an irq domain. The caller must ensure
 * that all mappings within the domain have been disposed of prior to
 * use, depending on the revmap type.
 */
void irq_domain_remove(struct irq_domain *domain)
{
	mutex_lock(&irq_domain_mutex);

	WARN_ON(!radix_tree_empty(&domain->revmap_tree));

	list_del(&domain->link);

	/*
	 * If the going away domain is the default one, reset it.
	 */
	if (unlikely(irq_default_domain == domain))
		irq_set_default_host(NULL);

	mutex_unlock(&irq_domain_mutex);

	pr_debug("Removed domain %s\n", domain->name);

	of_node_put(irq_domain_get_of_node(domain));
	if (domain->flags & IRQ_DOMAIN_NAME_ALLOCATED)
		kfree(domain->name);
	kfree(domain);
}

void irq_domain_update_bus_token(struct irq_domain *domain,
				 enum irq_domain_bus_token bus_token)
{
	char *name;

	if (domain->bus_token == bus_token)
		return;

	mutex_lock(&irq_domain_mutex);

	domain->bus_token = bus_token;

	name = kasprintf(GFP_KERNEL, "%s-%d", domain->name, bus_token);
	if (!name) {
		mutex_unlock(&irq_domain_mutex);
		return;
	}

	if (domain->flags & IRQ_DOMAIN_NAME_ALLOCATED)
		kfree(domain->name);
	else
		domain->flags |= IRQ_DOMAIN_NAME_ALLOCATED;

	domain->name = name;

	mutex_unlock(&irq_domain_mutex);
}

/**
 * irq_find_matching_fwspec() - Locates a domain for a given fwspec
 * @fwspec: FW specifier for an interrupt
 * @bus_token: domain-specific data
 */
struct irq_domain *irq_find_matching_fwspec(struct irq_fwspec *fwspec,
					    enum irq_domain_bus_token bus_token)
{
	struct irq_domain *h, *found = NULL;
	struct fwnode_handle *fwnode = fwspec->fwnode;
	int rc;

	/* We might want to match the legacy controller last since
	 * it might potentially be set to match all interrupts in
	 * the absence of a device node. This isn't a problem so far
	 * yet though...
	 *
	 * bus_token == DOMAIN_BUS_ANY matches any domain, any other
	 * values must generate an exact match for the domain to be
	 * selected.
	 */
	mutex_lock(&irq_domain_mutex);
	list_for_each_entry(h, &irq_domain_list, link) {
		if (h->ops->select && fwspec->param_count)
			rc = h->ops->select(h, fwspec, bus_token);
		else if (h->ops->match)
			rc = h->ops->match(h, to_of_node(fwnode), bus_token);
		else
			rc = ((fwnode != NULL) && (h->fwnode == fwnode) &&
			      ((bus_token == DOMAIN_BUS_ANY) ||
			       (h->bus_token == bus_token)));

		if (rc) {
			found = h;
			break;
		}
	}
	mutex_unlock(&irq_domain_mutex);

	return found;
}

/**
 * irq_set_default_host() - Set a "default" irq domain
 * @domain: default domain pointer
 *
 * For convenience, it's possible to set a "default" domain that will be used
 * whenever NULL is passed to irq_create_mapping(). It makes life easier for
 * platforms that want to manipulate a few hard coded interrupt numbers that
 * aren't properly represented in the device-tree.
 */
void irq_set_default_host(struct irq_domain *domain)
{
	pr_debug("Default domain set to @0x%p\n", domain);

	irq_default_domain = domain;
}

static void irq_domain_clear_mapping(struct irq_domain *domain,
				     irq_hw_number_t hwirq)
{
	mutex_lock(&domain->revmap_tree_mutex);
	radix_tree_delete(&domain->revmap_tree, hwirq);
	mutex_unlock(&domain->revmap_tree_mutex);
}

static void irq_domain_set_mapping(struct irq_domain *domain,
				   irq_hw_number_t hwirq,
				   struct irq_data *irq_data)
{
	mutex_lock(&domain->revmap_tree_mutex);
	radix_tree_insert(&domain->revmap_tree, hwirq, irq_data);
	mutex_unlock(&domain->revmap_tree_mutex);
}

void irq_domain_disassociate(struct irq_domain *domain, unsigned int irq)
{
	struct irq_data *irq_data = irq_get_irq_data(irq);
	irq_hw_number_t hwirq;

	if (WARN(!irq_data || irq_data->domain != domain,
		 "virq%i doesn't exist; cannot disassociate\n", irq))
		return;

	hwirq = irq_data->hwirq;
	irq_set_status_flags(irq, IRQ_NOREQUEST);

	/* remove chip and handler */
	irq_set_chip_and_handler(irq, NULL, NULL);

	/* Make sure it's completed */
	synchronize_irq(irq);

	/* Tell the PIC about it */
	if (domain->ops->unmap)
		domain->ops->unmap(domain, irq);
	smp_mb();

	irq_data->domain = NULL;
	irq_data->hwirq = 0;
	domain->mapcount--;

	/* Clear reverse map for this hwirq */
	irq_domain_clear_mapping(domain, hwirq);
}

int irq_domain_associate(struct irq_domain *domain, unsigned int virq,
			 irq_hw_number_t hwirq)
{
	struct irq_data *irq_data = irq_get_irq_data(virq);
	int ret;

	if (WARN(hwirq >= domain->hwirq_max,
		 "error: hwirq 0x%x is too large for %s\n", (int)hwirq, domain->name))
		return -EINVAL;
	if (WARN(!irq_data, "error: virq%i is not allocated", virq))
		return -EINVAL;
	if (WARN(irq_data->domain, "error: virq%i is already associated", virq))
		return -EINVAL;

	mutex_lock(&irq_domain_mutex);
	irq_data->hwirq = hwirq;
	irq_data->domain = domain;
	if (domain->ops->map) {
		ret = domain->ops->map(domain, virq, hwirq);
		if (ret != 0) {
			/*
			 * If map() returns -EPERM, this interrupt is protected
			 * by the firmware or some other service and shall not
			 * be mapped. Don't bother telling the user about it.
			 */
			if (ret != -EPERM) {
				pr_info("%s didn't like hwirq-0x%lx to VIRQ%i mapping (rc=%d)\n",
				       domain->name, hwirq, virq, ret);
			}
			irq_data->domain = NULL;
			irq_data->hwirq = 0;
			mutex_unlock(&irq_domain_mutex);
			return ret;
		}

		/* If not already assigned, give the domain the chip's name */
		if (!domain->name && irq_data->chip)
			domain->name = irq_data->chip->name;
	}

	domain->mapcount++;
	irq_domain_set_mapping(domain, hwirq, irq_data);
	mutex_unlock(&irq_domain_mutex);

	irq_clear_status_flags(virq, IRQ_NOREQUEST);

	return 0;
}

void irq_domain_associate_many(struct irq_domain *domain, unsigned int irq_base,
			       irq_hw_number_t hwirq_base, int count)
{
	struct device_node *of_node;
	int i;

	of_node = irq_domain_get_of_node(domain);
	pr_debug("%s(%s, irqbase=%i, hwbase=%i, count=%i)\n", __func__,
		of_node_full_name(of_node), irq_base, (int)hwirq_base, count);

	for (i = 0; i < count; i++) {
		irq_domain_associate(domain, irq_base + i, hwirq_base + i);
	}
}

/**
 * irq_find_mapping() - Find a linux irq from a hw irq number.
 * @domain: domain owning this hardware interrupt
 * @hwirq: hardware irq number in that domain space
 */
unsigned int irq_find_mapping(struct irq_domain *domain,
			      irq_hw_number_t hwirq)
{
	struct irq_data *data;

	/* Look for default domain if nececssary */
	if (domain == NULL)
		domain = irq_default_domain;
	if (domain == NULL)
		return 0;

	mutex_lock(&domain->revmap_tree_mutex);
	data = radix_tree_lookup(&domain->revmap_tree, hwirq);
	mutex_unlock(&domain->revmap_tree_mutex);

	return data ? data->irq : 0;
}

/**
 * irq_create_mapping() - Map a hardware interrupt into linux irq space
 * @domain: domain owning this hardware interrupt or NULL for default domain
 * @hwirq: hardware irq number in that domain space
 *
 * Only one mapping per hardware interrupt is permitted. Returns a linux
 * irq number.
 * If the sense/trigger is to be specified, set_irq_type() should be called
 * on the number returned from that call.
 */
unsigned int irq_create_mapping(struct irq_domain *domain,
				irq_hw_number_t hwirq)
{
	struct device_node *of_node;
	int virq;

	pr_debug("irq_create_mapping(0x%p, 0x%lx)\n", domain, hwirq);

	/* Look for default domain if nececssary */
	if (domain == NULL)
		domain = irq_default_domain;
	if (domain == NULL) {
		WARN(1, "%s(, %lx) called with NULL domain\n", __func__, hwirq);
		return 0;
	}
	pr_debug("-> using domain @%p\n", domain);

	of_node = irq_domain_get_of_node(domain);

	/* Check if mapping already exists */
	virq = irq_find_mapping(domain, hwirq);
	if (virq) {
		pr_debug("-> existing mapping on virq %d\n", virq);
		return virq;
	}

	/* Allocate a virtual interrupt number */
	virq = irq_domain_alloc_descs(-1, 1, hwirq, NULL);
	if (virq <= 0) {
		pr_debug("-> virq allocation failed\n");
		return 0;
	}

	if (irq_domain_associate(domain, virq, hwirq)) {
		irq_free_desc(virq);
		return 0;
	}

	pr_debug("irq %lu on domain %s mapped to virtual irq %u\n",
		hwirq, of_node_full_name(of_node), virq);

	return virq;
}

/**
 * irq_create_strict_mappings() - Map a range of hw irqs to fixed linux irqs
 * @domain: domain owning the interrupt range
 * @irq_base: beginning of linux IRQ range
 * @hwirq_base: beginning of hardware IRQ range
 * @count: Number of interrupts to map
 *
 * This routine is used for allocating and mapping a range of hardware
 * irqs to linux irqs where the linux irq numbers are at pre-defined
 * locations. For use by controllers that already have static mappings
 * to insert in to the domain.
 *
 * Non-linear users can use irq_create_identity_mapping() for IRQ-at-a-time
 * domain insertion.
 *
 * 0 is returned upon success, while any failure to establish a static
 * mapping is treated as an error.
 */
int irq_create_strict_mappings(struct irq_domain *domain, unsigned int irq_base,
			       irq_hw_number_t hwirq_base, int count)
{
	struct device_node *of_node;
	int ret;

	of_node = irq_domain_get_of_node(domain);
	ret = irq_alloc_descs(irq_base, irq_base, count);
	if (unlikely(ret < 0))
		return ret;

	irq_domain_associate_many(domain, irq_base, hwirq_base, count);
	return 0;
}

static int irq_domain_translate(struct irq_domain *d,
				struct irq_fwspec *fwspec,
				irq_hw_number_t *hwirq, unsigned int *type)
{
	if (d->ops->translate)
		return d->ops->translate(d, fwspec, hwirq, type);

	if (d->ops->xlate)
		return d->ops->xlate(d, to_of_node(fwspec->fwnode),
				     fwspec->param, fwspec->param_count,
				     hwirq, type);

	/* If domain has no translation, then we assume interrupt line */
	*hwirq = fwspec->param[0];
	return 0;
}

static void of_phandle_args_to_fwspec(struct of_phandle_args *irq_data,
				      struct irq_fwspec *fwspec)
{
	int i;

	fwspec->fwnode = irq_data->np ? &irq_data->np->fwnode : NULL;
	fwspec->param_count = irq_data->args_count;

	for (i = 0; i < irq_data->args_count; i++)
		fwspec->param[i] = irq_data->args[i];
}

unsigned int irq_create_fwspec_mapping(struct irq_fwspec *fwspec)
{
	struct irq_domain *domain;
	struct irq_data *irq_data;
	irq_hw_number_t hwirq;
	unsigned int type = IRQ_TYPE_NONE;
	int virq;

	if (fwspec->fwnode) {
		domain = irq_find_matching_fwspec(fwspec, DOMAIN_BUS_WIRED);
		if (!domain)
			domain = irq_find_matching_fwspec(fwspec, DOMAIN_BUS_ANY);
	} else {
		domain = irq_default_domain;
	}

	if (!domain) {
		pr_warn("no irq domain found for %s !\n",
			of_node_full_name(to_of_node(fwspec->fwnode)));
		return 0;
	}

	if (irq_domain_translate(domain, fwspec, &hwirq, &type))
		return 0;

	/*
	 * WARN if the irqchip returns a type with bits
	 * outside the sense mask set and clear these bits.
	 */
	if (WARN_ON(type & ~IRQ_TYPE_SENSE_MASK))
		type &= IRQ_TYPE_SENSE_MASK;

	/*
	 * If we've already configured this interrupt,
	 * don't do it again, or hell will break loose.
	 */
	virq = irq_find_mapping(domain, hwirq);
	if (virq) {
		/*
		 * If the trigger type is not specified or matches the
		 * current trigger type then we are done so return the
		 * interrupt number.
		 */
		if (type == IRQ_TYPE_NONE || type == irq_get_trigger_type(virq))
			return virq;

		/*
		 * If the trigger type has not been set yet, then set
		 * it now and return the interrupt number.
		 */
		if (irq_get_trigger_type(virq) == IRQ_TYPE_NONE) {
			irq_data = irq_get_irq_data(virq);
			if (!irq_data)
				return 0;

			irqd_set_trigger_type(irq_data, type);
			return virq;
		}

		pr_warn("type mismatch, failed to map hwirq-%lu for %s!\n",
			hwirq, of_node_full_name(to_of_node(fwspec->fwnode)));
		return 0;
	}

	if (irq_domain_is_hierarchy(domain)) {
		virq = irq_domain_alloc_irqs(domain, 1, fwspec);
		if (virq <= 0)
			return 0;
	} else {
		/* Create mapping */
		virq = irq_create_mapping(domain, hwirq);
		if (!virq)
			return virq;
	}

	irq_data = irq_get_irq_data(virq);
	if (!irq_data) {
		if (irq_domain_is_hierarchy(domain))
			irq_domain_free_irqs(virq, 1);
		else
			irq_dispose_mapping(virq);
		return 0;
	}

	/* Store trigger type */
	irqd_set_trigger_type(irq_data, type);

	return virq;
}

unsigned int irq_create_of_mapping(struct of_phandle_args *irq_data)
{
	struct irq_fwspec fwspec;

	of_phandle_args_to_fwspec(irq_data, &fwspec);
	return irq_create_fwspec_mapping(&fwspec);
}

/**
 * irq_dispose_mapping() - Unmap an interrupt
 * @virq: linux irq number of the interrupt to unmap
 */
void irq_dispose_mapping(unsigned int virq)
{
	struct irq_data *irq_data = irq_get_irq_data(virq);
	struct irq_domain *domain;

	if (!virq || !irq_data)
		return;

	domain = irq_data->domain;
	if (WARN_ON(domain == NULL))
		return;

	if (irq_domain_is_hierarchy(domain)) {
		irq_domain_free_irqs(virq, 1);
	} else {
		irq_domain_disassociate(domain, virq);
		irq_free_desc(virq);
	}
}

/**
 * irq_domain_xlate_onecell() - Generic xlate for direct one cell bindings
 *
 * Device Tree IRQ specifier translation function which works with one cell
 * bindings where the cell value maps directly to the hwirq number.
 */
int irq_domain_xlate_onecell(struct irq_domain *d, struct device_node *ctrlr,
			     const u32 *intspec, unsigned int intsize,
			     unsigned long *out_hwirq, unsigned int *out_type)
{
	if (WARN_ON(intsize < 1))
		return -EINVAL;
	*out_hwirq = intspec[0];
	*out_type = IRQ_TYPE_NONE;
	return 0;
}

/**
 * irq_domain_xlate_twocell() - Generic xlate for direct two cell bindings
 *
 * Device Tree IRQ specifier translation function which works with two cell
 * bindings where the cell values map directly to the hwirq number
 * and linux irq flags.
 */
int irq_domain_xlate_twocell(struct irq_domain *d, struct device_node *ctrlr,
			const u32 *intspec, unsigned int intsize,
			irq_hw_number_t *out_hwirq, unsigned int *out_type)
{
	if (WARN_ON(intsize < 2))
		return -EINVAL;
	*out_hwirq = intspec[0];
	*out_type = intspec[1] & IRQ_TYPE_SENSE_MASK;
	return 0;
}

/**
 * irq_domain_xlate_onetwocell() - Generic xlate for one or two cell bindings
 *
 * Device Tree IRQ specifier translation function which works with either one
 * or two cell bindings where the cell values map directly to the hwirq number
 * and linux irq flags.
 *
 * Note: don't use this function unless your interrupt controller explicitly
 * supports both one and two cell bindings.  For the majority of controllers
 * the _onecell() or _twocell() variants above should be used.
 */
int irq_domain_xlate_onetwocell(struct irq_domain *d,
				struct device_node *ctrlr,
				const u32 *intspec, unsigned int intsize,
				unsigned long *out_hwirq, unsigned int *out_type)
{
	if (WARN_ON(intsize < 1))
		return -EINVAL;
	*out_hwirq = intspec[0];
	if (intsize > 1)
		*out_type = intspec[1] & IRQ_TYPE_SENSE_MASK;
	else
		*out_type = IRQ_TYPE_NONE;
	return 0;
}

const struct irq_domain_ops irq_domain_simple_ops = {
	.xlate = irq_domain_xlate_onetwocell,
};

int irq_domain_alloc_descs(int virq, unsigned int cnt, irq_hw_number_t hwirq,
			   const struct irq_affinity_desc *affinity)
{
	unsigned int hint;

	if (virq >= 0) {
		virq = __irq_alloc_descs(virq, virq, cnt, affinity);
	} else {
		hint = hwirq % nr_irqs;
		if (hint == 0)
			hint++;
		virq = __irq_alloc_descs(-1, hint, cnt, affinity);
		if (virq <= 0 && hint > 1) {
			virq = __irq_alloc_descs(-1, 1, cnt, affinity);
		}
	}

	return virq;
}

/**
 * irq_domain_create_hierarchy - Add a irqdomain into the hierarchy
 * @parent:	Parent irq domain to associate with the new domain
 * @flags:	Irq domain flags associated to the domain
 * @size:	Size of the domain. See below
 * @fwnode:	Optional fwnode of the interrupt controller
 * @ops:	Pointer to the interrupt domain callbacks
 * @host_data:	Controller private data pointer
 *
 * If @size is 0 a tree domain is created, otherwise a linear domain.
 *
 * If successful the parent is associated to the new domain and the
 * domain flags are set.
 * Returns pointer to IRQ domain, or NULL on failure.
 */
struct irq_domain *irq_domain_create_hierarchy(struct irq_domain *parent,
					    unsigned int flags,
					    struct fwnode_handle *fwnode,
					    const struct irq_domain_ops *ops,
					    void *host_data)
{
	struct irq_domain *domain;

	domain = irq_domain_create_tree(fwnode, ops, host_data);
	if (domain) {
		domain->parent = parent;
		domain->flags |= flags;
	}

	return domain;
}

static void irq_domain_insert_irq(int virq)
{
	struct irq_data *data;

	for (data = irq_get_irq_data(virq); data; data = data->parent_data) {
		struct irq_domain *domain = data->domain;

		domain->mapcount++;
		irq_domain_set_mapping(domain, data->hwirq, data);

		/* If not already assigned, give the domain the chip's name */
		if (!domain->name && data->chip)
			domain->name = data->chip->name;
	}

	irq_clear_status_flags(virq, IRQ_NOREQUEST);
}

static void irq_domain_remove_irq(int virq)
{
	struct irq_data *data;

	irq_set_status_flags(virq, IRQ_NOREQUEST);
	irq_set_chip_and_handler(virq, NULL, NULL);
	synchronize_irq(virq);
	smp_mb();

	for (data = irq_get_irq_data(virq); data; data = data->parent_data) {
		struct irq_domain *domain = data->domain;
		irq_hw_number_t hwirq = data->hwirq;

		domain->mapcount--;
		irq_domain_clear_mapping(domain, hwirq);
	}
}

static struct irq_data *irq_domain_insert_irq_data(struct irq_domain *domain,
						   struct irq_data *child)
{
	struct irq_data *irq_data;

	irq_data = kzalloc(sizeof(*irq_data), GFP_KERNEL);
	if (irq_data) {
		child->parent_data = irq_data;
		irq_data->irq = child->irq;
		irq_data->common = child->common;
		irq_data->domain = domain;
	}

	return irq_data;
}

static void irq_domain_free_irq_data(unsigned int virq, unsigned int nr_irqs)
{
	struct irq_data *irq_data, *tmp;
	int i;

	for (i = 0; i < nr_irqs; i++) {
		irq_data = irq_get_irq_data(virq + i);
		tmp = irq_data->parent_data;
		irq_data->parent_data = NULL;
		irq_data->domain = NULL;

		while (tmp) {
			irq_data = tmp;
			tmp = tmp->parent_data;
			kfree(irq_data);
		}
	}
}

static int irq_domain_alloc_irq_data(struct irq_domain *domain,
				     unsigned int virq, unsigned int nr_irqs)
{
	struct irq_data *irq_data;
	struct irq_domain *parent;
	int i;

	/* The outermost irq_data is embedded in struct irq_desc */
	for (i = 0; i < nr_irqs; i++) {
		irq_data = irq_get_irq_data(virq + i);
		irq_data->domain = domain;

		for (parent = domain->parent; parent; parent = parent->parent) {
			irq_data = irq_domain_insert_irq_data(parent, irq_data);
			if (!irq_data) {
				irq_domain_free_irq_data(virq, i + 1);
				return -ENOMEM;
			}
		}
	}

	return 0;
}

/**
 * irq_domain_get_irq_data - Get irq_data associated with @virq and @domain
 * @domain:	domain to match
 * @virq:	IRQ number to get irq_data
 */
struct irq_data *irq_domain_get_irq_data(struct irq_domain *domain,
					 unsigned int virq)
{
	struct irq_data *irq_data;

	for (irq_data = irq_get_irq_data(virq); irq_data;
	     irq_data = irq_data->parent_data)
		if (irq_data->domain == domain)
			return irq_data;

	return NULL;
}

/**
 * irq_domain_set_hwirq_and_chip - Set hwirq and irqchip of @virq at @domain
 * @domain:	Interrupt domain to match
 * @virq:	IRQ number
 * @hwirq:	The hwirq number
 * @chip:	The associated interrupt chip
 * @chip_data:	The associated chip data
 */
int irq_domain_set_hwirq_and_chip(struct irq_domain *domain, unsigned int virq,
				  irq_hw_number_t hwirq, struct irq_chip *chip,
				  void *chip_data)
{
	struct irq_data *irq_data = irq_domain_get_irq_data(domain, virq);

	if (!irq_data)
		return -ENOENT;

	irq_data->hwirq = hwirq;
	irq_data->chip = chip ? chip : &no_irq_chip;
	irq_data->chip_data = chip_data;

	return 0;
}

/**
 * irq_domain_set_info - Set the complete data for a @virq in @domain
 * @domain:		Interrupt domain to match
 * @virq:		IRQ number
 * @hwirq:		The hardware interrupt number
 * @chip:		The associated interrupt chip
 * @chip_data:		The associated interrupt chip data
 * @handler:		The interrupt flow handler
 * @handler_data:	The interrupt flow handler data
 * @handler_name:	The interrupt handler name
 */
void irq_domain_set_info(struct irq_domain *domain, unsigned int virq,
			 irq_hw_number_t hwirq, struct irq_chip *chip,
			 void *chip_data, irq_flow_handler_t handler,
			 void *handler_data, const char *handler_name)
{
	irq_domain_set_hwirq_and_chip(domain, virq, hwirq, chip, chip_data);
	__irq_set_handler(virq, handler, 0, handler_name);
	irq_set_handler_data(virq, handler_data);
}

/**
 * irq_domain_reset_irq_data - Clear hwirq, chip and chip_data in @irq_data
 * @irq_data:	The pointer to irq_data
 */
void irq_domain_reset_irq_data(struct irq_data *irq_data)
{
	irq_data->hwirq = 0;
	irq_data->chip = &no_irq_chip;
	irq_data->chip_data = NULL;
}

/**
 * irq_domain_free_irqs_common - Clear irq_data and free the parent
 * @domain:	Interrupt domain to match
 * @virq:	IRQ number to start with
 * @nr_irqs:	The number of irqs to free
 */
void irq_domain_free_irqs_common(struct irq_domain *domain, unsigned int virq,
				 unsigned int nr_irqs)
{
	struct irq_data *irq_data;
	int i;

	for (i = 0; i < nr_irqs; i++) {
		irq_data = irq_domain_get_irq_data(domain, virq + i);
		if (irq_data)
			irq_domain_reset_irq_data(irq_data);
	}
	irq_domain_free_irqs_parent(domain, virq, nr_irqs);
}

/**
 * irq_domain_free_irqs_top - Clear handler and handler data, clear irqdata and free parent
 * @domain:	Interrupt domain to match
 * @virq:	IRQ number to start with
 * @nr_irqs:	The number of irqs to free
 */
void irq_domain_free_irqs_top(struct irq_domain *domain, unsigned int virq,
			      unsigned int nr_irqs)
{
	int i;

	for (i = 0; i < nr_irqs; i++) {
		irq_set_handler_data(virq + i, NULL);
		irq_set_handler(virq + i, NULL);
	}
	irq_domain_free_irqs_common(domain, virq, nr_irqs);
}

static void irq_domain_free_irqs_hierarchy(struct irq_domain *domain,
					   unsigned int irq_base,
					   unsigned int nr_irqs)
{
	if (domain->ops->free)
		domain->ops->free(domain, irq_base, nr_irqs);
}

int irq_domain_alloc_irqs_hierarchy(struct irq_domain *domain,
				    unsigned int irq_base,
				    unsigned int nr_irqs, void *arg)
{
	return domain->ops->alloc(domain, irq_base, nr_irqs, arg);
}

/**
 * __irq_domain_alloc_irqs - Allocate IRQs from domain
 * @domain:	domain to allocate from
 * @irq_base:	allocate specified IRQ nubmer if irq_base >= 0
 * @nr_irqs:	number of IRQs to allocate
 * @node:	NUMA node id for memory allocation
 * @arg:	domain specific argument
 * @realloc:	IRQ descriptors have already been allocated if true
 * @affinity:	Optional irq affinity mask for multiqueue devices
 *
 * Allocate IRQ numbers and initialized all data structures to support
 * hierarchy IRQ domains.
 * Parameter @realloc is mainly to support legacy IRQs.
 * Returns error code or allocated IRQ number
 *
 * The whole process to setup an IRQ has been split into two steps.
 * The first step, __irq_domain_alloc_irqs(), is to allocate IRQ
 * descriptor and required hardware resources. The second step,
 * irq_domain_activate_irq(), is to program hardwares with preallocated
 * resources. In this way, it's easier to rollback when failing to
 * allocate resources.
 */
int __irq_domain_alloc_irqs(struct irq_domain *domain, int irq_base,
			    unsigned int nr_irqs, void *arg,
			    bool realloc, const struct irq_affinity_desc *affinity)
{
	int i, ret, virq;

	if (domain == NULL) {
		domain = irq_default_domain;
		if (WARN(!domain, "domain is NULL; cannot allocate IRQ\n"))
			return -EINVAL;
	}

	if (!domain->ops->alloc) {
		pr_debug("domain->ops->alloc() is NULL\n");
		return -ENOSYS;
	}

	if (realloc && irq_base >= 0) {
		virq = irq_base;
	} else {
		virq = irq_domain_alloc_descs(irq_base, nr_irqs, 0, affinity);
		if (virq < 0) {
			pr_debug("cannot allocate IRQ(base %d, count %d)\n",
				 irq_base, nr_irqs);
			return virq;
		}
	}

	if (irq_domain_alloc_irq_data(domain, virq, nr_irqs)) {
		pr_debug("cannot allocate memory for IRQ%d\n", virq);
		ret = -ENOMEM;
		goto out_free_desc;
	}

	mutex_lock(&irq_domain_mutex);
	ret = irq_domain_alloc_irqs_hierarchy(domain, virq, nr_irqs, arg);
	if (ret < 0) {
		mutex_unlock(&irq_domain_mutex);
		goto out_free_irq_data;
	}
	for (i = 0; i < nr_irqs; i++)
		irq_domain_insert_irq(virq + i);
	mutex_unlock(&irq_domain_mutex);

	return virq;

out_free_irq_data:
	irq_domain_free_irq_data(virq, nr_irqs);
out_free_desc:
	irq_free_descs(virq, nr_irqs);
	return ret;
}

/* The irq_data was moved, fix the revmap to refer to the new location */
static void irq_domain_fix_revmap(struct irq_data *d)
{
	void **slot;

	/* Fix up the revmap. */
	mutex_lock(&d->domain->revmap_tree_mutex);
	slot = radix_tree_lookup_slot(&d->domain->revmap_tree, d->hwirq);
	if (slot)
		radix_tree_replace_slot(&d->domain->revmap_tree, slot, d);
	mutex_unlock(&d->domain->revmap_tree_mutex);
}

/**
 * irq_domain_push_irq() - Push a domain in to the top of a hierarchy.
 * @domain:	Domain to push.
 * @virq:	Irq to push the domain in to.
 * @arg:	Passed to the irq_domain_ops alloc() function.
 *
 * For an already existing irqdomain hierarchy, as might be obtained
 * via a call to pci_enable_msix(), add an additional domain to the
 * head of the processing chain.  Must be called before request_irq()
 * has been called.
 */
int irq_domain_push_irq(struct irq_domain *domain, int virq, void *arg)
{
	struct irq_data *child_irq_data;
	struct irq_data *root_irq_data = irq_get_irq_data(virq);
	struct irq_desc *desc;
	int rv = 0;

	/*
	 * Check that no action has been set, which indicates the virq
	 * is in a state where this function doesn't have to deal with
	 * races between interrupt handling and maintaining the
	 * hierarchy.  This will catch gross misuse.  Attempting to
	 * make the check race free would require holding locks across
	 * calls to struct irq_domain_ops->alloc(), which could lead
	 * to deadlock, so we just do a simple check before starting.
	 */
	desc = irq_to_desc(virq);
	if (!desc)
		return -EINVAL;
	if (WARN_ON(desc->action))
		return -EBUSY;

	if (domain == NULL)
		return -EINVAL;

	if (WARN_ON(!irq_domain_is_hierarchy(domain)))
		return -EINVAL;

	if (!root_irq_data)
		return -EINVAL;

	if (domain->parent != root_irq_data->domain)
		return -EINVAL;

	child_irq_data = kzalloc(sizeof(*child_irq_data), GFP_KERNEL);
	if (!child_irq_data)
		return -ENOMEM;

	mutex_lock(&irq_domain_mutex);

	/* Copy the original irq_data. */
	*child_irq_data = *root_irq_data;

	/*
	 * Overwrite the root_irq_data, which is embedded in struct
	 * irq_desc, with values for this domain.
	 */
	root_irq_data->parent_data = child_irq_data;
	root_irq_data->domain = domain;
	root_irq_data->mask = 0;
	root_irq_data->hwirq = 0;
	root_irq_data->chip = NULL;
	root_irq_data->chip_data = NULL;

	/* May (probably does) set hwirq, chip, etc. */
	rv = irq_domain_alloc_irqs_hierarchy(domain, virq, 1, arg);
	if (rv) {
		/* Restore the original irq_data. */
		*root_irq_data = *child_irq_data;
		goto error;
	}

	irq_domain_fix_revmap(child_irq_data);
	irq_domain_set_mapping(domain, root_irq_data->hwirq, root_irq_data);

error:
	mutex_unlock(&irq_domain_mutex);

	return rv;
}

/**
 * irq_domain_pop_irq() - Remove a domain from the top of a hierarchy.
 * @domain:	Domain to remove.
 * @virq:	Irq to remove the domain from.
 *
 * Undo the effects of a call to irq_domain_push_irq().  Must be
 * called either before request_irq() or after free_irq().
 */
int irq_domain_pop_irq(struct irq_domain *domain, int virq)
{
	struct irq_data *root_irq_data = irq_get_irq_data(virq);
	struct irq_data *child_irq_data;
	struct irq_data *tmp_irq_data;
	struct irq_desc *desc;

	/*
	 * Check that no action is set, which indicates the virq is in
	 * a state where this function doesn't have to deal with races
	 * between interrupt handling and maintaining the hierarchy.
	 * This will catch gross misuse.  Attempting to make the check
	 * race free would require holding locks across calls to
	 * struct irq_domain_ops->free(), which could lead to
	 * deadlock, so we just do a simple check before starting.
	 */
	desc = irq_to_desc(virq);
	if (!desc)
		return -EINVAL;
	if (WARN_ON(desc->action))
		return -EBUSY;

	if (domain == NULL)
		return -EINVAL;

	if (!root_irq_data)
		return -EINVAL;

	tmp_irq_data = irq_domain_get_irq_data(domain, virq);

	/* We can only "pop" if this domain is at the top of the list */
	if (WARN_ON(root_irq_data != tmp_irq_data))
		return -EINVAL;

	if (WARN_ON(root_irq_data->domain != domain))
		return -EINVAL;

	child_irq_data = root_irq_data->parent_data;
	if (WARN_ON(!child_irq_data))
		return -EINVAL;

	mutex_lock(&irq_domain_mutex);

	root_irq_data->parent_data = NULL;

	irq_domain_clear_mapping(domain, root_irq_data->hwirq);
	irq_domain_free_irqs_hierarchy(domain, virq, 1);

	/* Restore the original irq_data. */
	*root_irq_data = *child_irq_data;

	irq_domain_fix_revmap(root_irq_data);

	mutex_unlock(&irq_domain_mutex);

	kfree(child_irq_data);

	return 0;
}

/**
 * irq_domain_free_irqs - Free IRQ number and associated data structures
 * @virq:	base IRQ number
 * @nr_irqs:	number of IRQs to free
 */
void irq_domain_free_irqs(unsigned int virq, unsigned int nr_irqs)
{
	struct irq_data *data = irq_get_irq_data(virq);
	int i;

	if (WARN(!data || !data->domain || !data->domain->ops->free,
		 "NULL pointer, cannot free irq\n"))
		return;

	mutex_lock(&irq_domain_mutex);
	for (i = 0; i < nr_irqs; i++)
		irq_domain_remove_irq(virq + i);
	irq_domain_free_irqs_hierarchy(data->domain, virq, nr_irqs);
	mutex_unlock(&irq_domain_mutex);

	irq_domain_free_irq_data(virq, nr_irqs);
	irq_free_descs(virq, nr_irqs);
}

/**
 * irq_domain_alloc_irqs_parent - Allocate interrupts from parent domain
 * @irq_base:	Base IRQ number
 * @nr_irqs:	Number of IRQs to allocate
 * @arg:	Allocation data (arch/domain specific)
 *
 * Check whether the domain has been setup recursive. If not allocate
 * through the parent domain.
 */
int irq_domain_alloc_irqs_parent(struct irq_domain *domain,
				 unsigned int irq_base, unsigned int nr_irqs,
				 void *arg)
{
	if (!domain->parent)
		return -ENOSYS;

	return irq_domain_alloc_irqs_hierarchy(domain->parent, irq_base,
					       nr_irqs, arg);
}

/**
 * irq_domain_free_irqs_parent - Free interrupts from parent domain
 * @irq_base:	Base IRQ number
 * @nr_irqs:	Number of IRQs to free
 *
 * Check whether the domain has been setup recursive. If not free
 * through the parent domain.
 */
void irq_domain_free_irqs_parent(struct irq_domain *domain,
				 unsigned int irq_base, unsigned int nr_irqs)
{
	if (!domain->parent)
		return;

	irq_domain_free_irqs_hierarchy(domain->parent, irq_base, nr_irqs);
}

static void __irq_domain_deactivate_irq(struct irq_data *irq_data)
{
	if (irq_data && irq_data->domain) {
		struct irq_domain *domain = irq_data->domain;

		if (domain->ops->deactivate)
			domain->ops->deactivate(domain, irq_data);
		if (irq_data->parent_data)
			__irq_domain_deactivate_irq(irq_data->parent_data);
	}
}

static int __irq_domain_activate_irq(struct irq_data *irqd, bool reserve)
{
	int ret = 0;

	if (irqd && irqd->domain) {
		struct irq_domain *domain = irqd->domain;

		if (irqd->parent_data)
			ret = __irq_domain_activate_irq(irqd->parent_data,
							reserve);
		if (!ret && domain->ops->activate) {
			ret = domain->ops->activate(domain, irqd, reserve);
			/* Rollback in case of error */
			if (ret && irqd->parent_data)
				__irq_domain_deactivate_irq(irqd->parent_data);
		}
	}
	return ret;
}

/**
 * irq_domain_activate_irq - Call domain_ops->activate recursively to activate
 *			     interrupt
 * @irq_data:	Outermost irq_data associated with interrupt
 * @reserve:	If set only reserve an interrupt vector instead of assigning one
 *
 * This is the second step to call domain_ops->activate to program interrupt
 * controllers, so the interrupt could actually get delivered.
 */
int irq_domain_activate_irq(struct irq_data *irq_data, bool reserve)
{
	int ret = 0;

	if (!irqd_is_activated(irq_data))
		ret = __irq_domain_activate_irq(irq_data, reserve);
	if (!ret)
		irqd_set_activated(irq_data);
	return ret;
}

/**
 * irq_domain_deactivate_irq - Call domain_ops->deactivate recursively to
 *			       deactivate interrupt
 * @irq_data: outermost irq_data associated with interrupt
 *
 * It calls domain_ops->deactivate to program interrupt controllers to disable
 * interrupt delivery.
 */
void irq_domain_deactivate_irq(struct irq_data *irq_data)
{
	if (irqd_is_activated(irq_data)) {
		__irq_domain_deactivate_irq(irq_data);
		irqd_clr_activated(irq_data);
	}
}

/**
 * irq_domain_hierarchical_is_msi_remap - Check if the domain or any
 * parent has MSI remapping support
 * @domain: domain pointer
 */
bool irq_domain_hierarchical_is_msi_remap(struct irq_domain *domain)
{
	for (; domain; domain = domain->parent) {
		if (irq_domain_is_msi_remap(domain))
			return true;
	}
	return false;
}

/**
 * irq_domain_check_msi_remap - Check whether all MSI irq domains implement
 * IRQ remapping
 *
 * Return: false if any MSI irq domain does not support IRQ remapping,
 * true otherwise (including if there is no MSI irq domain)
 */
bool irq_domain_check_msi_remap(void)
{
	struct irq_domain *h;
	bool ret = true;

	mutex_lock(&irq_domain_mutex);
	list_for_each_entry(h, &irq_domain_list, link) {
		if (irq_domain_is_msi(h) &&
		    !irq_domain_hierarchical_is_msi_remap(h)) {
			ret = false;
			break;
		}
	}
	mutex_unlock(&irq_domain_mutex);
	return ret;
}
