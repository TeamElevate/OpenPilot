/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup Attitude Copter Control Attitude Estimation
 * @brief Acquires sensor data and computes attitude estimate
 * Specifically updates the the @ref AttitudeState "AttitudeState" and @ref AttitudeRaw "AttitudeRaw" settings objects
 * @{
 *
 * @file       attitude.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Module to handle all comms to the AHRS on a periodic basis.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/**
 * Input objects: None, takes sensor data via pios
 * Output objects: @ref AttitudeRaw @ref AttitudeState
 *
 * This module computes an attitude estimate from the sensor data
 *
 * The module executes in its own thread.
 *
 * UAVObjects are automatically generated by the UAVObjectGenerator from
 * the object definition XML file.
 *
 * Modules have no API, all communication to other modules is done through UAVObjects.
 * However modules may use the API exposed by shared libraries.
 * See the OpenPilot wiki for more details.
 * http://www.openpilot.org/OpenPilot_Application_Architecture
 *
 */

#include <openpilot.h>

#include <pios_board_info.h>

#include "attitude.h"
#include "gyrostate.h"
#include "accelstate.h"
#include "attitudestate.h"
#include "attitudesettings.h"
#include "accelgyrosettings.h"
#include "flightstatus.h"
#include "manualcontrolcommand.h"
#include "taskinfo.h"

#include "CoordinateConversions.h"
#include <pios_notify.h>


#include <pios_instrumentation_helper.h>

PERF_DEFINE_COUNTER(counterUpd);
PERF_DEFINE_COUNTER(counterAccelSamples);
PERF_DEFINE_COUNTER(counterPeriod);
PERF_DEFINE_COUNTER(counterAtt);
// Counters:
// - 0xA7710001 sensor fetch duration
// - 0xA7710002 updateAttitude execution time
// - 0xA7710003 Attitude loop rate(period)
// - 0xA7710004 number of accel samples read for each loop (cc only).

// Private constants
#define STACK_SIZE_BYTES 540
#define TASK_PRIORITY    (tskIDLE_PRIORITY + 3)

#define SENSOR_PERIOD    4
#define UPDATE_RATE      25.0f

#define UPDATE_EXPECTED  (1.0f / 500.0f)
#define UPDATE_MIN       1.0e-6f
#define UPDATE_MAX       1.0f
#define UPDATE_ALPHA     1.0e-2f

// Private types

// Private variables
static xTaskHandle taskHandle;
static PiOSDeltatimeConfig dtconfig;

// Private functions
static void AttitudeTask(void *parameters);

static float gyro_correct_int[3] = { 0, 0, 0 };
static xQueueHandle gyro_queue;

static int32_t updateSensors(AccelStateData *, GyroStateData *);
static int32_t updateSensorsCC3D(AccelStateData *accelStateData, GyroStateData *gyrosData);
static void updateAttitude(AccelStateData *, GyroStateData *);
static void settingsUpdatedCb(UAVObjEvent *objEv);

static float accelKi     = 0;
static float accelKp     = 0;
static float accel_alpha = 0;
static bool accel_filter_enabled = false;
static float accels_filtered[3];
static float grot_filtered[3];
static float yawBiasRate = 0;
static float rollPitchBiasRate = 0.0f;
static AccelGyroSettingsaccel_biasData accel_bias;
static float q[4] = { 1, 0, 0, 0 };
static float R[3][3];
static int8_t rotate = 0;
static bool zero_during_arming = false;
static bool bias_correct_gyro  = true;

// static float gyros_passed[3];

// temp coefficient to calculate gyro bias
static bool apply_gyro_temp  = false;
static bool apply_accel_temp = false;
static AccelGyroSettingsgyro_temp_coeffData gyro_temp_coeff;;
static AccelGyroSettingsaccel_temp_coeffData accel_temp_coeff;
static AccelGyroSettingstemp_calibrated_extentData temp_calibrated_extent;

// Accel and Gyro scaling (this is the product of sensor scale and adjustement in AccelGyroSettings
static AccelGyroSettingsgyro_scaleData gyro_scale;
static AccelGyroSettingsaccel_scaleData accel_scale;


// For running trim flights
static volatile bool trim_requested   = false;
static volatile int32_t trim_accels[3];
static volatile int32_t trim_samples;
int32_t const MAX_TRIM_FLIGHT_SAMPLES = 65535;

#define GRAV                       9.81f
#define STD_CC_ACCEL_SCALE         (GRAV * 0.004f)
/* 0.004f is gravity / LSB */
#define STD_CC_ANALOG_GYRO_NEUTRAL 1665
#define STD_CC_ANALOG_GYRO_GAIN    0.42f

// Used to detect CC vs CC3D
static const struct pios_board_info *bdinfo = &pios_board_info_blob;
#define BOARDISCC3D                (bdinfo->board_rev == 0x02)
/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AttitudeStart(void)
{
    // Start main task
    xTaskCreate(AttitudeTask, "Attitude", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);
    PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_ATTITUDE, taskHandle);
#ifdef PIOS_INCLUDE_WDG
    PIOS_WDG_RegisterFlag(PIOS_WDG_ATTITUDE);
#endif

    return 0;
}

