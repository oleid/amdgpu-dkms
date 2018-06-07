#include "drm_compat.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0) && \
    LINUX_VERSION_CODE < KERNEL_VERSION(4, 17, 0)

// Code taken from 4.17; that's now exported as a symbol

struct drm_prime_member {
	struct dma_buf *dma_buf;
	uint32_t handle;

	struct rb_node dmabuf_rb;
	struct rb_node handle_rb;
};

struct drm_prime_attachment {
	struct sg_table *sgt;
	enum dma_data_direction dir;
};


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
		       struct dma_buf_attachment *attach)
{
	struct drm_prime_attachment *prime_attach;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	prime_attach = kzalloc(sizeof(*prime_attach), GFP_KERNEL);
	if (!prime_attach)
		return -ENOMEM;

	prime_attach->dir = DMA_NONE;
	attach->priv = prime_attach;

	if (!dev->driver->gem_prime_pin)
		return 0;

	return dev->driver->gem_prime_pin(obj);
}

/**
 * drm_gem_map_detach - dma_buf detach implementation for GEM
 * @dma_buf: buffer to detach from
 * @attach: attachment to be detached
 *
 * Cleans up &dma_buf_attachment. This can be used as the &dma_buf_ops.detach
 * callback.
 */
void drm_gem_map_detach(struct dma_buf *dma_buf,
			struct dma_buf_attachment *attach)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (prime_attach) {
		struct sg_table *sgt = prime_attach->sgt;

		if (sgt) {
			if (prime_attach->dir != DMA_NONE)
				dma_unmap_sg_attrs(attach->dev, sgt->sgl,
						   sgt->nents,
						   prime_attach->dir,
						   DMA_ATTR_SKIP_CPU_SYNC);
			sg_free_table(sgt);
		}

		kfree(sgt);
		kfree(prime_attach);
		attach->priv = NULL;
	}

	if (dev->driver->gem_prime_unpin)
		dev->driver->gem_prime_unpin(obj);
}

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
				     enum dma_data_direction dir)
{
	struct drm_prime_attachment *prime_attach = attach->priv;
	struct drm_gem_object *obj = attach->dmabuf->priv;
	struct sg_table *sgt;

	if (WARN_ON(dir == DMA_NONE || !prime_attach))
		return ERR_PTR(-EINVAL);

	/* return the cached mapping when possible */
	if (prime_attach->dir == dir)
		return prime_attach->sgt;

	/*
	 * two mappings with different directions for the same attachment are
	 * not allowed
	 */
	if (WARN_ON(prime_attach->dir != DMA_NONE))
		return ERR_PTR(-EBUSY);

	sgt = obj->dev->driver->gem_prime_get_sg_table(obj);

	if (!IS_ERR(sgt)) {
		if (!dma_map_sg_attrs(attach->dev, sgt->sgl, sgt->nents, dir,
				      DMA_ATTR_SKIP_CPU_SYNC)) {
			sg_free_table(sgt);
			kfree(sgt);
			sgt = ERR_PTR(-ENOMEM);
		} else {
			prime_attach->sgt = sgt;
			prime_attach->dir = dir;
		}
	}

	return sgt;
}

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
			   enum dma_data_direction dir)
{
	/* nothing to be done here */
}

/**
 * drm_gem_dmabuf_kmap_atomic - map_atomic implementation for GEM
 * @dma_buf: buffer to be mapped
 * @page_num: page number within the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.map_atomic callback.
 */
void *drm_gem_dmabuf_kmap_atomic(struct dma_buf *dma_buf,
				 unsigned long page_num)
{
	return NULL;
}

/**
 * drm_gem_dmabuf_kmap - map implementation for GEM
 * @dma_buf: buffer to be mapped
 * @page_num: page number within the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.map callback.
 */
void *drm_gem_dmabuf_kmap(struct dma_buf *dma_buf, unsigned long page_num)
{
	return NULL;
}

/**
 * drm_gem_dmabuf_kunmap_atomic - unmap_atomic implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @page_num: page number within the buffer
 * @addr: virtual address of the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.unmap_atomic callback.
 */
void drm_gem_dmabuf_kunmap_atomic(struct dma_buf *dma_buf,
				  unsigned long page_num, void *addr)
{

}

/**
 * drm_gem_dmabuf_kunmap - unmap implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @page_num: page number within the buffer
 * @addr: virtual address of the buffer
 *
 * Not implemented. This can be used as the &dma_buf_ops.unmap callback.
 */
void drm_gem_dmabuf_kunmap(struct dma_buf *dma_buf, unsigned long page_num,
			   void *addr)
{

}

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
int drm_gem_dmabuf_mmap(struct dma_buf *dma_buf, struct vm_area_struct *vma)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (!dev->driver->gem_prime_mmap)
		return -ENOSYS;

	return dev->driver->gem_prime_mmap(obj, vma);
}


/**
 * drm_gem_dmabuf_vmap - dma_buf vmap implementation for GEM
 * @dma_buf: buffer to be mapped
 *
 * Sets up a kernel virtual mapping. This can be used as the &dma_buf_ops.vmap
 * callback.
 *
 * Returns the kernel virtual address.
 */
void *drm_gem_dmabuf_vmap(struct dma_buf *dma_buf)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (dev->driver->gem_prime_vmap)
		return dev->driver->gem_prime_vmap(obj);
	else
		return NULL;
}

/**
 * drm_gem_dmabuf_vunmap - dma_buf vunmap implementation for GEM
 * @dma_buf: buffer to be unmapped
 * @vaddr: the virtual address of the buffer
 *
 * Releases a kernel virtual mapping. This can be used as the
 * &dma_buf_ops.vunmap callback.
 */
void drm_gem_dmabuf_vunmap(struct dma_buf *dma_buf, void *vaddr)
{
	struct drm_gem_object *obj = dma_buf->priv;
	struct drm_device *dev = obj->dev;

	if (dev->driver->gem_prime_vunmap)
		dev->driver->gem_prime_vunmap(obj, vaddr);
}

#endif // LINUX 4.15,16
