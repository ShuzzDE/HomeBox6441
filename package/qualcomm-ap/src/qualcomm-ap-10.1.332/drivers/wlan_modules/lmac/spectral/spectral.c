/*
 * Copyright (c) 2002-2009 Atheros Communications, Inc.
 * All Rights Reserved.
 *
 * Copyright (c) 2011 Qualcomm Atheros, Inc.
 * All Rights Reserved.
 * Qualcomm Atheros Confidential and Proprietary.
 *
 */
#include <osdep.h>
#include "ah.h"
#include "spectral.h"
#include "ath_internal.h"
#include "dfs_ioctl.h"
#include "ieee80211_var.h"

#if ATH_SUPPORT_SPECTRAL

// Globals
int BTH_MIN_NUMBER_OF_FRAMES    =   8;
int spectral_debug_level        =   ATH_DEBUG_SPECTRAL;
static int nobeacons    =   0;
static int nfrssi       =   0;
static int maxholdintvl =   0;

// Function declarations
static u_int64_t null_get_tsf64(void* arg);
static u_int32_t null_get_capability(void* arg, HAL_CAPABILITY_TYPE type);
static u_int32_t null_set_rxfilter(void* arg, int rxfilter);
static u_int32_t null_get_rxfilter(void* arg);
static u_int32_t null_is_spectral_active(void* arg);
static u_int32_t null_is_spectral_enabled(void* arg);
static u_int32_t null_start_spectral_scan(void* arg);
static u_int32_t null_stop_spectral_scan(void* arg);
static u_int32_t null_get_extension_channel(void* arg);
static int8_t null_get_ctl_noisefloor(void* arg);
static int8_t null_get_ext_noisefloor(void* arg);
static u_int32_t null_configure_spectral(void* arg, HAL_SPECTRAL_PARAM* params);
static u_int32_t null_get_spectral_config(void* arg, HAL_SPECTRAL_PARAM* params);
static u_int32_t null_get_ent_spectral_mask(void* arg);
static u_int32_t null_get_mac_address(void* arg, char* addr);
static u_int32_t null_get_current_channel(void* arg);
static u_int32_t null_reset_hw(void* arg);
static u_int32_t null_get_chain_noise_floor(void* arg, int16_t* nfBuf);
static void test_spectral_ops(struct ath_spectral* spectral);
static void spectral_init_param_defaults(HAL_SPECTRAL_PARAM* params);

/*
 * Function     : spectral_init_dummy_function_table
 * Description  : Initializes dummy Spectral functions
 * Input        : Pointer to Spectral Struct
 * Output       : Void
 *
 */
void spectral_init_dummy_function_table(struct ath_spectral* ps)
{
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(ps);

    p_sops->get_tsf64               = null_get_tsf64;
    p_sops->get_capability          = null_get_capability;
    p_sops->set_rxfilter            = null_set_rxfilter;
    p_sops->get_rxfilter            = null_get_rxfilter;
    p_sops->is_spectral_enabled     = null_is_spectral_enabled;
    p_sops->is_spectral_active      = null_is_spectral_active;
    p_sops->start_spectral_scan     = null_start_spectral_scan;
    p_sops->stop_spectral_scan      = null_stop_spectral_scan;
    p_sops->get_extension_channel   = null_get_extension_channel;
    p_sops->get_ctl_noisefloor      = null_get_ctl_noisefloor;
    p_sops->get_ext_noisefloor      = null_get_ext_noisefloor;
    p_sops->configure_spectral      = null_configure_spectral;
    p_sops->get_spectral_config     = null_get_spectral_config;
    p_sops->get_ent_spectral_mask   = null_get_ent_spectral_mask;
    p_sops->get_mac_address         = null_get_mac_address;
    p_sops->get_current_channel     = null_get_current_channel;
    p_sops->reset_hw                = null_reset_hw;
    p_sops->get_chain_noise_floor   = null_get_chain_noise_floor;

}

/*
 * Function     : spectral_init_param_defaults
 * Description  : Initializes Spectral parameter defaults.
 *                It is the caller's responsibility to ensure
 *                that the Spectral parameters structure passed
 *                is valid.
 * Input        : Pointer to Spectral parameters structure
 * Output       : Void
 *
 */
static void spectral_init_param_defaults(HAL_SPECTRAL_PARAM* params)
{
    params->ss_count = SPECTRAL_SCAN_COUNT_DEFAULT;
    params->ss_period = SPECTRAL_SCAN_PERIOD_DEFAULT;
    params->ss_spectral_pri = SPECTRAL_SCAN_PRIORITY_DEFAULT;
    params->ss_fft_size = SPECTRAL_SCAN_FFT_SIZE_DEFAULT;
    params->ss_gc_ena = SPECTRAL_SCAN_GC_ENA_DEFAULT;
    params->ss_restart_ena = SPECTRAL_SCAN_RESTART_ENA_DEFAULT;
    params->ss_noise_floor_ref = SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT;
    params->ss_init_delay = SPECTRAL_SCAN_INIT_DELAY_DEFAULT;
    params->ss_nb_tone_thr = SPECTRAL_SCAN_NB_TONE_THR_DEFAULT;
    params->ss_str_bin_thr = SPECTRAL_SCAN_STR_BIN_THR_DEFAULT;
    params->ss_wb_rpt_mode = SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT;
    params->ss_rssi_rpt_mode = SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT;
    params->ss_rssi_thr = SPECTRAL_SCAN_RSSI_THR_DEFAULT;
    params->ss_pwr_format = SPECTRAL_SCAN_PWR_FORMAT_DEFAULT;
    params->ss_rpt_mode = SPECTRAL_SCAN_RPT_MODE_DEFAULT;
    params->ss_bin_scale = SPECTRAL_SCAN_BIN_SCALE_DEFAULT;
    params->ss_dBm_adj = SPECTRAL_SCAN_DBM_ADJ_DEFAULT;
    params->ss_chn_mask = SPECTRAL_SCAN_CHN_MASK_DEFAULT;
    
    params->ss_short_report = SPECTRAL_SCAN_SHORT_REPORT_DEFAULT;
    params->ss_fft_period = SPECTRAL_SCAN_FFT_PERIOD_DEFAULT;
}

/*
 * Function     : spectral_check_hw_capability
 * Description  : Check Hardware capabaility
 * Input        : Pointer to IC
 * Output       : Void
 *
 */
int spectral_check_hw_capability(struct ieee80211com* ic)
{
    struct ath_spectral* spectral   = NULL;
    struct spectral_ops* p_sops     = NULL;
    struct ath_spectral_caps* pcap  = NULL;

    int is_spectral_supported = AH_TRUE;

    spectral    = ic->ic_spectral;
    p_sops      = GET_SPECTRAL_OPS(spectral);
    pcap        = &spectral->capability;

    if (p_sops->get_capability(spectral, HAL_CAP_PHYDIAG) == AH_FALSE) {
        is_spectral_supported = AH_FALSE;
        printk("SPECTRAL : No PHYDIAG support\n");
        return is_spectral_supported;
    } else {
        pcap->phydiag_cap = 1;
    }

    if (p_sops->get_capability(spectral, HAL_CAP_RADAR) == AH_FALSE) {
        is_spectral_supported = AH_FALSE;
        printk("SPECTRAL : No RADAR support\n");
        return is_spectral_supported;
    } else {
        pcap->radar_cap = 1;
    }

    if (p_sops->get_capability(spectral, HAL_CAP_SPECTRAL_SCAN) == AH_FALSE) {
        is_spectral_supported = AH_FALSE;
        printk("SPECTRAL : No SPECTRAL SUPPORT\n");
        return is_spectral_supported;
    } else {
        pcap->spectral_cap = 1;
    }

    if (p_sops->get_capability(spectral, HAL_CAP_ADVNCD_SPECTRAL_SCAN)
            == AH_FALSE) {
        printk("SPECTRAL : No ADVANCED SPECTRAL SUPPORT\n");
    } else {
        pcap->advncd_spectral_cap = 1;
    }

    return is_spectral_supported;
}OS_EXPORT_SYMBOL(spectral_check_hw_capability);


/*
 * Function     : spectral_clear_stats
 * Description  : Clears Spectral related stats structure
 * Input        : Pointer to Spectral Struct
 * Output       : Void
 *
 */
void spectral_clear_stats(struct ath_spectral* spectral)
{
    SPECTRAL_OPS *p_sops = GET_SPECTRAL_OPS(spectral);
    OS_MEMZERO(&spectral->ath_spectral_stats, sizeof (struct spectral_stats));
    spectral->ath_spectral_stats.last_reset_tstamp = p_sops->get_tsf64(spectral);
}


