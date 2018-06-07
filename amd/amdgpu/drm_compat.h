#pragma once

#include <drm/drm_prime.h>
#include <drm/drm_gem.h>

//////// Kernel 4.12 and below
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
#define for_each_old_connector_in_state(old_state, connector, old_conn_state, i) \
	for_each_connector_in_state(old_state, connector, old_conn_state, i)
#define for_each_old_crtc_in_state (old_state, crtc, old_crtc_state, i) \
	for_each_crtc_in_state(old_state, crtc, old_crtc_state, i)
#endif


//////// Kernel 4.15 and above
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0)
// this one got dropped, c.f https://lists.freedesktop.org/archives/dri-devel/2017-November/156484.html
#define drm_edid_to_eld(connector, edid)

#define release_pages(x, y, z) release_pages(x, y)

#define drm_mode_object_find(con, id, x) drm_mode_object_find(con, NULL, id, x)

#define drm_encoder_find(x, y) drm_encoder_find(x, 0, y)
#endif


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0) && \
    LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)

// Code taken from 4.17; that's now exported as a symbol

/**
 * drm_gem_map_attach - dma_buf attach implementation for GEM
 * @dma_buf: buffer to attach device to
 * @target_dev: not used
 * @attach: buffer attachment data
 *
 * Allocates &drm_prime_attachment and calls &drm_driver.gem_prime_pin for
 * device specific attachment. This can be used as the &dma_buf_ops.attach
 * callback.
 *
 * Returns 0 on success, negative error code on failure.
 */
int drm_gem_map_attach(struct dma_buf *dma_buf, struct device *target_dev,
		       struct dma_buf_attachment *attach);

/**
 * drm_gem_map_detach - dma_buf detach implementation for GEM
 * @dma_buf: buffer to detach from
 * @attach: attachment to be detached
 *
 * Cleans up &dma_buf_attachment. This can be used as the &dma_buf_ops.detach
 * callback.
 */
void drm_gem_map_detach(struct dma_buf *dma_buf,
			struct dma_buf_attachment *attach);
/**
 * drm_gem_map_dma_buf - map_dma_buf implementation for GEM
 * @attach: attachment whose scatterlist is to be returned
 * @dir: direction of DMA transfer
 *
 * Calls &drm_driver.gem_prime_get_sg_table and then maps the scatterlist. This
 * can be used as the &dma_buf_ops.map_dma_buf callback.
 *
 * Returns sg_table containing the scatterlist to be returned; returns ERR_PTR
 * on error. May return -EINTR if it is interrupted by a signal.
 */

struct sg_table *drm_gem_map_dma_buf(struct dma_buf_attachment *attach,
				     enum dma_data_direction dir);

/**
 * drm_gem_unmap_dma_buf - unmap_dma_buf implementation for GEM
 * @attach: attachment to unmap buffer from
 * @sgt: scatterlist info of the buffer to unmap
 * @dir: direction of DMA transfer
 *
 * Not implemented. The unmap is done at drm_gem_map_detach().  This can be
 * used as the &dma_buf_ops.unmap_dma_buf callback.
 */
void drm_gem_unmap_dma_buf(struct dma_buf_attachment *attach,
			   struct sg_table *sgt,
			   enum dma_data_direction dir);

/**
 * drm_gem_dmabuf_kmap_atomic - map_atomic implementation for GEM
 * @dma_buf: buffer to be mapped
 * @page_num: page number within the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.map_atomic callback.
 */
void *drm_gem_dmabuf_kmap_atomic(struct dma_buf *dma_buf,
				 unsigned long page_num);

/**
 * drm_gem_dmabuf_kmap - map implementation for GEM
 * @dma_buf: buffer to be mapped
 * @page_num: page number within the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.map callback.
 */
void *drm_gem_dmabuf_kmap(struct dma_buf *dma_buf, unsigned long page_num);

/**
 * drm_gem_dmabuf_kunmap_atomic - unmap_atomic implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @page_num: page number within the buffer
 * @addr: virtual address of the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.unmap_atomic callback.
 */
void drm_gem_dmabuf_kunmap_atomic(struct dma_buf *dma_buf,
				  unsigned long page_num, void *addr);

/**
 * drm_gem_dmabuf_kunmap - unmap implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @page_num: page number within the buffer
 * @addr: virtual address of the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.unmap callback.
 */
void drm_gem_dmabuf_kunmap(struct dma_buf *dma_buf, unsigned long page_num,
			   void *addr);

/**
 * drm_gem_dmabuf_mmap - dma_buf mmap implementation for GEM
 * @dma_buf: buffer to be mapped
 * @vma: virtual address range
 *
 * Provides memory mapping for the buffer. This can be used as the
 * &dma_buf_ops.mmap callback.
 *
 * Returns 0 on success or a negative error code on failure.
 */
int drm_gem_dmabuf_mmap(struct dma_buf *dma_buf, struct vm_area_struct *vma);

/**
 * drm_gem_dmabuf_vmap - dma_buf vmap implementation for GEM
 * @dma_buf: buffer to be mapped
 *
 * Sets up a kernel virtual mapping. This can be used as the &dma_buf_ops.vmap
 * callback.
 *
 * Returns the kernel virtual address.
 */
void *drm_gem_dmabuf_vmap(struct dma_buf *dma_buf);

/**
 * drm_gem_dmabuf_vunmap - dma_buf vunmap implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @vaddr: the virtual address of the buffer
 *
 * Releases a kernel virtual mapping. This can be used as the
 * &dma_buf_ops.vunmap callback.
 */
void drm_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr);

#endif // LINUX 4.15,16
