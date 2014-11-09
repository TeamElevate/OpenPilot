#include <openpilot.h>
#include <pios_board_info.h>
#include "taskinfo.h"
#include "pios_i2c_uavtalk.h"

//#include "ledexample.h"
#define UPDATE_PERIOD 500
#define MAX_I2C_RX_BUF_LEN 100 //bytes
#define MAX_I2C_RX_DELAY   50 //ms

//Private Type
typedef struct {
	//Task handles
	xTaskHandle i2cRxTaskHandle;
	xTaskHandle i2cTxTaskHandle;

	//UAVTalk connection to abstract UAVObjects over i2c

	//Queue
	//xQueueHandle i2cEventQueue;

	//Raw buffer?
	uint8_t i2c_rx_buf[MAX_I2C_RX_BUF_LEN];

	//Stats
	uint32_t i2cTxRetries;
} I2CBridgeData;

static UAVTalkConnection i2cUAVTalkCon;

static int32_t sendHandler(uint8_t *buf, int32_t length) {
	return PIOS_I2C_UAVTALK_Write(buf, length);
}

int32_t rx_completed_objects = 0;
int32_t bad_checksums = 0;
int32_t rx_processed_bytes = 0;
UAVTalkRxState old_rx_state = UAVTALK_STATE_COMPLETE, 
			   cur_rx_state = UAVTALK_STATE_COMPLETE;

static void ProcessI2CStream(UAVTalkConnection inConnectionHandle,
		uint8_t rxbyte) {
	rx_processed_bytes++;
	old_rx_state = cur_rx_state;
	UAVTalkRxState state = UAVTalkProcessInputStreamQuiet(inConnectionHandle, rxbyte);
	cur_rx_state = state;
	if(cur_rx_state == UAVTALK_STATE_SYNC &&
			old_rx_state == UAVTALK_STATE_CS) {
		bad_checksums ++;
	}
	if(state == UAVTALK_STATE_COMPLETE) {
		rx_completed_objects++;
		uint32_t objId = UAVTalkGetPacketObjId(inConnectionHandle);
		switch(objId) {
			//case OPLINKSTATUS_OBJID:
			default:
				//Broadcast received UAVObject to system
				//UAVTalkRelayObject(inConnectionHandle, outConnectionHandle);
				if(UAVTalkReceiveObject(inConnectionHandle) == 0) {
					//Succesfully unpacked!
					//PIOS_LED_Toggle(PIOS_LED_D1);
				} else {
				}
		}
	}
}

int32_t ledexampleInitialize() {
	PIOS_I2C_UAVTALK_Init();
	//initialize UAVTalk
	i2cUAVTalkCon = UAVTalkInitialize(&sendHandler);
	
	return 0;
}


static void ledexampleTask(__attribute__((unused)) void *parameters)
{
	//static portTickType lastSysTime;
	//lastSysTime = xTaskGetTickCount();
	//This data struct is contained in the automatically
	//generated UAVObject code
	//<MyUAVObject>Data data;

	//Populate the data struct with the UAVObject's
	//current values
	//<MyUAVObject>Get(data);

	//bool err_state = false;
	//uint8_t i2c_data = 0;
	uint8_t i2c_data[MAX_I2C_RX_BUF_LEN];
	int32_t bytes_to_process = -1;
	/*
	uint8_t *byte;
	for(int i = 0; i < 1000000; ++i) {
		byte = pios_malloc(128);
		if(!byte) {
			return;
		}
	}
	*/
	while(1) {
		//PIOS_LED_Toggle(PIOS_LED_D1);
		//vTaskDelayUntil(&lastSysTime, UPDATE_PERIOD / portTICK_RATE_MS);
		//PIOS_LED_Toggle(PIOS_LED_D2);
		//PIOS_I2C_UAVTALK_Write(0,i2c_data);
		//struct pios_i2c_txn i2c_txn  = PIOS_I2C_UAVTALK_Read();
		//vTaskDelay(100 / portTICK_RATE_MS);
		bytes_to_process = PIOS_I2C_UAVTALK_Read(i2c_data, MAX_I2C_RX_BUF_LEN, 
				MAX_I2C_RX_DELAY);
		if(bytes_to_process < 0) {
			//ERROR, or none available
		} else {
			for(int32_t i = 0; i < bytes_to_process; ++i) {
				ProcessI2CStream(i2cUAVTalkCon, i2c_data[i]);
			}
			//PIOS_LED_Toggle(PIOS_LED_D1);
		}
		/*
		uint8_t * i2c_val = i2c_txn.buf;
		uint8_t response [] = {
			*i2c_val + 1, 
			*i2c_val + 2
		};
		uint8_t response [] = {
			i2c_txn.addr & 1, 
			2
		};
		PIOS_I2C_UAVTALK_Respond(response);
		*/
		//PIOS_LED_Toggle(PIOS_LED_D3); //Unused?
		//PIOS_LED_Toggle(PIOS_LED_D4); //Unused?
		//Get a new reading from the
		//thermocouple
		//data.thermocouple1 = ReadThermocouple1();

		//Update the UAVObject
		//data. The updated values
		//can be viewed in the
		//GCS.
		//<MyUAVObject>Set(&data);

		// Delay until it
		// is time to read
		// the next sample
		/*
		vTaskDelayUntil(&lastSysTime,
				UPDATE_PERIOD / portTICK_RATE_MS);
				*/
	}
}


int32_t ledexampleStart()
{
	// Start main task
	xTaskCreate(ledexampleTask, "LedExample", 
			128, NULL, tskIDLE_PRIORITY+4, NULL);
		//STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);

		//Monitor the running status of this
		//PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_TEMPERATURE, taskHandle);
	return 0;
}

MODULE_INITCALL( ledexampleInitialize , ledexampleStart );