/*
 * Function     : spectral_attach
 * Description  : Attaches Spectral module and it's data structures
 * Input        : Pointer to IC
 * Output       : Pointer to spectral
 *
 */
void* spectral_attach(struct ieee80211com* ic)
{
    struct ath_spectral *spectral = NULL;
    SPECTRAL_OPS *p_sops = NULL;

    if (ic->ic_spectral != NULL) {
        printk("SPECTRAL : module already attached\n");
        return spectral;
    }

    spectral = (struct ath_spectral *)OS_MALLOC(ic->ic_osdev, sizeof(struct ath_spectral), GFP_KERNEL);

    if (spectral == NULL) {
        printk("SPECTRAL : Memory allocation failed\n");
        return spectral;
    }

    OS_MEMZERO(spectral, sizeof (struct ath_spectral));

    /* Store IC in Spectral and vice versa */
    spectral->ic = ic;
    ic->ic_spectral = spectral;

    /* init the function ptr table */
    spectral_init_dummy_function_table(spectral);

    /* get spectral function table */
    p_sops = GET_SPECTRAL_OPS(spectral);


    /* TODO : Should this be called here of after ath_attach ? */
    if (p_sops->get_capability(spectral, HAL_CAP_PHYDIAG)) {
        printk("HAL_CAP_PHYDIAG : Capable\n");
    }

    SPECTRAL_TODO("Need to fix the capablity check for RADAR");
    if (p_sops->get_capability(spectral, HAL_CAP_RADAR)) {
        printk("HAL_CAP_RADAR   : Capable\n");
    }

    SPECTRAL_TODO("Need to fix the capablity check for SPECTRAL\n");
    /* TODO : Should this be called here of after ath_attach ? */
    if (p_sops->get_capability(spectral, HAL_CAP_SPECTRAL_SCAN)) {
        printk("HAL_CAP_SPECTRAL_SCAN : Capable\n");
    }

    SPECTRAL_LOCK_INIT(spectral);
    spin_lock_init(&spectral->noise_pwr_reports_lock);
    spectral_clear_stats(spectral);

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
    spectral_init_netlink(spectral);
#endif

    /* Set the default values for spectral parameters */
    spectral_init_param_defaults(&spectral->params);

    return spectral;
}

/*
 * Function     : spectral_detach
 * Description  : Deattach Spectral module and it's data structures
 * Input        : Pointer to IC
 * Output       : Success/Failure
 *
 */
void spectral_detach(struct ieee80211com* ic)
{

    struct ath_spectral * spectral = ic->ic_spectral;

    if (spectral == NULL) {
       printk("SPECTRAL : Module dosen't exist\n");
       return;
    }

    printk("SPECTRAL : Module removed (spectral = %p)\n", spectral);

#ifdef SPECTRAL_USE_NETLINK_SOCKETS
    spectral_destroy_netlink(spectral);
#endif

    SPECTRAL_LOCK_DESTROY(spectral);

    spin_lock_destroy(&spectral->noise_pwr_reports_lock);

    if (spectral) {
       OS_FREE(spectral);
       spectral = NULL;
       ic->ic_spectral = NULL;
    }

}

/*
 * Function     : print_chan_loading_details
 * Description  : Debug function to print channel loading information
 * Input        : Pointer to Spectral
 * Output       : void
 *
 */
static void print_chan_loading_details(struct ath_spectral* spectral)
{
    printk("ctl_chan_freq       = %d\n",spectral->ctl_chan_frequency);
    printk("ctl_interf_count    = %d\n",spectral->ctl_eacs_interf_count);
    printk("ctl_duty_cycle      = %d\n",spectral->ctl_eacs_duty_cycle);
    printk("ctl_chan_loading    = %d\n",spectral->ctl_chan_loading);
    printk("ctl_nf              = %d\n",spectral->ctl_chan_noise_floor);

    if (spectral->sc_spectral_20_40_mode) {
        // HT40 mode
        printk("ext_chan_freq       = %d\n",spectral->ext_chan_frequency);
        printk("ext_interf_count    = %d\n",spectral->ext_eacs_interf_count);
        printk("ext_duty_cycle      = %d\n",spectral->ext_eacs_duty_cycle);
        printk("ext_chan_loading    = %d\n",spectral->ext_chan_loading);
        printk("ext_nf              = %d\n",spectral->ext_chan_noise_floor);
    }

    printk("%s this_scan_spectral_data count = %d\n", __func__, spectral->eacs_this_scan_spectral_data);
}

/*
 * Function     : init_chan_loading
 * Description  : Initializes Channel loading information
 * Input        : Pointer to Spectral
 * Output       : void
 *
 */
void init_chan_loading(struct ath_spectral* spectral)
{

    int current_channel     = 0;
    int ext_channel         = 0;
    SPECTRAL_OPS* p_sops    = GET_SPECTRAL_OPS(spectral);

    current_channel = p_sops->get_current_channel(spectral);
    ext_channel     = p_sops->get_extension_channel(spectral);

    if ((current_channel == 0) || (ext_channel == 0)) {
        return;
    }

    /* Check if channel change has happened in this reset */
    if(spectral->ctl_chan_frequency != current_channel) {
        /* If needed, check the channel loading details
         * print_chan_loading_details(spectral);
         */
        spectral->eacs_this_scan_spectral_data = 0;
    }

    if (spectral->sc_spectral_20_40_mode) {
        // HT40 mode
        spectral->ext_eacs_interf_count = 0;
        spectral->ext_eacs_duty_cycle   = 0;
        spectral->ext_chan_loading      = 0;
        spectral->ext_chan_noise_floor  = 0;
        spectral->ext_chan_frequency    = current_channel;
        spectral->ext_chan_frequency    = ext_channel;
    }

    spectral->ctl_eacs_interf_count = 0;
    spectral->ctl_eacs_duty_cycle   = 0;
    spectral->ctl_chan_loading      = 0;
    spectral->ctl_chan_noise_floor  = 0;
    spectral->ctl_chan_frequency    = current_channel;
    spectral->ctl_chan_frequency    = ext_channel;

    spectral->ctl_eacs_rssi_thresh = SPECTRAL_EACS_RSSI_THRESH;
    spectral->ext_eacs_rssi_thresh = SPECTRAL_EACS_RSSI_THRESH;

    spectral->ctl_eacs_avg_rssi = SPECTRAL_EACS_RSSI_THRESH;
    spectral->ext_eacs_avg_rssi = SPECTRAL_EACS_RSSI_THRESH;

    spectral->ctl_eacs_spectral_reports = 0;
    spectral->ext_eacs_spectral_reports = 0;
}

static void init_upper_lower_flags(struct ath_spectral *spectral)
{

    int current_channel = 0;
    int ext_channel = 0;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);

    current_channel = p_sops->get_current_channel(spectral);
    ext_channel     = p_sops->get_extension_channel(spectral);

    if ((current_channel == 0) || (ext_channel == 0)) {
        return;
    }

    if (spectral->sc_spectral_20_40_mode) {
        // HT40 mode
        if (ext_channel < current_channel) {
            spectral->lower_is_extension = 1;
            spectral->upper_is_control   = 1;
            spectral->lower_is_control   = 0;
            spectral->upper_is_extension = 0;
        } else {
            spectral->lower_is_extension = 0;
            spectral->upper_is_control   = 0;
            spectral->lower_is_control   = 1;
            spectral->upper_is_extension = 1;
        }
    } else {
      // HT20 mode, lower is always control
      spectral->lower_is_extension = 0;
      spectral->upper_is_control   = 0;
      spectral->lower_is_control   = 1;
      spectral->upper_is_extension = 0;
    }

}


int get_fft_bin_count(int fft_len)
{
    int bin_count = 0;
    switch(fft_len) {
        case 5:
          bin_count = 16;
          break;
        case 6:
          bin_count = 32;
          break;
        case 7:
          bin_count = 64;
          break;
        case 8:
          bin_count = 128;
          break;
        case 9:
          bin_count = 256;
          break;
        default:
          break;
    }

    return bin_count;
}

/*
 * Function     : spectral_scan_enable_params
 * Description  : Program Spectral Scan Params
 * Input        : Pointer to Spectral Struct and HAL_SPECTRAL_PARAM struct
 * Output       : Success/Failure
 *
 */
