/********************************** (C) COPYRIGHT *******************************
* File Name          : app.c
* Author             : WCH
* Version            : V1.0
* Date               : 2019/12/15
* Description 
*******************************************************************************/

#include "mesh_error.h"
#include "CH57x_common.h"

#define BT_DBG_ENABLED 1
#define CONFIG_BLE_MESH_STACK_TRACE_LEVEL LOG_LEVEL_DEBUG

#include "cfg_srv.h"
#include "cfg_cli.h"

#include "health_srv.h"
#include "health_cli.h"

#include "mesh_trace.h"
#include "mesh_types.h"
#include "mesh_access.h"
#include "mesh_main.h"
#include "mesh_cfg_pub.h"
#include "HAL.h"
#include "dbg.h"

#define LED_PIN	GPIO_Pin_16

static struct bt_mesh_cfg_srv cfg_srv = {
    .relay = BLE_MESH_RELAY_ENABLED,
    .beacon = BLE_MESH_BEACON_ENABLED,
    .frnd = BLE_MESH_FRIEND_ENABLED,
    .gatt_proxy = BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
    .default_ttl = 7,

    /* 3 transmissions with 20ms interval */
    .net_transmit = BLE_MESH_TRANSMIT(4, 20),
    .relay_retransmit = BLE_MESH_TRANSMIT(4, 20),
};

static struct bt_mesh_health_srv health_srv;

BLE_MESH_HEALTH_PUB_DEFINE(health_pub, 8);

static const u8_t net_key[16] 	= {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};
static const u8_t dev_key[16] 	= {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
};

static const u16_t net_idx 		=	0x0000;
static const u32_t iv_index		=	0x00000000;
static u16_t addr 				= 	0x0005;
static u8_t flags				=	0x00;

static struct bt_mesh_model root_models[] = {
    BLE_MESH_MODEL_CFG_SRV(&cfg_srv),
    BLE_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),
};

static struct bt_mesh_elem elements[] = {
    BLE_MESH_ELEM(0, root_models, BLE_MESH_MODEL_NONE),
};

static const struct bt_mesh_comp comp = {
    .cid = 0x0739,
    .elem = elements,
    .elem_count = ARRAY_SIZE(elements),
};

static const uint8_t dev_uuid[16] = {0x01};

static void complete(u16_t net_idx, u16_t addr, u8_t flags, u32_t iv_index)
{
	BT_DBG("");
}

static const struct bt_mesh_prov prov = {
    .uuid = dev_uuid,
    .output_size = 0,
    .output_actions = BLE_MESH_NO_OUTPUT,
	.complete = complete,
	.unprovisioned_beacon = NULL,
};

u32 MEM_BUF[BLE_MEMHEAP_SIZE / 4];
static u8_t MESH_MEM[3048];

u8C MacAddr[] = {0x12, 0x23, 0x34, 0x45, 0x56, 0x78};

#define LOCAL_ADV_NAME	"WCH-BLE-MESH"

extern const ble_mesh_cfg_t mesh_cfg;
extern const struct device dev;

static void blemesh_on_sync(void)
{
    int err;
	mem_info_t info;
	
	info.base_addr = MESH_MEM;
	info.mem_len = ARRAY_SIZE(MESH_MEM);
	
	err = bt_mesh_cfg_set(&mesh_cfg, &dev, NULL, &info);
	if (err)
	{
		BT_ERR("Unable set configuration (err:%d)", err);
		return;
	}

    err = bt_mesh_init(&prov, &comp, NULL);
    if (err)
    {
        BT_ERR("Initializing mesh failed (err %d)", err);
        return;
    }
	
	BT_DBG("Bluetooth initialized");

    if (IS_ENABLED(CONFIG_BLE_MESH_SETTINGS))
    {
        settings_load();
    }
	
    if (bt_mesh_is_provisioned())
    {
        BT_INFO("Mesh network restored from flash");
    }
    else
    {
        err = bt_mesh_provision(net_key, net_idx, flags, iv_index, addr, dev_key);
		if (err)
		{
			BT_ERR("Self Privisioning (err %d)", err);
			return;
		}
    }

    BT_DBG("Mesh initialized");
}

int main()
{
#if defined(DEBUG) || defined(NOLIB_DEBUG)
    GPIOA_SetBits(GPIO_Pin_9);
    GPIOA_ModeCfg(GPIO_Pin_9, GPIO_ModeOut_PP_5mA);
    GPIOA_ModeCfg(GPIO_Pin_8, GPIO_ModeIN_PU);
    UART1_DefInit();
    UART1_BaudRateCfg(115200);
    R8_UART1_FCR = (2 << 6) | RB_FCR_TX_FIFO_CLR | RB_FCR_RX_FIFO_CLR | RB_FCR_FIFO_EN;
    R8_UART1_LCR = RB_LCR_WORD_SZ;
    R8_UART1_IER = RB_IER_TXD_EN;
    R8_UART1_DIV = 1;
    UART1_ByteTrigCfg(UART_1BYTE_TRIG);
#endif

	GPIOB_ModeCfg(LED_PIN, GPIO_ModeOut_PP_5mA);

#if 0
	FlashBlockErase(CONFIG_MESH_NVS_ADDR_DEF + 512 * 0);
	FlashBlockErase(CONFIG_MESH_NVS_ADDR_DEF + 512 * 1);
	FlashBlockErase(CONFIG_MESH_NVS_ADDR_DEF + 512 * 2);
	FlashBlockErase(CONFIG_MESH_NVS_ADDR_DEF + 512 * 3);
	
	for(uint16 i=0;i<2048;i++)
	{
		if(i%512 == 0)
		PRINT("\nidx :%u\n",i/512);
		
		PRINT("%02x ",*(char *)(CONFIG_MESH_NVS_ADDR_DEF+i));
	}
	
	while(1);
#endif

	CH57X_BLEInit();
	HAL_Init();
	
	bt_mesh_lib_init();
	
#if 0
	dbg_init();
#endif

	blemesh_on_sync();
	
	while(1)
	{
		TMOS_SystemProcess();
	}
}