/**
 * Initialise the module, called on startup
 * \returns 0 on success or -1 if initialisation failed
 */
int32_t AttitudeInitialize(void)
{
    AttitudeStateInitialize();
    AttitudeSettingsInitialize();
    AccelGyroSettingsInitialize();
    AccelStateInitialize();
    GyroStateInitialize();

    // Initialize quaternion
    AttitudeStateData attitude;
    AttitudeStateGet(&attitude);
    attitude.q1 = 1;
    attitude.q2 = 0;
    attitude.q3 = 0;
    attitude.q4 = 0;
    AttitudeStateSet(&attitude);

    // Cannot trust the values to init right above if BL runs
    gyro_correct_int[0] = 0;
    gyro_correct_int[1] = 0;
    gyro_correct_int[2] = 0;

    q[0] = 1;
    q[1] = 0;
    q[2] = 0;
    q[3] = 0;
    for (uint8_t i = 0; i < 3; i++) {
        for (uint8_t j = 0; j < 3; j++) {
            R[i][j] = 0;
        }
    }

    trim_requested = false;

    AttitudeSettingsConnectCallback(&settingsUpdatedCb);
    AccelGyroSettingsConnectCallback(&settingsUpdatedCb);
    return 0;
}

MODULE_INITCALL(AttitudeInitialize, AttitudeStart);

/**
 * Module thread, should not return.
 */