int spectral_scan_enable_params(struct ath_spectral *spectral, HAL_SPECTRAL_PARAM* spectral_params)
{
    u_int32_t rfilt         = 0;
    int extension_channel   = 0;
    int current_channel     = 0;
    SPECTRAL_OPS* p_sops    = NULL;
    struct ieee80211com* ic = NULL;

    if (spectral == NULL) {
        printk("SPECTRAL : Spectral is NULL\n");
        return 1;
    }

    ic = spectral->ic;
    p_sops = GET_SPECTRAL_OPS(spectral);

    if (p_sops == NULL) {
        printk("SPECTRAL : p_sops is NULL\n");
        return 1;
    }

    /* get the receive filters */
    rfilt = p_sops->get_rxfilter(spectral);
    rfilt &= ~HAL_RX_FILTER_PHYRADAR;

    p_sops->set_rxfilter(spectral, rfilt);
    rfilt |= HAL_RX_FILTER_PHYRADAR;
    p_sops->set_rxfilter(spectral, rfilt);

    spectral->sc_spectral_noise_pwr_cal = spectral_params->ss_spectral_pri ? 1 : 0;


    /* check if extension channel is present */
    extension_channel   = p_sops->get_extension_channel(spectral);
    current_channel     = p_sops->get_current_channel(spectral);
    spectral->ch_width  = ic->ic_cwm_get_width(ic);

    if (spectral->capability.advncd_spectral_cap) {
        if (spectral->ch_width == IEEE80211_CWM_WIDTH20) {
            printk("SPECTRAL : (11AC) 20MHz Channel Width (Channel = %d)\n", current_channel);
            spectral->sc_spectral_20_40_mode    = 0;

            spectral->spectral_numbins  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_fft_len  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_data_len = get_fft_bin_count(spectral->params.ss_fft_size);

            /* Initialize classifier params to be sent to user space classifier */
            spectral->classifier_params.lower_chan_in_mhz = current_channel;
            spectral->classifier_params.upper_chan_in_mhz = 0;

        } else if (spectral->ch_width == IEEE80211_CWM_WIDTH40) {
            printk("SPECTRAL : (11AC) 40MHz Channel Width (Channel = %d)\n", current_channel);
            spectral->sc_spectral_20_40_mode    = 1;    // TODO : Remove this variable

            spectral->spectral_numbins  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_fft_len  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_data_len = get_fft_bin_count(spectral->params.ss_fft_size);

            /* Initialize classifier params to be sent to user space classifier */
            if (extension_channel < current_channel) {
                 spectral->classifier_params.lower_chan_in_mhz = extension_channel;
                 spectral->classifier_params.upper_chan_in_mhz = current_channel;
            } else {
                 spectral->classifier_params.lower_chan_in_mhz = current_channel;
                 spectral->classifier_params.upper_chan_in_mhz = extension_channel;
            }

        } else if (spectral->ch_width == IEEE80211_CWM_WIDTH80) {
            printk("SPECTRAL : (11AC) 80MHz Channel Width (Channel = %d)\n", current_channel);
            /* Set the FFT Size */
            spectral->sc_spectral_20_40_mode    = 0;    // TODO : Remove this variable
            spectral->spectral_numbins  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_fft_len  = get_fft_bin_count(spectral->params.ss_fft_size);
            spectral->spectral_data_len = get_fft_bin_count(spectral->params.ss_fft_size);

            /* Initialize classifier params to be sent to user space classifier */
            spectral->classifier_params.lower_chan_in_mhz = current_channel;
            spectral->classifier_params.upper_chan_in_mhz = 0;

            /* Initialize classifier params to be sent to user space classifier */
            if (extension_channel < current_channel) {
                 spectral->classifier_params.lower_chan_in_mhz = extension_channel;
                 spectral->classifier_params.upper_chan_in_mhz = current_channel;
            } else {
                 spectral->classifier_params.lower_chan_in_mhz = current_channel;
                 spectral->classifier_params.upper_chan_in_mhz = extension_channel;
            }

        }
    } else {
        printk("SPECTRAL : Legacy (Non-11AC)\n");
        if (spectral->ch_width == IEEE80211_CWM_WIDTH20) {
            printk("SPECTRAL : (Legacy) 20MHz Channel Width (Channel = %d)\n", current_channel);
            spectral->spectral_numbins                  = SPECTRAL_HT20_NUM_BINS;
            spectral->spectral_dc_index                 = SPECTRAL_HT20_DC_INDEX;
            spectral->spectral_fft_len                  = SPECTRAL_HT20_FFT_LEN;
            spectral->spectral_data_len                 = SPECTRAL_HT20_TOTAL_DATA_LEN;
            spectral->spectral_lower_max_index_offset   = -1; // only valid in 20-40 mode
            spectral->spectral_upper_max_index_offset   = -1; // only valid in 20-40 mode
            spectral->spectral_max_index_offset         = spectral->spectral_fft_len + 2;
            spectral->sc_spectral_20_40_mode            = 0;

            /* Initialize classifier params to be sent to user space classifier */
            spectral->classifier_params.lower_chan_in_mhz = current_channel;
            spectral->classifier_params.upper_chan_in_mhz = 0;

        } else if (spectral->ch_width == IEEE80211_CWM_WIDTH40) {
            printk("SPECTRAL : (Legacy) 40MHz Channel Width (Channel = %d)\n", current_channel);
            spectral->spectral_numbins                  = SPECTRAL_HT40_TOTAL_NUM_BINS;
            spectral->spectral_fft_len                  = SPECTRAL_HT40_FFT_LEN;
            spectral->spectral_data_len                 = SPECTRAL_HT40_TOTAL_DATA_LEN;
            spectral->spectral_dc_index                 = SPECTRAL_HT40_DC_INDEX;
            spectral->spectral_max_index_offset         = -1; //only valid in 20 mode
            spectral->spectral_lower_max_index_offset   = spectral->spectral_fft_len + 2;
            spectral->spectral_upper_max_index_offset   = spectral->spectral_fft_len + 5;
            spectral->sc_spectral_20_40_mode            = 1;

            /* Initialize classifier params to be sent to user space classifier */
            if (extension_channel < current_channel) {
                 spectral->classifier_params.lower_chan_in_mhz = extension_channel;
                 spectral->classifier_params.upper_chan_in_mhz = current_channel;
            } else {
                 spectral->classifier_params.lower_chan_in_mhz = current_channel;
                 spectral->classifier_params.upper_chan_in_mhz = extension_channel;
            }
        }
    }

    spectral->send_single_packet                    = 0;
    spectral->classifier_params.spectral_20_40_mode = spectral->sc_spectral_20_40_mode;
    spectral->classifier_params.spectral_dc_index   = spectral->spectral_dc_index;
    spectral->spectral_sent_msg                     = 0;
    spectral->classify_scan                         = 0;
    spectral->num_spectral_data                     = 0;
    spectral->eacs_this_scan_spectral_data          = 0;

     if (!p_sops->is_spectral_active(spectral)) {
        p_sops->configure_spectral(spectral, spectral_params);
        p_sops->start_spectral_scan(spectral);
        printk("Enabled spectral scan on channel %d\n", p_sops->get_current_channel(spectral));
    } else {
        printk("Spectral scan is already ACTIVE on channel %d\n", p_sops->get_current_channel(spectral));
    }

    /* get current spectral configuration */
    p_sops->get_spectral_config(spectral, &spectral->params);

    init_upper_lower_flags(spectral);
    init_chan_loading(spectral);

#ifdef SPECTRAL_CLASSIFIER_IN_KERNEL
    init_classifier(sc);
#endif
    return 0;
}

/*
 * Function     : spectral_scan_enable
 * Description  : Enable Spectral Scan
 * Input        : Pointer to Spectral Struct
 * Output       : Success/Failure
 *
 */
int spectral_scan_enable(struct ath_spectral* spectral)
{
    spectral_init_param_defaults(&spectral->params);  
    spectral->params.ss_spectral_pri    = 1;
    return spectral_scan_enable_params(spectral, &spectral->params);
}

/*
 * Function     : spectral_control
 * Description  : Handle Spectral related IOCTL
 * Input        : Pointer to Spectral Struct, standard IOCTL params
 * Output       : Success/Failure
 *
 */
