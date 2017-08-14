/* IEEE 802.11 Definitions
 * 
 * Inspired and with some details copied from the Linux kernel.
 * https://github.com/torvalds/linux/blob/master/include/linux/ieee80211.h
 * 
 *  Copyright (C) 2017 Mikkel Kroman <mk@maero.dk>
 */

#ifndef IEEE80211_H
#define IEEE80211_H

#include <esp_types.h>

/*
 * DS bit usage
 *
 * TA = transmitter address
 * RA = receiver address
 * DA = destination address
 * SA = source address
 *
 * ToDS    FromDS  A1(RA)  A2(TA)  A3      A4      Use
 * -----------------------------------------------------------------
 *  0       0       DA      SA      BSSID   -       IBSS/DLS
 *  0       1       DA      BSSID   SA      -       AP -> STA
 *  1       0       BSSID   SA      DA      -       AP <- STA
 *  1       1       RA      TA      DA      SA      unspecified (WDS)
 */

#define IEEE80211_FCTL_FTYPE    0x000c
#define IEEE80211_FCTL_STYPE    0x00f0
#define IEEE80211_FCTL_TODS     0x0100
#define IEEE80211_FCTL_FROMDS   0x0200

#define IEEE80211_FTYPE_MGMT    0x0000

#define IEEE80211_STYPE_ASSOC_REQ    0x0000
#define IEEE80211_STYPE_ASSOC_RESP   0x0010
#define IEEE80211_STYPE_REASSOC_REQ  0x0020
#define IEEE80211_STYPE_REASSOC_RESP 0x0030
#define IEEE80211_STYPE_PROBE_REQ    0x0040
#define IEEE80211_STYPE_PROBE_RESP   0x0050
#define IEEE80211_STYPE_BEACON       0x0080
#define IEEE80211_STYPE_ATIM         0x0090
#define IEEE80211_STYPE_DISASSOC     0x00A0
#define IEEE80211_STYPE_AUTH         0x00B0
#define IEEE80211_STYPE_DEAUTH       0x00C0
#define IEEE80211_STYPE_ACTION       0x00D0

namespace ieee80211 {

class Header {
  public:
    /**
     * @brief checks if IEEE80211_FTYPE_MGMT and IEEE80211_STYPE_DEAUTH is set
     */
    inline int is_deauth()
    {
      return (frame_control & (IEEE80211_FCTL_FTYPE | IEEE80211_FCTL_STYPE)) == 
        (IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_DEAUTH);
    }

    /**
     * @brief check if IEEE80211_FCTL_FROMDS is set
     */
    inline bool has_fromds()
    {
        return (frame_control & IEEE80211_FCTL_FROMDS) != 0;
    }

    /**
     * @brief check if IEEE80211_FCTL_TODS is set
     */
    inline bool has_tods()
    {
        return (frame_control & IEEE80211_FCTL_TODS) != 0;
    }
  
public:
  uint16_t frame_control;
  uint16_t duration_id;
  uint8_t addr1[6];
  uint8_t addr2[6];
  uint8_t addr3[6];
  uint16_t seq_ctrl;
  uint8_t addr4[6];
  union {
      struct {
          uint16_t reason_code;
      } __attribute__((packed)) deauth;
  } u;
} __attribute__((packed, aligned(2)));

}

#endif
