#include <openpilot.h>
#include <pios_board_info.h>
#include "taskinfo.h"
#include "pios_i2c_uavtalk.h"

//#include "ledexample.h"
#define UPDATE_PERIOD 500

int32_t ledexampleInitialize() {
	PIOS_I2C_UAVTALK_Init();
	return 0;
}

static void ledexampleTask(__attribute__((unused)) void *parameters)
{
	static portTickType lastSysTime;
	lastSysTime = xTaskGetTickCount();
	//This data struct is contained in the automatically
	//generated UAVObject code
	//<MyUAVObject>Data data;

	//Populate the data struct with the UAVObject's
	//current values
	//<MyUAVObject>Get(data);

	bool err_state = false;
	//uint8_t i2c_data = 0;
	while(1) {
		PIOS_LED_Toggle(PIOS_LED_D1);
		vTaskDelayUntil(&lastSysTime,
				UPDATE_PERIOD / portTICK_RATE_MS);
		PIOS_LED_Toggle(PIOS_LED_D2);
		//PIOS_I2C_UAVTALK_Write(0,i2c_data);
		//struct pios_i2c_txn i2c_txn  = PIOS_I2C_UAVTALK_Read();
		vTaskDelay(100 / portTICK_RATE_MS);
		//PIOS_I2C_UAVTALK_Read(&i2c_data, 1);
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
		if(!err_state) {
			/*
			switch(i2c_val) {
				case -1:
					PIOS_LED_On(PIOS_LED_D1);
					PIOS_LED_Off(PIOS_LED_D2);
					err_state = true;
					break;
				case -2:
					PIOS_LED_Off(PIOS_LED_D1);
					PIOS_LED_On(PIOS_LED_D2);
					err_state = true;
					break;
				case -3:
					PIOS_LED_On(PIOS_LED_D1);
					PIOS_LED_On(PIOS_LED_D2);
					err_state = true;
					break;
				default:
					PIOS_LED_Off(PIOS_LED_D1);
					PIOS_LED_Off(PIOS_LED_D2);
					break;
			}
			*/
		}
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
		vTaskDelayUntil(&lastSysTime,
				UPDATE_PERIOD / portTICK_RATE_MS);
	}
}


int32_t ledexampleStart()
{
	// Start main task
	xTaskCreate(ledexampleTask, "LedExample", 128, NULL, 3, NULL);
			//STACK_SIZE_BYTES/4, NULL, TASK_PRIORITY, &taskHandle);

	//Monitor the running status of this
	//PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_TEMPERATURE, taskHandle);
	return 0;
}

MODULE_INITCALL( ledexampleInitialize , ledexampleStart );