int spectral_control(struct ieee80211com* ic, u_int id, void *indata, u_int32_t insize,
                 void *outdata, u_int32_t *outsize)
{

    int error = 0;
    int temp_debug;
    HAL_SPECTRAL_PARAM sp_out;
    HAL_SPECTRAL_PARAM *sp_in;
    HAL_SPECTRAL_PARAM  *spectralparams;
    SPECTRAL_OPS* p_sops = NULL;
    struct ath_spectral *spectral = ic->ic_spectral;

    p_sops = GET_SPECTRAL_OPS(spectral);


    if (spectral == NULL) {
        error = -EINVAL;
        goto bad;
    }

    if (p_sops == NULL) {
        error = -EINVAL;
        goto bad;
    }

    switch (id) {
        case SPECTRAL_SET_CONFIG:
          {
            if (insize < sizeof(HAL_SPECTRAL_PARAM) || !indata) {
              error = -EINVAL;
              break;
            }

            sp_in = (HAL_SPECTRAL_PARAM *) indata;

            if (sp_in->ss_count != HAL_PHYERR_PARAM_NOVAL ) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_SCAN_COUNT, sp_in->ss_count))
                error = -EINVAL;
            }

            if (sp_in->ss_fft_period != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_FFT_PERIOD, sp_in->ss_fft_period))
                error = -EINVAL;
            }

            if (sp_in->ss_period != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_SCAN_PERIOD, sp_in->ss_period))
                error = -EINVAL;
            }

            if (sp_in->ss_short_report != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_SHORT_REPORT,
                    (u_int32_t)(sp_in->ss_short_report == true?1:0)))
                error = -EINVAL;
            }

            if (sp_in->ss_spectral_pri != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_SPECT_PRI,
                    (u_int32_t)(sp_in->ss_spectral_pri)))
                error = -EINVAL;
            }

            if (sp_in->ss_fft_size != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_FFT_SIZE, sp_in->ss_fft_size))
                error = -EINVAL;
            }

            if (sp_in->ss_gc_ena != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_GC_ENA, sp_in->ss_gc_ena))
                error = -EINVAL;
            }

            if (sp_in->ss_restart_ena != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_RESTART_ENA, sp_in->ss_restart_ena))
                error = -EINVAL;
            }

            if (sp_in->ss_noise_floor_ref != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_NOISE_FLOOR_REF, sp_in->ss_noise_floor_ref))
                error = -EINVAL;
            }

            if (sp_in->ss_init_delay != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_INIT_DELAY, sp_in->ss_init_delay))
                error = -EINVAL;
            }

            if (sp_in->ss_nb_tone_thr != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_NB_TONE_THR, sp_in->ss_nb_tone_thr))
                error = -EINVAL;
            }

            if (sp_in->ss_str_bin_thr != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_STR_BIN_THR, sp_in->ss_str_bin_thr))
                error = -EINVAL;
            }

            if (sp_in->ss_wb_rpt_mode != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_WB_RPT_MODE, sp_in->ss_wb_rpt_mode))
                error = -EINVAL;
            }

            if (sp_in->ss_rssi_rpt_mode != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_RSSI_RPT_MODE, sp_in->ss_rssi_rpt_mode))
                error = -EINVAL;
            }

            if (sp_in->ss_rssi_thr != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_RSSI_THR, sp_in->ss_rssi_thr))
                error = -EINVAL;
            }

            if (sp_in->ss_pwr_format != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_PWR_FORMAT, sp_in->ss_pwr_format))
                error = -EINVAL;
            }

            if (sp_in->ss_rpt_mode != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_RPT_MODE, sp_in->ss_rpt_mode))
                error = -EINVAL;
            }

            if (sp_in->ss_bin_scale != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_BIN_SCALE, sp_in->ss_bin_scale))
                error = -EINVAL;
            }

            if (sp_in->ss_dBm_adj != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_DBM_ADJ, sp_in->ss_dBm_adj))
                error = -EINVAL;
            }

            if (sp_in->ss_chn_mask != HAL_PHYERR_PARAM_NOVAL) {
              if (!spectral_set_thresholds(spectral, SPECTRAL_PARAM_CHN_MASK, sp_in->ss_chn_mask))
                error = -EINVAL;
            }
        }
        break;

    case SPECTRAL_GET_CONFIG:
        {
          if (!outdata || !outsize || (*outsize < sizeof(HAL_SPECTRAL_PARAM))) {
            error = -EINVAL;
            break;
          }

          *outsize = sizeof(HAL_SPECTRAL_PARAM);

          spectral_get_thresholds(spectral, &sp_out);

          spectralparams = (HAL_SPECTRAL_PARAM *) outdata;

          spectralparams->ss_fft_period      = sp_out.ss_fft_period;
          spectralparams->ss_period          = sp_out.ss_period;
          spectralparams->ss_count           = sp_out.ss_count;
          spectralparams->ss_short_report    = sp_out.ss_short_report;
          spectralparams->ss_spectral_pri    = sp_out.ss_spectral_pri;
          spectralparams->ss_fft_size        = sp_out.ss_fft_size;
          spectralparams->ss_gc_ena          = sp_out.ss_gc_ena;
          spectralparams->ss_restart_ena     = sp_out.ss_restart_ena;
          spectralparams->ss_noise_floor_ref = sp_out.ss_noise_floor_ref;
          spectralparams->ss_init_delay      = sp_out.ss_init_delay;
          spectralparams->ss_nb_tone_thr     = sp_out.ss_nb_tone_thr;
          spectralparams->ss_str_bin_thr     = sp_out.ss_str_bin_thr;
          spectralparams->ss_wb_rpt_mode     = sp_out.ss_wb_rpt_mode;
          spectralparams->ss_rssi_rpt_mode   = sp_out.ss_rssi_rpt_mode;
          spectralparams->ss_rssi_thr        = sp_out.ss_rssi_thr;
          spectralparams->ss_pwr_format      = sp_out.ss_pwr_format;
          spectralparams->ss_rpt_mode        = sp_out.ss_rpt_mode;
          spectralparams->ss_bin_scale       = sp_out.ss_bin_scale;
          spectralparams->ss_dBm_adj         = sp_out.ss_dBm_adj;
          spectralparams->ss_chn_mask        = sp_out.ss_chn_mask;
        }
        break;
    case SPECTRAL_IS_ACTIVE:
        {
          if (!outdata || !outsize || *outsize < sizeof(u_int32_t)) {
            error = -EINVAL;
            break;
          }

          *outsize = sizeof(u_int32_t);
          *((u_int32_t *)outdata) = (u_int32_t)p_sops->is_spectral_active(spectral);
        }
        break;
    case SPECTRAL_IS_ENABLED:
        {
          if (!outdata || !outsize || *outsize < sizeof(u_int32_t)) {
            error = -EINVAL;
            break;
          }

          *outsize = sizeof(u_int32_t);
          *((u_int32_t *)outdata) = (u_int32_t)p_sops->is_spectral_enabled(spectral);
        }
        break;

    case SPECTRAL_SET_DEBUG_LEVEL:
        {
          if (insize < sizeof(u_int32_t) || !indata) {
            error = -EINVAL;
            break;
          }
          temp_debug = *(u_int32_t *)indata;
          spectral_debug_level = (ATH_DEBUG_SPECTRAL << temp_debug);
        }
        break;

    case SPECTRAL_ACTIVATE_SCAN:
        {
          spectral->scan_start_tstamp = p_sops->get_tsf64(spectral);
          SPECTRAL_LOCK(spectral);
          start_spectral_scan(spectral);
          spectral->sc_spectral_scan = 1;
          SPECTRAL_UNLOCK(spectral);
        }
        break;

    case SPECTRAL_STOP_SCAN:
        {
          SPECTRAL_LOCK(spectral);
          stop_current_scan(spectral);
          spectral->sc_spectral_scan = 0;
          SPECTRAL_UNLOCK(spectral);
        }
        break;
    case SPECTRAL_ACTIVATE_FULL_SCAN:
        {
          SPECTRAL_LOCK(spectral);
          start_spectral_scan(spectral);
          spectral->sc_spectral_full_scan = 1;
          SPECTRAL_UNLOCK(spectral);
        }
        break;

    case SPECTRAL_STOP_FULL_SCAN:
        {
          SPECTRAL_LOCK(spectral);
          stop_current_scan(spectral);
          spectral->sc_spectral_full_scan = 0;
          SPECTRAL_UNLOCK(spectral);
        }
        break;
    
    case SPECTRAL_GET_CAPABILITY_INFO:
        {

          ASSERT(sizeof(spectral->capability) ==
                 sizeof(struct ath_spectral_caps));

          if (!outdata ||
              !outsize ||
              *outsize < sizeof(struct ath_spectral_caps)) {
            error = -EINVAL;
            break;
          }

          *outsize = sizeof(struct ath_spectral_caps);
          OS_MEMCPY(outdata,
                    &spectral->capability,
                    sizeof(struct ath_spectral_caps));
        }
        break;
    
    case SPECTRAL_GET_DIAG_STATS:
        {
          if (!outdata ||
              !outsize ||
              (*outsize < sizeof(struct spectral_diag_stats))) {
            error = -EINVAL;
            break;
          }

         *outsize = sizeof(struct spectral_diag_stats);
          
          OS_MEMCPY(outdata,
                    &spectral->diag_stats,
                    sizeof(struct spectral_diag_stats));
         }
        break;

    case SPECTRAL_GET_CHAN_WIDTH:
        {
            u_int32_t chan_width = ic->ic_cwm_get_width(ic);
            if (!outdata ||
                !outsize ||
                *outsize < sizeof(chan_width)) {
                error = -EINVAL;
                break;
            }
            *outsize = sizeof(chan_width);
            *((u_int32_t *)outdata) = (u_int32_t)chan_width;
        }
        break;

    default:
        error = -EINVAL;
        break;
    }
