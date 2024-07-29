#include <zboss_api.h>

// ZCL spec 4.13.1.3
#define ZB_ZCL_CLUSTER_ID_AIQ (0xfc81) // leverage heiman's cluster id (0xfc81) from zigbee2mqtt

// ZCL spec 4.13.1.1
#define ZB_ZCL_AIQ_CLUSTER_REVISION_DEFAULT ((zb_uint16_t)0x0001u)
#define ZB_ZCL_ATTR_AIQ_VALUE_UNKNOWN ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_UNKNOWN
#define ZCL_AIQ_MEASURED_VALUE_MULTIPLIER 1

/*! @brief CurrentTemperature, ZCL spec 3.4.2.2.1 */
#define ZB_ZCL_ATTR_AIQ_IAQ_ID (0xf005)
#define ZB_ZCL_ATTR_AIQ_VOC_ID (0xf004)

#define ZB_ZCL_CLUSTER_ID_AIQ_SERVER_ROLE_INIT zb_zcl_aiq_init_server
#define ZB_ZCL_CLUSTER_ID_AIQ_CLIENT_ROLE_INIT (NULL)

typedef void * zb_voidp_t;
#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_AIQ_IAQ_ID(data_ptr) \
  { \
    ZB_ZCL_ATTR_AIQ_IAQ_ID, \
    ZB_ZCL_ATTR_TYPE_U16, \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING, \
    (ZB_UINT16_MAX), \
    (zb_voidp_t) data_ptr \
  }

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_AIQ_VOC_ID(data_ptr) \
  { \
    ZB_ZCL_ATTR_AIQ_VOC_ID, \
    ZB_ZCL_ATTR_TYPE_U16, \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING, \
    (ZB_UINT16_MAX), \
    (zb_voidp_t) data_ptr \
  }

#define ZB_SET_ATTR_DESCR_WITH_ZB_ZCL_ATTR_AIQ_BAT_ID(data_ptr) \
  { \
    ZB_ZCL_ATTR_AIQ_BAT_ID, \
    ZB_ZCL_ATTR_TYPE_U16, \
    ZB_ZCL_ATTR_ACCESS_READ_ONLY | ZB_ZCL_ATTR_ACCESS_REPORTING, \
    (ZB_UINT16_MAX), \
    (zb_voidp_t) data_ptr \
  }

#define ZB_ZCL_DECLARE_AIQ_ATTRIB_LIST(attr_list, iaq, voc) \
  ZB_ZCL_START_DECLARE_ATTRIB_LIST_CLUSTER_REVISION(attr_list, ZB_ZCL_AIQ) \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_AIQ_IAQ_ID, (iaq))          \
  ZB_ZCL_SET_ATTR_DESC(ZB_ZCL_ATTR_AIQ_VOC_ID, (voc))          \
  ZB_ZCL_FINISH_DECLARE_ATTRIB_LIST

void zb_zcl_aiq_init_server();