int32_t accel_test;
int32_t gyro_test;
static void AttitudeTask(__attribute__((unused)) void *parameters)
{
    uint8_t init = 0;

    AlarmsClear(SYSTEMALARMS_ALARM_ATTITUDE);

    // Set critical error and wait until the accel is producing data
    while (PIOS_ADXL345_FifoElements() == 0) {
        AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_CRITICAL);
#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_ATTITUDE);
#endif
    }

    bool cc3d = BOARDISCC3D;

    if (cc3d) {
#if defined(PIOS_INCLUDE_MPU6000)
        gyro_test = PIOS_MPU6000_Test();
#endif
    } else {
#if defined(PIOS_INCLUDE_ADXL345)
        accel_test = PIOS_ADXL345_Test();
#endif

#if defined(PIOS_INCLUDE_ADC)
        // Create queue for passing gyro data, allow 2 back samples in case
        gyro_queue = xQueueCreate(1, sizeof(float) * 4);
        PIOS_Assert(gyro_queue != NULL);
        PIOS_ADC_SetQueue(gyro_queue);
        PIOS_ADC_Config(46);
#endif
    }

    PERF_INIT_COUNTER(counterUpd, 0xA7710001);
    PERF_INIT_COUNTER(counterAtt, 0xA7710002);
    PERF_INIT_COUNTER(counterPeriod, 0xA7710003);
    PERF_INIT_COUNTER(counterAccelSamples, 0xA7710004);

    // Force settings update to make sure rotation loaded
    settingsUpdatedCb(AttitudeSettingsHandle());
	volatile float num_good = 0, num_bad = 0, percent_good = 0;

    PIOS_DELTATIME_Init(&dtconfig, UPDATE_EXPECTED, UPDATE_MIN, UPDATE_MAX, UPDATE_ALPHA);

    // Main task loop
    while (1) {
        FlightStatusData flightStatus;
        FlightStatusGet(&flightStatus);

        if ((xTaskGetTickCount() < 7000) && (xTaskGetTickCount() > 1000)) {
            // Use accels to initialise attitude and calculate gyro bias
            accelKp     = 1.0f;
            accelKi     = 0.0f;
            yawBiasRate = 0.01f;
            rollPitchBiasRate    = 0.01f;
            accel_filter_enabled = false;
            init = 0;
            PIOS_NOTIFY_StartNotification(NOTIFY_DRAW_ATTENTION, NOTIFY_PRIORITY_REGULAR);
        } else if (zero_during_arming && (flightStatus.Armed == FLIGHTSTATUS_ARMED_ARMING)) {
            accelKp     = 1.0f;
            accelKi     = 0.0f;
            yawBiasRate = 0.01f;
            rollPitchBiasRate    = 0.01f;
            accel_filter_enabled = false;
            init = 0;
            PIOS_NOTIFY_StartNotification(NOTIFY_DRAW_ATTENTION, NOTIFY_PRIORITY_REGULAR);
        } else if (init == 0) {
            // Reload settings (all the rates)
            AttitudeSettingsAccelKiGet(&accelKi);
            AttitudeSettingsAccelKpGet(&accelKp);
            AttitudeSettingsYawBiasRateGet(&yawBiasRate);
            rollPitchBiasRate = 0.0f;
            if (accel_alpha > 0.0f) {
                accel_filter_enabled = true;
            }
            init = 1;
        }

#ifdef PIOS_INCLUDE_WDG
        PIOS_WDG_UpdateFlag(PIOS_WDG_ATTITUDE);
#endif

        AccelStateData accelState;
        GyroStateData gyros;
        int32_t retval = 0;

        if (cc3d) {
            retval = updateSensorsCC3D(&accelState, &gyros);
        } else {
            retval = updateSensors(&accelState, &gyros);
        }

        // Only update attitude when sensor data is good
        if (retval != 0) {
            AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_ERROR);
			num_bad ++;
			percent_good = (num_bad + num_good) > 0.0f ? (num_good / (num_bad+num_good)) : 0;
        } else {
            // Do not update attitude data in simulation mode
            if (!AttitudeStateReadOnly()) {
                PERF_TIMED_SECTION_START(counterAtt);
                updateAttitude(&accelState, &gyros);
                PERF_TIMED_SECTION_END(counterAtt);
            }
            PERF_MEASURE_PERIOD(counterPeriod);
            AlarmsClear(SYSTEMALARMS_ALARM_ATTITUDE);
			num_good ++;
			if(percent_good < 0) {
			} else {
				percent_good = (num_bad + num_good) > 0.0f ? (num_good / (num_bad+num_good)) : 0;
			}
        }
    }
}

/**
 * Get an update from the sensors
 * @param[in] attitudeRaw Populate the UAVO instead of saving right here
 * @return 0 if successfull, -1 if not
 */