bad:
    return error;
}

/*
 * Function     : spectral_set_thresholds
 * Description  : Set SPECTRAL thresholds
 * Input        : Pointer to Spectral Struct, threshold type, value
 * Output       : Success/Failure
 *
 */
int spectral_set_thresholds(struct ath_spectral *spectral,
                            const u_int32_t threshtype,
                            const u_int32_t value)
{
    HAL_SPECTRAL_PARAM params;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);

    if (spectral == NULL) {
        printk(  "%s: sc_spectral is NULL\n", __func__);
        return 0;
    }

    switch (threshtype) {
        case SPECTRAL_PARAM_FFT_PERIOD:
            spectral->params.ss_fft_period = value;
            break;
        case SPECTRAL_PARAM_SCAN_PERIOD:
            spectral->params.ss_period = value;
            break;
        case SPECTRAL_PARAM_SCAN_COUNT:
            spectral->params.ss_count = value;
            break;
        case SPECTRAL_PARAM_SHORT_REPORT:
            spectral->params.ss_short_report = (!!value) ? true:false;
            break;
        case SPECTRAL_PARAM_SPECT_PRI:
            spectral->params.ss_spectral_pri = (!!value)? true:false;
            break;
        case SPECTRAL_PARAM_FFT_SIZE:
            spectral->params.ss_fft_size = value;
            break;
        case SPECTRAL_PARAM_GC_ENA:
            spectral->params.ss_gc_ena = !!value;
            break;
        case SPECTRAL_PARAM_RESTART_ENA:
            spectral->params.ss_restart_ena = !!value;
            break;
        case SPECTRAL_PARAM_NOISE_FLOOR_REF:
            spectral->params.ss_noise_floor_ref = value;
            break;
        case SPECTRAL_PARAM_INIT_DELAY:
            spectral->params.ss_init_delay = value;
            break;
        case SPECTRAL_PARAM_NB_TONE_THR:
            spectral->params.ss_nb_tone_thr = value;
            break;
        case SPECTRAL_PARAM_STR_BIN_THR:
            spectral->params.ss_str_bin_thr = value;
            break;
        case SPECTRAL_PARAM_WB_RPT_MODE:
            spectral->params.ss_wb_rpt_mode = !!value;
            break;
        case SPECTRAL_PARAM_RSSI_RPT_MODE:
            spectral->params.ss_rssi_rpt_mode = !!value;
            break;
        case SPECTRAL_PARAM_RSSI_THR:
            spectral->params.ss_rssi_thr = value;
            break;
        case SPECTRAL_PARAM_PWR_FORMAT:
            spectral->params.ss_pwr_format = !!value;
            break;
        case SPECTRAL_PARAM_RPT_MODE:
            spectral->params.ss_rpt_mode = value;
            break;
        case SPECTRAL_PARAM_BIN_SCALE:
            spectral->params.ss_bin_scale = value;
            break;
        case SPECTRAL_PARAM_DBM_ADJ:
            spectral->params.ss_dBm_adj = !!value;
            break;
        case SPECTRAL_PARAM_CHN_MASK:
            spectral->params.ss_chn_mask = value;
            break;
    }

    p_sops->configure_spectral(spectral, &spectral->params);
    p_sops->get_spectral_config(spectral, &params); /* only to validate the writes */
    //print_spectral_params(&spectral->params);
    return 1;
}

/*
 * Function     : spectral_get_thresholds
 * Description  : Get SPECTRAL thresholds
 * Input        : Pointer to Spectral Struct, threshold type, value
 * Output       : Success/Failure
 *
 */
int spectral_get_thresholds(struct ath_spectral *spectral, HAL_SPECTRAL_PARAM *param)
{
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);
    OS_MEMZERO(param, sizeof (HAL_SPECTRAL_PARAM));
    p_sops->get_spectral_config(spectral, param);
    return 1;
}

static pwr_dBm spectral_get_median_pwr(pwr_dBm arr[], int length)
{
    int i, j;
    pwr_dBm tmp;
    for (i = 1; i < length; i++) {
        j = i;
        while (j > 0 && arr[j - 1] > arr[j]) {
            tmp = arr[j];
            arr[j] = arr[j - 1];
            arr[j - 1] = tmp;
            j--;
        }
    }
    return((length & 1) ? arr[length >> 1] : ((arr[(length >> 1) - 1] + arr[length >> 1]) >> 1));
}

/*
 * Function     : spectral_get_noise_power
 * Description  : Get SPECTRAL noise power
 * Input        : Pointer to Spectral Struct, threshold type, value
 * Output       : Success/Failure
 *
 */
#define MAX_NOISE_PWR_WAIT 20000
int spectral_get_noise_power(struct ath_spectral *spectral,
                             int rptcount,                  /* number of noise pwr reports required */
                             NOISE_PWR_CAL* cal_override,   /* cal debug override - may be NULL */
                             CHAIN_NOISE_PWR_INFO* ctl_c0,  /* reports for chain 0 control */
                             CHAIN_NOISE_PWR_INFO* ctl_c1,  /* reports for chain 1 control */
                             CHAIN_NOISE_PWR_INFO* ctl_c2,  /* reports for chain 2 control */
                             CHAIN_NOISE_PWR_INFO* ext_c0,  /* reports for chain 0 ext */
                             CHAIN_NOISE_PWR_INFO* ext_c1,  /* reports for chain 1 ext */
                             CHAIN_NOISE_PWR_INFO* ext_c2)  /* reports for chain 2 ext */
{

    int ch, i, j, error = 0, idx, val;
    HAL_SPECTRAL_PARAM params;
    CHAIN_NOISE_PWR_INFO* ns;
    int rssi_total[ATH_MAX_ANTENNA*2];
    //pwr_dBm un_cal;
    pwr_dBm *tmp_arr = NULL;
    int16_t nfBuf[ATH_MAX_ANTENNA*2];
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);

    if (rptcount < 1 || rptcount > MAX_NOISE_PWR_REPORTS) {
        error = EINVAL;
        printk(
                         "%s[%d]: error=%d\n", __func__, __LINE__, error);
        return error;
    }

    if (spectral->noise_pwr_reports_reqd || spectral->noise_pwr_reports_recv) {
        error = EINPROGRESS;
        printk(
                         "%s[%d]: error=%d\n", __func__, __LINE__, error);
        return error;
    }

    if(p_sops->is_spectral_active(spectral)) {
        printk("%s[%d]: spectral already active - cancel\n", __func__, __LINE__);
        SPECTRAL_LOCK(spectral);
        p_sops->stop_spectral_scan(spectral);
        spectral->sc_spectral_scan = 0;
        SPECTRAL_UNLOCK(spectral);
    }

    spin_lock(&spectral->noise_pwr_reports_lock);
    spectral->noise_pwr_reports_reqd = rptcount;
    spectral->noise_pwr_reports_recv = 0;
    spectral->noise_pwr_chain_ctl[0] = ctl_c0;
    spectral->noise_pwr_chain_ctl[1] = ctl_c1;
    spectral->noise_pwr_chain_ctl[2] = ctl_c2;
    spectral->noise_pwr_chain_ext[0] = ext_c0;
    spectral->noise_pwr_chain_ext[1] = ext_c1;
    spectral->noise_pwr_chain_ext[2] = ext_c2;
    /* init report counts */
    for(i = 0; i < ATH_MAX_ANTENNA; i++) {
        if (spectral->noise_pwr_chain_ctl[i]) {
            spectral->noise_pwr_chain_ctl[i]->rptcount = 0;
        }
        if (spectral->noise_pwr_chain_ext[i]) {
            spectral->noise_pwr_chain_ext[i]->rptcount = 0;
        }

#ifndef ATH_PERF_PWR_OFFLOAD
        if ((spectral->ath_softc_handle->sc_rx_chainmask & (1 << i)) == 0) {
            spectral->noise_pwr_chain_ctl[i] = spectral->noise_pwr_chain_ext[i] = NULL;
        }
        if (!(spectral->ath_softc_handle->sc_curchan.channel_flags & (CHANNEL_HT40PLUS | CHANNEL_HT40MINUS))) {
            spectral->noise_pwr_chain_ext[i] = NULL;
        }
#else   // ATH_PERF_PWR_OFFLOAD
        not_yet_implemented();
#endif  // ATH_PERF_PWR_OFFLOAD
    }
    spin_unlock(&spectral->noise_pwr_reports_lock);

    /* get the lastest noisefloor reading */
    p_sops->get_chain_noise_floor(spectral, nfBuf);

    /* Get current spectral parameters */
    spectral_get_thresholds(spectral, &params);
    for(i = 0; i < 2; i++) {
        for(ch = 0; ch < ATH_MAX_ANTENNA; ch++) {
            idx = ch + (i*ATH_MAX_ANTENNA);
            if (cal_override && (cal_override->valid_chain_mask & (1 << ch)) != 0) {
                params.ss_nf_cal[idx] = cal_override->chain[ch].cal;
                params.ss_nf_pwr[idx] = cal_override->chain[ch].pwr;
            }
            if (params.ss_nf_cal[idx] & 3) {
                /* adjust dBm by any fractional dBr amount */
                params.ss_nf_pwr[idx] += (params.ss_nf_cal[idx] & 3);
                params.ss_nf_cal[idx] &= ~3;
            }
#ifndef ATH_PERF_PWR_OFFLOAD
            if ((sc->sc_rx_chainmask & (1 << ch)) &&
                idx < sizeof(params.ss_nf_cal)/sizeof(params.ss_nf_cal[0]))
            {
                un_cal = nfBuf[idx] + params.ss_nf_pwr[idx] - params.ss_nf_cal[idx];
                if (!i || (sc->sc_curchan.channel_flags & (CHANNEL_HT40PLUS | CHANNEL_HT40MINUS))) {
                    printk(
                             "eeprom nf chan=%d chain%d%s: un-cal=%4d.%02d dBm   cal=%4d.%02d dBr   pwr=%4d.%02d dBm\n",
                             sc->sc_curchan.channel, ch, i ? "ext" : "ctl",
                             NOISE_PWR_DBM_2_INT(un_cal),                  NOISE_PWR_DBM_2_DEC(un_cal),
                             NOISE_PWR_DBM_2_INT(params.ss_nf_cal[idx]),   NOISE_PWR_DBM_2_DEC(params.ss_nf_cal[idx]),
                             NOISE_PWR_DBM_2_INT(params.ss_nf_pwr[idx]),   NOISE_PWR_DBM_2_DEC(params.ss_nf_pwr[idx]));
                }
            }
#else   // ATH_PERF_PWR_OFFLOAD
            not_yet_implemented();
#endif  // ATH_PERF_PWR_OFFLOAD
        }
    }

    /* override to configure hardware */
    spectral_init_param_defaults(&params);

    params.ss_spectral_pri = 1;

