#ifndef __RTOCHIUS_GFP_H_
#define __RTOCHIUS_GFP_H_

#define GFP_KERNEL ((gfp_t) 1)
#define __GFP_ZERO ((gfp_t) 2)

#define __get_free_page(gfp_mask) 0

#define free_page(addr)
#define __free_page(page) 
#define alloc_pages(gfp_mask, order) 0

#endif /* !__RTOCHIUS_GFP_H_ */