static int32_t updateSensors(AccelStateData *accelState, GyroStateData *gyros)
{
    struct pios_adxl345_data accel_data;
    float gyro[4];

    // Only wait the time for two nominal updates before setting an alarm
    if (xQueueReceive(gyro_queue, (void *const)gyro, UPDATE_RATE * 2) == errQUEUE_EMPTY) {
        AlarmsSet(SYSTEMALARMS_ALARM_ATTITUDE, SYSTEMALARMS_ALARM_ERROR);
        return -1;
    }

    // Do not read raw sensor data in simulation mode
    if (GyroStateReadOnly() || AccelStateReadOnly()) {
        return 0;
    }

    // No accel data available
    uint8_t fifoSamples = PIOS_ADXL345_FifoElements();
    if (fifoSamples == 0) {
        return -1;
    }
    PERF_TIMED_SECTION_START(counterUpd);
    // First sample is temperature
    gyros->x = -(gyro[1] - STD_CC_ANALOG_GYRO_NEUTRAL) * gyro_scale.X;
    gyros->y = (gyro[2] - STD_CC_ANALOG_GYRO_NEUTRAL) * gyro_scale.Y;
    gyros->z = -(gyro[3] - STD_CC_ANALOG_GYRO_NEUTRAL) * gyro_scale.Z;

    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;

    uint8_t i = fifoSamples;
    uint8_t samples_remaining;
    samples_remaining = PIOS_ADXL345_ReadAndAccumulateSamples(&accel_data, fifoSamples);
    x = accel_data.x;
    y = -accel_data.y;
    z = -accel_data.z;
    if (samples_remaining > 0) {
        do {
            i++;
            samples_remaining = PIOS_ADXL345_Read(&accel_data);
            x += accel_data.x;
            y += -accel_data.y;
            z += -accel_data.z;
        } while ((i < 32) && (samples_remaining > 0));
    }
    PERF_TRACK_VALUE(counterAccelSamples, i);
    float accel[3] = { accel_scale.X * (float)x / i,
                       accel_scale.Y * (float)y / i,
                       accel_scale.Z * (float)z / i };

    if (rotate) {
        // TODO: rotate sensors too so stabilization is well behaved
        float vec_out[3];
        rot_mult(R, accel, vec_out);
        accelState->x = vec_out[0];
        accelState->y = vec_out[1];
        accelState->z = vec_out[2];
        rot_mult(R, &gyros->x, vec_out);
        gyros->x = vec_out[0];
        gyros->y = vec_out[1];
        gyros->z = vec_out[2];
    } else {
        accelState->x = accel[0];
        accelState->y = accel[1];
        accelState->z = accel[2];
    }

    if (trim_requested) {
        if (trim_samples >= MAX_TRIM_FLIGHT_SAMPLES) {
            trim_requested = false;
        } else {
            uint8_t armed;
            float throttle;
            FlightStatusArmedGet(&armed);
            ManualControlCommandThrottleGet(&throttle); // Until flight status indicates airborne
            if ((armed == FLIGHTSTATUS_ARMED_ARMED) && (throttle > 0.0f)) {
                trim_samples++;
                // Store the digitally scaled version since that is what we use for bias
                trim_accels[0] += accelState->x;
                trim_accels[1] += accelState->y;
                trim_accels[2] += accelState->z;
            }
        }
    }

    // Scale accels and correct bias
    accelState->x -= accel_bias.X;
    accelState->y -= accel_bias.Y;
    accelState->z -= accel_bias.Z;

    if (bias_correct_gyro) {
        // Applying integral component here so it can be seen on the gyros and correct bias
        gyros->x += gyro_correct_int[0];
        gyros->y += gyro_correct_int[1];
        gyros->z += gyro_correct_int[2];
    }

    // Force the roll & pitch gyro rates to average to zero during initialisation
    gyro_correct_int[0] += -gyros->x * rollPitchBiasRate;
    gyro_correct_int[1] += -gyros->y * rollPitchBiasRate;

    // Because most crafts wont get enough information from gravity to zero yaw gyro, we try
    // and make it average zero (weakly)
    gyro_correct_int[2] += -gyros->z * yawBiasRate;
    PERF_TIMED_SECTION_END(counterUpd);

    GyroStateSet(gyros);
    AccelStateSet(accelState);

    return 0;
}