#ifndef ATH_PERF_PWR_OFFLOAD
    spectral->ath_softc_handle->sc_scanning = 1; // dummy a scan to prevent ath_calibrate
#endif // ATH_PERF_PWR_OFFLOAD

    SPECTRAL_LOCK(spectral);
    spectral->scan_start_tstamp = p_sops->get_tsf64(spectral);
    error = spectral_scan_enable_params(spectral, &params);
    spectral->sc_spectral_scan = 1;
    SPECTRAL_UNLOCK(spectral);

    if (error) {
        SPECTRAL_LOCK(spectral);
        spectral->sc_spectral_scan = 0;
        SPECTRAL_UNLOCK(spectral);
        printk("%s[%d]: error=%d\n", __func__, __LINE__, error);
    }
    else {
        /* wait for ss to complete */
        for (i = 0; i < MAX_NOISE_PWR_WAIT; i++) {
            if (!p_sops->is_spectral_active(spectral)) {
                printk("%s[%d]: noise pwr done - reports_recv=%d\n", __func__, __LINE__, spectral->noise_pwr_reports_recv);
                break;
            }
            OS_DELAY(100);
        }
        /* did we timeout ? */
        if (i == MAX_NOISE_PWR_WAIT) {
            SPECTRAL_LOCK(spectral);
            stop_current_scan(spectral);
            spectral->sc_spectral_scan = 0;
            SPECTRAL_UNLOCK(spectral);
            error = EBUSY;
        }
    }

    spin_lock(&spectral->noise_pwr_reports_lock);

    if (!error && spectral->noise_pwr_reports_recv < spectral->noise_pwr_reports_reqd) {
        error = EAGAIN;
        printk("%s[%d]: noise pwr done but only %d/%d reports_recv\n", __func__, __LINE__, spectral->noise_pwr_reports_recv,
                         spectral->noise_pwr_reports_reqd);
    }

#if 0
    if (!error) {
        tmp_arr = OS_MALLOC(sc->sc_osdev, spectral->noise_pwr_reports_recv*sizeof(pwr_dBm), GFP_KERNEL);
        if (tmp_arr == NULL) {
            error = ENOMEM;
            printk("%s[%d]: error=%d\n", __func__, __LINE__, error);
        }
    }
#endif

    /*
     * for both ctl and ext:
     * - fill in factory nf pwr from eeprom and un-cal nf
     * - add nf to rssi reports to get pwr
     * - calc mean pwr
    */
    if (!error) {
        OS_MEMZERO(rssi_total, sizeof(rssi_total));
        for(i = 0; i < 2; i++) {
            for(ch = 0; ch < ATH_MAX_ANTENNA; ch++) {
                ns = (i == 0) ? spectral->noise_pwr_chain_ctl[ch] : spectral->noise_pwr_chain_ext[ch];
                idx = ch + (i*ATH_MAX_ANTENNA);
                if (ns && ns->rptcount) {
                    ns->un_cal_nf = nfBuf[idx] + params.ss_nf_pwr[idx] - params.ss_nf_cal[idx];
                    ns->factory_cal_nf = params.ss_nf_pwr[idx];
                    rssi_total[idx] = 0;
                    for(j = 0; j < ns->rptcount; j++) {
                        rssi_total[idx] += ns->pwr[j];
                        val = INT_2_NOISE_PWR_DBM(ns->pwr[j] + (ns->factory_cal_nf >> 2) + NOISE_PWR_DATA_OFFSET)
                              + (ns->factory_cal_nf & 3);
                        // clamp under/overflows to range
                        val = (val < -128) ? -128 : ((val > 127) ? 127 : val);
                        ns->pwr[j] = val;
                    }
                    OS_MEMCPY(tmp_arr, ns->pwr, ns->rptcount);
                    ns->median_pwr = spectral_get_median_pwr(tmp_arr, ns->rptcount);
                    rssi_total[idx] = (rssi_total[idx] << 2) / ns->rptcount;
                }
            }
        }
    }

    /* reset/cleanup */
    OS_MEMZERO(spectral->noise_pwr_chain_ctl, sizeof(spectral->noise_pwr_chain_ctl));
    OS_MEMZERO(spectral->noise_pwr_chain_ext, sizeof(spectral->noise_pwr_chain_ext));
    spectral->noise_pwr_reports_reqd = spectral->noise_pwr_reports_recv = 0;

    spin_unlock(&spectral->noise_pwr_reports_lock);

    if (!error) {
        printk("%s", "rssi ave: ");
        for(i = 0; i < ATH_MAX_ANTENNA; i++) {
            printk(  "%3d.%02d ",
                (rssi_total[i] < 0) ? ((rssi_total[i]+3)>>2) : rssi_total[i]>>2,
                (rssi_total[i]&3)*25);
        }
        printk(  "%s", " |");
        for(i = ATH_MAX_ANTENNA; i < ATH_MAX_ANTENNA*2; i++) {
            printk(  "%3d.%02d ",
                (rssi_total[i] < 0) ? ((rssi_total[i]+3)>>2) : rssi_total[i]>>2,
                (rssi_total[i]&3)*25);
        }
        printk(  "%s", "\n");
        printk(  "Temp data: %d [0x%x]\n",
                         params.ss_nf_temp_data, params.ss_nf_temp_data);
    }

    if (tmp_arr != NULL) {
        OS_FREE(tmp_arr);
    }

#ifndef ATH_PERF_PWR_OFFLOAD
    spectral->ath_softc_handle->sc_scanning = 0;
#endif // ATH_PERF_PWR_OFFLOAD

    if (error == EBUSY) {
        /* force a full reset to restore normal operation */
        printk(  "%s[%d]: TIME_OUT: force reset on exit\n", __func__, __LINE__);
        p_sops->reset_hw(spectral);
    }

    if (error) {
      printk(  "%s[%d]: error=%d\n", __func__, __LINE__, error);
    }
    return error;
}


/*
 * Function     : spectral_send_eacs_msg
 * Description  : Send EACS Message
 * Input        : Pointer to Spectral Struct
 * Output       : Success/Failure
 *
 */

