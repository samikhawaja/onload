/****************************************************************************
 * Driver for Solarflare network controllers and boards
 * Copyright 2018 Solarflare Communications Inc.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation, incorporated herein by reference.
 */

#ifndef EFX_COMMON_H
#define EFX_COMMON_H

int efx_port_dummy_op_int(struct efx_nic *efx);
void efx_port_dummy_op_void(struct efx_nic *efx);

bool efx_is_supported_ringsize(struct efx_nic *efx, unsigned long entries);
bool efx_is_guaranteed_ringsize(struct efx_nic *efx, unsigned long entries);
unsigned long
efx_best_guaranteed_ringsize(struct efx_nic *efx, unsigned long entries,
			     bool fallback_to_supported);
unsigned long
efx_next_guaranteed_ringsize(struct efx_nic *efx, unsigned long entries,
			     bool fallback_to_supported);
int efx_init_io(struct efx_nic *efx, int bar, dma_addr_t dma_mask, unsigned int mem_map_size);
void efx_fini_io(struct efx_nic *efx);
int efx_pci_map_bar(struct efx_nic *efx, int bar, unsigned int mem_map_size,
		    resource_size_t *membase_phys_out,
		    void __iomem **membase_out);
void efx_pci_unmap_bar(struct efx_nic *efx, int bar,
		       resource_size_t membase_phys, void __iomem *membase);
int efx_init_probe_data(struct pci_dev *pci_dev,
			const struct efx_nic_type *nic_type,
			struct efx_probe_data **pd);
void efx_fini_probe_data(struct efx_probe_data *probe_data);

int efx_probe_common(struct efx_nic *efx);
void efx_remove_common(struct efx_nic *efx);

int efx_start_all(struct efx_nic *efx);
void efx_stop_all(struct efx_nic *efx);
int efx_try_recovery(struct efx_nic *efx);
#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NETDEV_STATS64_VOID)
void efx_net_stats(struct net_device *net_dev, struct rtnl_link_stats64 *stats);
#else
struct rtnl_link_stats64 *efx_net_stats(struct net_device *net_dev,
					struct rtnl_link_stats64 *stats);
#endif
void efx_reset_sw_stats(struct efx_nic *efx);
void efx_print_stopped_queues(struct efx_nic *efx);
#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_TX_TIMEOUT_TXQUEUE)
void efx_watchdog(struct net_device *net_dev, unsigned int txqueue);
#else
void efx_watchdog(struct net_device *net_dev);
#endif

#define EFX_ASSERT_RESET_SERIALISED(efx)                \
	do {                                            \
		if (efx->state != STATE_UNINIT && efx->state != STATE_PROBED) \
			ASSERT_RTNL();                  \
	} while (0)

void efx_reset_down(struct efx_nic *efx, enum reset_type method);
int efx_reset_up(struct efx_nic *efx, enum reset_type method, bool ok);
int efx_reset(struct efx_nic *efx, enum reset_type method);


extern unsigned int monitor_interval_ms;
void efx_start_monitor(struct efx_nic *efx);

int efx_create_reset_workqueue(void);
void efx_queue_reset_work(struct efx_nic *efx);
void efx_flush_reset_workqueue(struct efx_nic *efx);
void efx_destroy_reset_workqueue(void);
void efx_schedule_reset(struct efx_nic *efx, enum reset_type type);

static inline int efx_check_disabled(struct efx_nic *efx)
{
	if (efx->state == STATE_DISABLED || efx_recovering(efx->state)) {
		netif_err(efx, drv, efx->net_dev,
			  "device is disabled due to earlier errors\n");
		return -EIO;
	}
	return 0;
}
int efx_check_queue_size(struct efx_nic *efx, u32 *entries,
			 u32 min, u32 max, bool fix);

#ifdef CONFIG_SFC_MCDI_LOGGING
void efx_init_mcdi_logging(struct efx_nic *efx);
void efx_fini_mcdi_logging(struct efx_nic *efx);
#else
static inline void efx_init_mcdi_logging(struct efx_nic *efx) {}
static inline void efx_fini_mcdi_logging(struct efx_nic *efx) {}
#endif

/* V-ports */
struct efx_vport *efx_alloc_vport_entry(struct efx_nic *efx);
struct efx_vport *efx_find_vport_entry(struct efx_nic *efx, u16 id);
void efx_free_vport_entry(struct efx_vport *ctx);
int efx_vport_add(struct efx_nic *efx, u16 vlan, bool vlan_restrict);
int efx_vport_del(struct efx_nic *efx, u16 port_user_id);