/**
 * Get an update from the sensors
 * @param[in] attitudeRaw Populate the UAVO instead of saving right here
 * @return 0 if successfull, -1 if not
 */
static struct pios_mpu6000_data mpu6000_data;
static int32_t updateSensorsCC3D(AccelStateData *accelStateData, GyroStateData *gyrosData)
{
    float accels[3], gyros[3];

#if defined(PIOS_INCLUDE_MPU6000)

    xQueueHandle queue = PIOS_MPU6000_GetQueue();

    if (xQueueReceive(queue, (void *)&mpu6000_data, SENSOR_PERIOD) == errQUEUE_EMPTY) {
        return -1; // Error, no data
    }
    // Do not read raw sensor data in simulation mode
    if (GyroStateReadOnly() || AccelStateReadOnly()) {
        return 0;
    }
    PERF_TIMED_SECTION_START(counterUpd);
    gyros[0]  = mpu6000_data.gyro_x * gyro_scale.X;
    gyros[1]  = mpu6000_data.gyro_y * gyro_scale.Y;
    gyros[2]  = mpu6000_data.gyro_z * gyro_scale.Z;

    accels[0] = mpu6000_data.accel_x * accel_scale.X;
    accels[1] = mpu6000_data.accel_y * accel_scale.Y;
    accels[2] = mpu6000_data.accel_z * accel_scale.Z;

    float ctemp = mpu6000_data.temperature > temp_calibrated_extent.max ? temp_calibrated_extent.max :
                  (mpu6000_data.temperature < temp_calibrated_extent.min ? temp_calibrated_extent.min
                   : mpu6000_data.temperature);


    if (apply_gyro_temp) {
        gyros[0] -= (gyro_temp_coeff.X + gyro_temp_coeff.X2 * ctemp) * ctemp;
        gyros[1] -= (gyro_temp_coeff.Y + gyro_temp_coeff.Y2 * ctemp) * ctemp;
        gyros[2] -= (gyro_temp_coeff.Z + gyro_temp_coeff.Z2 * ctemp) * ctemp;
    }

    if (apply_accel_temp) {
        accels[0] -= accel_temp_coeff.X * ctemp;
        accels[1] -= accel_temp_coeff.Y * ctemp;
        accels[2] -= accel_temp_coeff.Z * ctemp;
    }
    // gyrosData->temperature  = 35.0f + ((float)mpu6000_data.temperature + 512.0f) / 340.0f;
    // accelsData->temperature = 35.0f + ((float)mpu6000_data.temperature + 512.0f) / 340.0f;
#endif /* if defined(PIOS_INCLUDE_MPU6000) */

    if (rotate) {
        // TODO: rotate sensors too so stabilization is well behaved
        float vec_out[3];
        rot_mult(R, accels, vec_out);
        accels[0] = vec_out[0];
        accels[1] = vec_out[1];
        accels[2] = vec_out[2];
        rot_mult(R, gyros, vec_out);
        gyros[0]  = vec_out[0];
        gyros[1]  = vec_out[1];
        gyros[2]  = vec_out[2];
    }

    accelStateData->x = accels[0] - accel_bias.X;
    accelStateData->y = accels[1] - accel_bias.Y;
    accelStateData->z = accels[2] - accel_bias.Z;

    gyrosData->x = gyros[0];
    gyrosData->y = gyros[1];
    gyrosData->z = gyros[2];

    if (bias_correct_gyro) {
        // Applying integral component here so it can be seen on the gyros and correct bias
        gyrosData->x += gyro_correct_int[0];
        gyrosData->y += gyro_correct_int[1];
        gyrosData->z += gyro_correct_int[2];
    }

    // Force the roll & pitch gyro rates to average to zero during initialisation
    gyro_correct_int[0] += -gyrosData->x * rollPitchBiasRate;
    gyro_correct_int[1] += -gyrosData->y * rollPitchBiasRate;

    // Because most crafts wont get enough information from gravity to zero yaw gyro, we try
    // and make it average zero (weakly)
    gyro_correct_int[2] += -gyrosData->z * yawBiasRate;
    PERF_TIMED_SECTION_END(counterUpd);
    GyroStateSet(gyrosData);
    AccelStateSet(accelStateData);

    return 0;
}