void spectral_send_eacs_msg(struct ath_spectral* spectral)
{
#ifdef SPECTRAL_USE_NETLINK_SOCKETS
    SPECTRAL_SAMP_MSG *msg = NULL;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);
    spectral_prep_skb(spectral);
    if (spectral->spectral_skb != NULL) {
         spectral->spectral_nlh = (struct nlmsghdr*)spectral->spectral_skb->data;
         msg = (SPECTRAL_SAMP_MSG*) NLMSG_DATA(spectral->spectral_nlh);
         msg->cw_interferer = 1;
         p_sops->get_mac_address(spectral, msg->macaddr);
         spectral_bcast_msg(spectral);
    }
#endif
}

#define MIN_SPECTRAL_DATA_SIZE (10)

/*
 * Function     : is_spectral_phyerr
 * Description  : Check for PHY ERR
 * Input        : Pointer to Spectral Struct
 * Output       : Success/Failure
 *
 */
int is_spectral_phyerr(struct ath_spectral* spectral,
                      struct ath_buf *bf,
                      struct ath_rx_status *rxs)
{

    u_int8_t pulse_bw_info = 0;
    u_int8_t *byte_ptr;
    u_int8_t last_byte_0;
    u_int8_t last_byte_1;
    u_int8_t last_byte_2;
    u_int8_t last_byte_3;
    u_int8_t secondlast_byte_0;
    u_int8_t secondlast_byte_1;
    u_int8_t secondlast_byte_2;
    u_int8_t secondlast_byte_3;
    u_int8_t *dataPtr;
    u_int16_t datalen;

    u_int32_t *last_word_ptr;
    u_int32_t *secondlast_word_ptr;

    if ((!(rxs->rs_phyerr == HAL_PHYERR_RADAR)) &&
           (!(rxs->rs_phyerr == HAL_PHYERR_FALSE_RADAR_EXT)) &&
           (!(rxs->rs_phyerr == HAL_PHYERR_SPECTRAL))) {
        return AH_FALSE;
    }

    if (!rxs->rs_datalen) {
        return AH_FALSE;
    }

    if (!spectral) {
        return AH_FALSE;
    }

    // If spectral scan is not started, just drop this packet.
    if (!(spectral->sc_spectral_scan || spectral->sc_spectral_full_scan)) {
        return AH_FALSE;
    }

    //FIXME. When datalen is less than 2 words, following process will underrun.
    if ( rxs->rs_datalen < MIN_SPECTRAL_DATA_SIZE ) {
        return AH_FALSE;
    }

    /* This is the case for Kiwi and later chips, where we do not need
     * the bandwidth info to make sure it is a spectral data */
    if( rxs->rs_phyerr == HAL_PHYERR_SPECTRAL ) return AH_TRUE;

    datalen = rxs->rs_datalen;
    dataPtr = bf->bf_vdata;
    last_word_ptr = (u_int32_t *)(dataPtr + datalen - (datalen % 4));
    secondlast_word_ptr = last_word_ptr - 1;

    byte_ptr = (u_int8_t *)last_word_ptr;
    last_byte_0 = (*(byte_ptr) & 0xff);
    last_byte_1 = (*(byte_ptr + 1) & 0xff);
    last_byte_2 = (*(byte_ptr + 2) & 0xff);
    last_byte_3 = (*(byte_ptr + 3) & 0xff);

    byte_ptr = (u_int8_t*)secondlast_word_ptr;
    secondlast_byte_0 = (*(byte_ptr) & 0xff);
    secondlast_byte_1 = (*(byte_ptr + 1) & 0xff);
    secondlast_byte_2 = (*(byte_ptr + 2) & 0xff);
    secondlast_byte_3 = (*(byte_ptr + 3) & 0xff);

    /* extract the bwinfo */
    switch ((datalen & 0x3))  {
        case 0:
            pulse_bw_info = secondlast_byte_3;
            break;
        case 1:
            pulse_bw_info = last_byte_0;
            break;
        case 2:
            pulse_bw_info = last_byte_1;
            break;
        case 3:
            pulse_bw_info = last_byte_2;
            break;
    }

    /* the 5lsbs contain information regarding the source of data */
    pulse_bw_info &= 0x10; //SPECTRAL_SCAN_BITMASK;
    return (pulse_bw_info ? AH_TRUE:AH_FALSE);
}



/*
 * Function     : get_nfc_ext_rssi
 * Description  : Get Noisefloor compensated control channel RSSI
 * Input        : Pointer to Spectral Struct, rssi, CTL noisefloor
 * Output       : Noisefloor
 *
 */
int8_t get_nfc_ctl_rssi(struct ath_spectral* spectral, int8_t rssi, int8_t *ctl_nf)
{
    int8_t temp;
    int16_t nf = -110;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);
    *ctl_nf = -110;

    nf = p_sops->get_ctl_noisefloor(spectral);
    temp = 110 + (nf) + (rssi);
    *ctl_nf = nf;
    return temp;
}

/*
 * Function     : get_nfc_ext_rssi
 * Description  : Get Noisefloor compensated extension channel RSSI
 * Input        : Pointer to Spectral Struct, rssi, CTL noisefloor
 * Output       : Noisefloor
 *
 */
int8_t get_nfc_ext_rssi(struct ath_spectral* spectral, int8_t rssi, int8_t *ext_nf)
{
    int8_t temp;
    int16_t nf = -110;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);
    *ext_nf = -110;
    nf = p_sops->get_ext_noisefloor(spectral);
    temp = 110 + (nf) + (rssi);
    *ext_nf = nf;
    return temp;
}

/*
 * Function     : update_eacs_avg_rssi
 * Description  : Update average RSSI
 * Input        : Pointer to Spectral Struct
 * Output       : Noisefloor
 *
 */
static void update_eacs_avg_rssi(struct ath_spectral* spectral, int8_t nfc_ctl_rssi, int8_t nfc_ext_rssi)
{
    int temp=0;

    if(spectral->sc_spectral_20_40_mode) {
        // HT40 mode
        temp = (spectral->ext_eacs_avg_rssi * (spectral->ext_eacs_spectral_reports));
        temp += nfc_ext_rssi;
        spectral->ext_eacs_spectral_reports++;
        spectral->ext_eacs_avg_rssi = (temp / spectral->ext_eacs_spectral_reports);
    }


    temp = (spectral->ctl_eacs_avg_rssi * (spectral->ctl_eacs_spectral_reports));
    temp += nfc_ctl_rssi;

    spectral->ctl_eacs_spectral_reports++;
    spectral->ctl_eacs_avg_rssi = (temp / spectral->ctl_eacs_spectral_reports);
}

/*
 * Function     : update_eacs_counters
 * Description  : Update EACS counters
 * Input        : Pointer to Spectral Struct
 * Output       : Noisefloor
 *
 */
void update_eacs_counters(struct ath_spectral* spectral, int8_t nfc_ctl_rssi, int8_t nfc_ext_rssi)
{
    update_eacs_thresholds(spectral);
    update_eacs_avg_rssi(spectral, nfc_ctl_rssi, nfc_ext_rssi);

    if (spectral->sc_spectral_20_40_mode) {
        // HT40 mode
        if (nfc_ext_rssi > spectral->ext_eacs_rssi_thresh){
            spectral->ext_eacs_interf_count++;
        }
        spectral->ext_eacs_duty_cycle=((spectral->ext_eacs_interf_count * 100)/spectral->eacs_this_scan_spectral_data);
    }

    if (nfc_ctl_rssi > spectral->ctl_eacs_rssi_thresh){
        spectral->ctl_eacs_interf_count++;
    }

    spectral->ctl_eacs_duty_cycle=((spectral->ctl_eacs_interf_count * 100)/spectral->eacs_this_scan_spectral_data);
}

/*
 * Function     : get_eacs_control_duty_cycle
 * Description  : Get EACS control duty cycle
 * Input        : Pointer to Spectral Struct
 * Output       : Duty Cycle
 *
 */
int get_eacs_control_duty_cycle(struct ath_spectral* spectral)
{
    return spectral->ctl_eacs_duty_cycle;
}

/* Function     : get_eacs_extension_duty_cycle
 * Description  : Get EACS extension duty cycle
 * Input        : Pointer to Spectral Struct
 * Output       : Duty Cycle
 *
 */
int get_eacs_extension_duty_cycle(struct ath_spectral* spectral)
{
    return spectral->ext_eacs_duty_cycle;
}

/*
 * Function     : update EACS Thresholds
 * Description  : Updates EACS Thresholds
 * Input        : Pointer to Spectral Struct
 * Output       : void
 *
 */