int efx_set_multicast_loopback_suppression(struct efx_nic *efx,
					   bool suppress, u16 vport_id,
					   u8 stack_id);

int efx_mac_reconfigure(struct efx_nic *efx, bool mtu_only);
void efx_link_status_changed(struct efx_nic *efx);
int efx_reconfigure_port(struct efx_nic *efx);
int __efx_reconfigure_port(struct efx_nic *efx);
int efx_change_mtu(struct net_device *net_dev, int new_mtu);
int efx_set_mac_address(struct net_device *net_dev, void *data);
void efx_set_rx_mode(struct net_device *net_dev);

#ifdef EFX_NOT_UPSTREAM
#if IS_MODULE(CONFIG_SFC_DRIVERLINK)
void efx_dl_probe(struct efx_nic *efx);
bool efx_dl_supported(struct efx_nic *efx);
#endif
#endif

extern const struct pci_error_handlers efx_err_handlers;

#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_FEATURES_CHECK)
netdev_features_t efx_features_check(struct sk_buff *skb,
				     struct net_device *dev,
				     netdev_features_t features);
#endif

#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_GET_PHYS_PORT_ID)
int efx_get_phys_port_id(struct net_device *net_dev,
			 struct netdev_phys_item_id *ppid);
#endif

#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_GET_PHYS_PORT_NAME)
int efx_get_phys_port_name(struct net_device *net_dev,
			   char *name, size_t len);
#endif

#if defined(EFX_NOT_UPSTREAM) && defined(EFX_USE_SFC_LRO)
netdev_features_t efx_fix_features(struct net_device *net_dev, netdev_features_t data);
#endif
int efx_set_features(struct net_device *net_dev, netdev_features_t data);

#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_VLAN_RX_ADD_VID_PROTO)
int efx_vlan_rx_add_vid(struct net_device *net_dev, __be16 proto, u16 vid);
int efx_vlan_rx_kill_vid(struct net_device *net_dev, __be16 proto, u16 vid);
#elif defined(EFX_HAVE_NDO_VLAN_RX_ADD_VID_RC)
int efx_vlan_rx_add_vid(struct net_device *net_dev, u16 vid);
int efx_vlan_rx_kill_vid(struct net_device *net_dev, u16 vid);
#else
void efx_vlan_rx_add_vid(struct net_device *net_dev, unsigned short vid);
void efx_vlan_rx_kill_vid(struct net_device *net_dev, unsigned short vid);
#endif

#if !defined(EFX_USE_KCOMPAT) || defined(EFX_HAVE_NDO_HWTSTAMP_GET)
int efx_hwtstamp_set(struct net_device *net_dev,
		     struct kernel_hwtstamp_config *config,
		     struct netlink_ext_ack *extack);
int efx_hwtstamp_get(struct net_device *net_dev,
		     struct kernel_hwtstamp_config *config);
#endif

enum efx_tlv_state_machine {
	EFX_TLV_TYPE,
	EFX_TLV_TYPE_CONT,
	EFX_TLV_LENGTH,
	EFX_TLV_VALUE
};

struct efx_tlv_state {
	enum efx_tlv_state_machine state;
	u64 value;
	u32 value_offset;
	u16 type;
	u8 len;
};

typedef int efx_design_param_processor(struct efx_nic *efx,
				       const struct efx_tlv_state *reader);
typedef void efx_readd_fn(struct efx_nic *efx, efx_dword_t *value,
			  unsigned int reg);

/** efx_check_design_params - Read and parse the nic's design parameters.
 *
 * @efx: NIC to check the design parameters for.
 * @processor: Function to process the parsed design parameters.
 * @tlv_len_off: Offset in the NIC's BAR for PARAMS_TLV_LEN.
 * @tlv_off: Offset in the NIC's BAR for PARAMS_TLV.
 * @readd: Function to read a dword at a given offset in the NIC's BAR.
 *
 * Return: a negative error code, or 0 on success.
 */
int efx_check_design_params(struct efx_nic *efx,
			    efx_design_param_processor *processor,
			    int tlv_len_off, int tlv_off, size_t bar_size,
			    efx_readd_fn *readd);

#endif