static inline void apply_accel_filter(const float *raw, float *filtered)
{
    if (accel_filter_enabled) {
        filtered[0] = filtered[0] * accel_alpha + raw[0] * (1 - accel_alpha);
        filtered[1] = filtered[1] * accel_alpha + raw[1] * (1 - accel_alpha);
        filtered[2] = filtered[2] * accel_alpha + raw[2] * (1 - accel_alpha);
    } else {
        filtered[0] = raw[0];
        filtered[1] = raw[1];
        filtered[2] = raw[2];
    }
}

__attribute__((optimize("O3"))) static void updateAttitude(AccelStateData *accelStateData, GyroStateData *gyrosData)
{
    float dT      = PIOS_DELTATIME_GetAverageSeconds(&dtconfig);

    // Bad practice to assume structure order, but saves memory
    float *gyros  = &gyrosData->x;
    float *accels = &accelStateData->x;

    float grot[3];
    float accel_err[3];

    // Apply smoothing to accel values, to reduce vibration noise before main calculations.
    apply_accel_filter(accels, accels_filtered);

    // Rotate gravity unit vector to body frame, filter and cross with accels
    grot[0] = -(2 * (q[1] * q[3] - q[0] * q[2]));
    grot[1] = -(2 * (q[2] * q[3] + q[0] * q[1]));
    grot[2] = -(q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);

    apply_accel_filter(grot, grot_filtered);

    CrossProduct((const float *)accels_filtered, (const float *)grot_filtered, accel_err);

    // Account for accel magnitude
    float accel_mag = sqrtf(accels_filtered[0] * accels_filtered[0] + accels_filtered[1] * accels_filtered[1] + accels_filtered[2] * accels_filtered[2]);
    if (accel_mag < 1.0e-3f) {
        return;
    }

    // Account for filtered gravity vector magnitude
    float grot_mag;

    if (accel_filter_enabled) {
        grot_mag = sqrtf(grot_filtered[0] * grot_filtered[0] + grot_filtered[1] * grot_filtered[1] + grot_filtered[2] * grot_filtered[2]);
    } else {
        grot_mag = 1.0f;
    }

    if (grot_mag < 1.0e-3f) {
        return;
    }
    const float invMag = 1.0f / (accel_mag * grot_mag);
    accel_err[0] *= invMag;
    accel_err[1] *= invMag;
    accel_err[2] *= invMag;

    // Accumulate integral of error.  Scale here so that units are (deg/s) but Ki has units of s
    gyro_correct_int[0] += accel_err[0] * accelKi;
    gyro_correct_int[1] += accel_err[1] * accelKi;

    // gyro_correct_int[2] += accel_err[2] * accelKi;

    // Correct rates based on error, integral component dealt with in updateSensors
    const float kpInvdT = accelKp / dT;
    gyros[0] += accel_err[0] * kpInvdT;
    gyros[1] += accel_err[1] * kpInvdT;
    gyros[2] += accel_err[2] * kpInvdT;

    { // scoping variables to save memory
      // Work out time derivative from INSAlgo writeup
      // Also accounts for the fact that gyros are in deg/s
        float qdot[4];
        qdot[0] = (-q[1] * gyros[0] - q[2] * gyros[1] - q[3] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[1] = (q[0] * gyros[0] - q[3] * gyros[1] + q[2] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[2] = (q[3] * gyros[0] + q[0] * gyros[1] - q[1] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);
        qdot[3] = (-q[2] * gyros[0] + q[1] * gyros[1] + q[0] * gyros[2]) * dT * (M_PI_F / 180.0f / 2.0f);

        // Take a time step
        q[0]    = q[0] + qdot[0];
        q[1]    = q[1] + qdot[1];
        q[2]    = q[2] + qdot[2];
        q[3]    = q[3] + qdot[3];

        if (q[0] < 0) {
            q[0] = -q[0];
            q[1] = -q[1];
            q[2] = -q[2];
            q[3] = -q[3];
        }
    }

    // Renomalize
    float qmag = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);

    // If quaternion has become inappropriately short or is nan reinit.
    // THIS SHOULD NEVER ACTUALLY HAPPEN
    if ((fabsf(qmag) < 1e-3f) || isnan(qmag)) {
        q[0] = 1;
        q[1] = 0;
        q[2] = 0;
        q[3] = 0;
    } else {
        const float invQmag = 1.0f / qmag;
        q[0] = q[0] * invQmag;
        q[1] = q[1] * invQmag;
        q[2] = q[2] * invQmag;
        q[3] = q[3] * invQmag;
    }

    AttitudeStateData attitudeState;
    AttitudeStateGet(&attitudeState);

    quat_copy(q, &attitudeState.q1);

    // Convert into eueler degrees (makes assumptions about RPY order)
    Quaternion2RPY(&attitudeState.q1, &attitudeState.Roll);

    AttitudeStateSet(&attitudeState);
}

static void settingsUpdatedCb(__attribute__((unused)) UAVObjEvent *objEv)
{
    AttitudeSettingsData attitudeSettings;
    AccelGyroSettingsData accelGyroSettings;

    AttitudeSettingsGet(&attitudeSettings);
    AccelGyroSettingsGet(&accelGyroSettings);

    accelKp     = attitudeSettings.AccelKp;
    accelKi     = attitudeSettings.AccelKi;
    yawBiasRate = attitudeSettings.YawBiasRate;

    // Calculate accel filter alpha, in the same way as for gyro data in stabilization module.
    const float fakeDt = 0.0025f;
    if (attitudeSettings.AccelTau < 0.0001f) {
        accel_alpha = 0; // not trusting this to resolve to 0
        accel_filter_enabled = false;
    } else {
        accel_alpha = expf(-fakeDt / attitudeSettings.AccelTau);
        accel_filter_enabled = true;
    }

    zero_during_arming = attitudeSettings.ZeroDuringArming == ATTITUDESETTINGS_ZERODURINGARMING_TRUE;
    bias_correct_gyro  = attitudeSettings.BiasCorrectGyro == ATTITUDESETTINGS_BIASCORRECTGYRO_TRUE;

    memcpy(&gyro_temp_coeff, &accelGyroSettings.gyro_temp_coeff, sizeof(AccelGyroSettingsgyro_temp_coeffData));
    memcpy(&accel_temp_coeff, &accelGyroSettings.accel_temp_coeff, sizeof(AccelGyroSettingsaccel_temp_coeffData));


    apply_gyro_temp = (fabsf(gyro_temp_coeff.X) > 1e-6f ||
                       fabsf(gyro_temp_coeff.Y) > 1e-6f ||
                       fabsf(gyro_temp_coeff.Z) > 1e-6f ||
                       fabsf(gyro_temp_coeff.X2) > 1e-6f ||
                       fabsf(gyro_temp_coeff.Y2) > 1e-6f ||
                       fabsf(gyro_temp_coeff.Z2) > 1e-6f);

    apply_accel_temp = (fabsf(accel_temp_coeff.X) > 1e-6f ||
                        fabsf(accel_temp_coeff.Y) > 1e-6f ||
                        fabsf(accel_temp_coeff.Z) > 1e-6f);

    gyro_correct_int[0] = accelGyroSettings.gyro_bias.X;
    gyro_correct_int[1] = accelGyroSettings.gyro_bias.Y;
    gyro_correct_int[2] = accelGyroSettings.gyro_bias.Z;

    temp_calibrated_extent.min = accelGyroSettings.temp_calibrated_extent.min;
    temp_calibrated_extent.max = accelGyroSettings.temp_calibrated_extent.max;

    if (BOARDISCC3D) {
        accel_bias.X  = accelGyroSettings.accel_bias.X;
        accel_bias.Y  = accelGyroSettings.accel_bias.Y;
        accel_bias.Z  = accelGyroSettings.accel_bias.Z;

        gyro_scale.X  = accelGyroSettings.gyro_scale.X * PIOS_MPU6000_GetScale();
        gyro_scale.Y  = accelGyroSettings.gyro_scale.Y * PIOS_MPU6000_GetScale();
        gyro_scale.Z  = accelGyroSettings.gyro_scale.Z * PIOS_MPU6000_GetScale();

        accel_scale.X = accelGyroSettings.accel_scale.X * PIOS_MPU6000_GetAccelScale();
        accel_scale.Y = accelGyroSettings.accel_scale.Y * PIOS_MPU6000_GetAccelScale();
        accel_scale.Z = accelGyroSettings.accel_scale.Z * PIOS_MPU6000_GetAccelScale();
    } else {
        // Original CC with analog gyros and ADXL accel
        accel_bias.X  = accelGyroSettings.accel_bias.X;
        accel_bias.Y  = accelGyroSettings.accel_bias.Y;
        accel_bias.Z  = accelGyroSettings.accel_bias.Z;

        gyro_scale.X  = accelGyroSettings.gyro_scale.X * STD_CC_ANALOG_GYRO_GAIN;
        gyro_scale.Y  = accelGyroSettings.gyro_scale.Y * STD_CC_ANALOG_GYRO_GAIN;
        gyro_scale.Z  = accelGyroSettings.gyro_scale.Z * STD_CC_ANALOG_GYRO_GAIN;

        accel_scale.X = accelGyroSettings.accel_scale.X * STD_CC_ACCEL_SCALE;
        accel_scale.Y = accelGyroSettings.accel_scale.Y * STD_CC_ACCEL_SCALE;
        accel_scale.Z = accelGyroSettings.accel_scale.Z * STD_CC_ACCEL_SCALE;
    }

    // Indicates not to expend cycles on rotation
    if (fabsf(attitudeSettings.BoardRotation.Pitch) < 0.00001f &&
        fabsf(attitudeSettings.BoardRotation.Roll) < 0.00001f &&
        fabsf(attitudeSettings.BoardRotation.Yaw) < 0.00001f) {
        rotate = 0;

        // Shouldn't be used but to be safe
        float rotationQuat[4] = { 1, 0, 0, 0 };
        Quaternion2R(rotationQuat, R);
    } else {
        float rotationQuat[4];
        const float rpy[3] = { attitudeSettings.BoardRotation.Roll,
                               attitudeSettings.BoardRotation.Pitch,
                               attitudeSettings.BoardRotation.Yaw };
        RPY2Quaternion(rpy, rotationQuat);
        Quaternion2R(rotationQuat, R);
        rotate = 1;
    }

    if (attitudeSettings.TrimFlight == ATTITUDESETTINGS_TRIMFLIGHT_START) {
        trim_accels[0] = 0;
        trim_accels[1] = 0;
        trim_accels[2] = 0;
        trim_samples   = 0;
        trim_requested = true;
    } else if (attitudeSettings.TrimFlight == ATTITUDESETTINGS_TRIMFLIGHT_LOAD) {
        trim_requested = false;
        accelGyroSettings.accel_scale.X = trim_accels[0] / trim_samples;
        accelGyroSettings.accel_scale.Y = trim_accels[1] / trim_samples;
        // Z should average -grav
        accelGyroSettings.accel_scale.Z = trim_accels[2] / trim_samples + GRAV;
        attitudeSettings.TrimFlight     = ATTITUDESETTINGS_TRIMFLIGHT_NORMAL;
        AttitudeSettingsSet(&attitudeSettings);
    } else {
        trim_requested = false;
    }
}
/**
 * @}
 * @}
 */