void update_eacs_thresholds(struct ath_spectral* spectral)
{
    int16_t nf = -110;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);

    nf = p_sops->get_ctl_noisefloor(spectral);

    spectral->ctl_eacs_rssi_thresh = nf + 10;

    if (spectral->sc_spectral_20_40_mode) {
        nf = p_sops->get_ext_noisefloor(spectral);
        spectral->ext_eacs_rssi_thresh = nf + 10;
    }

    spectral->ctl_eacs_rssi_thresh = 32;
    spectral->ext_eacs_rssi_thresh = 32;
}

/*
 * Dummy Functions : 
 * These functions are there, to avoid any crashes, due to invoking
 * of spectral functions before they are registered
 *
 */
static u_int64_t null_get_tsf64(void* arg)
{
    spectral_ops_not_registered("get_tsf64");
    return 0;
}

static u_int32_t null_get_capability(void* arg, HAL_CAPABILITY_TYPE type)
{
    /*
     * TODO : We should have conditional compilation to get the capability
     *      : We have not yet attahced ATH layer here, so there is no 
     *      : way to check the HAL capbalities
     */
    spectral_ops_not_registered("get_capability");

    /* TODO : For the time being, we are returning TRUE */
    return AH_TRUE;
}

static u_int32_t null_set_rxfilter(void* arg, int rxfilter)
{
    spectral_ops_not_registered("set_rxfilter");
    return 1;
}

static u_int32_t null_get_rxfilter(void* arg)
{
    spectral_ops_not_registered("get_rxfilter");
    return 0;
}

static u_int32_t null_is_spectral_active(void* arg)
{
    spectral_ops_not_registered("is_spectral_active");
    return 1;
}

static u_int32_t null_is_spectral_enabled(void* arg)
{
    spectral_ops_not_registered("is_spectral_enabled");
    return 1;
}

static u_int32_t null_start_spectral_scan(void* arg)
{
    spectral_ops_not_registered("start_spectral_scan");
    return 1;
}

static u_int32_t null_stop_spectral_scan(void* arg)
{
    spectral_ops_not_registered("stop_spectral_scan");
    return 1;
}

static u_int32_t null_get_extension_channel(void* arg)
{
    spectral_ops_not_registered("get_extension_channel");
    return 1;
}

static int8_t null_get_ctl_noisefloor(void* arg)
{
    spectral_ops_not_registered("get_ctl_noisefloor");
    return 1;
}

static int8_t null_get_ext_noisefloor(void* arg)
{
    spectral_ops_not_registered("get_ext_noisefloor");
    return 0;
}

static u_int32_t null_configure_spectral(void* arg, HAL_SPECTRAL_PARAM* params)
{
    spectral_ops_not_registered("configure_spectral");
    return 0;
}

static u_int32_t null_get_spectral_config(void* arg, HAL_SPECTRAL_PARAM* params)
{
    spectral_ops_not_registered("get_spectral_config");
    return 0;
}

static u_int32_t null_get_ent_spectral_mask(void* arg)
{
    spectral_ops_not_registered("get_ent_spectral_mask");
    return 0;
}

static u_int32_t null_get_mac_address(void* arg, char* addr)
{
    spectral_ops_not_registered("get_mac_address");
    return 0;
}

static u_int32_t null_get_current_channel(void* arg)
{
    spectral_ops_not_registered("get_current_channel");
    return 0;
}

static u_int32_t null_reset_hw(void* arg)
{
    spectral_ops_not_registered("get_current_channel");
    return 0;
}

static u_int32_t null_get_chain_noise_floor(void* arg, int16_t* nfBuf)
{
    spectral_ops_not_registered("get_chain_noise_floor");
    return 0;
}

/*
 * Function     : test_spectral_ops
 * Description  : Checks the registred Spectral functions
 * Input        : Pointer to Spectral Struct
 * Output       : Void
 *
 */
void test_spectral_ops(struct ath_spectral* spectral)
{
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);
    printk("Testing Spectral Ops\n");
    printk("TSF         = %llu\n", p_sops->get_tsf64(spectral));
    printk("RXFILT      = %d\n", p_sops->get_rxfilter(spectral));
    printk("Spectral    = %s\n", p_sops->is_spectral_enabled(spectral)?"Enabled":"Disabled");
    printk("Spectral    = %s\n", p_sops->is_spectral_active(spectral)?"Active":"Inactive");
    printk("CTL NF      = %d\n", p_sops->get_ctl_noisefloor(spectral));
    printk("EXT NF      = %d\n", p_sops->get_ext_noisefloor(spectral));
    printk("CUR CH      = %d\n", p_sops->get_current_channel(spectral));
}

/*
 * Function     : spectral_register_funcs
 * Description  : Registers Spectral related functions
 * Input        : Pointer to Spectral Struct, Pointer to Spectral function tabel
 * Output       : Void
 *
 */
void spectral_register_funcs(void* arg, SPECTRAL_OPS* p)
{
    struct ath_spectral *spectral = ((struct ieee80211com*)(arg))->ic_spectral;
    SPECTRAL_OPS* p_sops = GET_SPECTRAL_OPS(spectral);

    p_sops->get_tsf64               = p->get_tsf64;
    p_sops->get_capability          = p->get_capability;
    p_sops->set_rxfilter            = p->set_rxfilter;
    p_sops->get_rxfilter            = p->get_rxfilter;
    p_sops->is_spectral_enabled     = p->is_spectral_enabled;
    p_sops->is_spectral_active      = p->is_spectral_active;
    p_sops->start_spectral_scan     = p->start_spectral_scan;
    p_sops->stop_spectral_scan      = p->stop_spectral_scan;
    p_sops->get_extension_channel   = p->get_extension_channel;
    p_sops->get_ctl_noisefloor      = p->get_ctl_noisefloor;
    p_sops->get_ext_noisefloor      = p->get_ext_noisefloor;
    p_sops->configure_spectral      = p->configure_spectral;
    p_sops->get_spectral_config     = p->get_spectral_config;
    p_sops->get_ent_spectral_mask   = p->get_ent_spectral_mask;
    p_sops->get_mac_address         = p->get_mac_address;
    p_sops->get_current_channel     = p->get_current_channel;
    p_sops->reset_hw                = p->reset_hw;
    p_sops->get_chain_noise_floor   = p->get_chain_noise_floor;

    return;

}OS_EXPORT_SYMBOL(spectral_register_funcs);

#ifndef __NetBSD__
#ifdef __linux__
/*
 * Linux Module glue.
 */
static char *dev_info = "ath_spectral";

MODULE_AUTHOR("Atheros Communications, Inc.");
MODULE_DESCRIPTION("SPECTRAL Support for Atheros 802.11 wireless LAN cards.");
MODULE_SUPPORTED_DEVICE("Atheros WLAN cards");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Proprietary");
#endif



static int __init
init_ath_spectral(void)
{
    printk (KERN_INFO "%s: Version 2.0.0\n"
        "Copyright (c) 2005-2009 Atheros Communications, Inc. "
        "All Rights Reserved\n",dev_info);
        printk(KERN_INFO "SPECTRAL module built on %s %s\n", __DATE__, __TIME__);
    return 0;
}
module_init(init_ath_spectral);

static void __exit
exit_ath_spectral(void)
{
    printk (KERN_INFO "%s: driver unloaded\n", dev_info);
}
module_exit(exit_ath_spectral);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,52))
MODULE_PARM(nfrssi, "i");
MODULE_PARM(nobeacons, "i");
MODULE_PARM(maxholdintvl, "i");
MODULE_PARM_DESC(nfrssi, "Adjust for noise floor or not");
MODULE_PARM_DESC(nobeacons, "Should the AP beacon during spectral scan?");
MODULE_PARM_DESC(maxholdintvl, "Should max hold be used and what is the max hold interval?");
#else
#include <linux/moduleparam.h>
module_param(nfrssi, int, 0600);
module_param(nobeacons, int, 0600);
module_param(maxholdintvl, int, 0600);
#endif

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

EXPORT_SYMBOL(spectral_attach);
EXPORT_SYMBOL(spectral_detach);
EXPORT_SYMBOL(spectral_scan_enable);
EXPORT_SYMBOL(ath_process_spectraldata);
EXPORT_SYMBOL(spectral_control);
EXPORT_SYMBOL(spectral_set_thresholds);
EXPORT_SYMBOL(is_spectral_phyerr);
EXPORT_SYMBOL(start_spectral_scan);
EXPORT_SYMBOL(stop_current_scan);
EXPORT_SYMBOL(get_eacs_extension_duty_cycle);
EXPORT_SYMBOL(get_eacs_control_duty_cycle);
EXPORT_SYMBOL(spectral_send_eacs_msg);
EXPORT_SYMBOL(spectral_scan_enable_params);

#endif /* __linux__ */
#endif /* __netbsd__ */

#undef printk
#undef SPECTRAL_MIN
#undef SPECTRAL_MAX
#undef SPECTRAL_DIFF

#endif
