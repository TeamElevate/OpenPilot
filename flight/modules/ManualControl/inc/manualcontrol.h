/**
 ******************************************************************************
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{
 * @addtogroup ManualControlModule Manual Control Module
 * @{
 *
 * @file       manualcontrol.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      ManualControl module. Handles safety R/C link and flight mode.
 *
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
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
#ifndef MANUALCONTROL_H
#define MANUALCONTROL_H

#include <openpilot.h>
#include <flightstatus.h>

typedef void (*handlerFunc)(bool newinit);

typedef struct controlHandlerStruct {
    FlightStatusControlChainData controlChain;
    handlerFunc handler;
} controlHandler;

/**
 * @brief Handler to interprete Command inputs in regards to arming/disarming (called in all flight modes)
 * @input: ManualControlCommand, AccessoryDesired
 * @output: FlightStatus.Arming
 */
void armHandler(bool newinit);

/**
 * @brief Handler to control Manual flightmode - input directly steers actuators
 * @input: ManualControlCommand
 * @output: ActuatorDesired
 */
void manualHandler(bool newinit);

/**
 * @brief Handler to control Stabilized flightmodes. FlightControl is governed by "Stabilization"
 * @input: ManualControlCommand
 * @output: StabilizationDesired
 */
void stabilizedHandler(bool newinit);

/**
 * @brief Handler to control Guided flightmodes. FlightControl is governed by PathFollower, control via PathDesired
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands
 * @output: PathDesired
 */
void pathFollowerHandler(bool newinit);

/**
 * @brief Handler to control Navigated flightmodes. FlightControl is governed by PathFollower, controlled indirectly via PathPlanner
 * @input: NONE: fully automated mode -- TODO recursively call handler for advanced stick commands to affect navigation
 * @output: NONE
 */
void pathPlannerHandler(bool newinit);

/**
 * @brief Handler to setup takeofflocation on arming. it is set up during Arming
 * @input: NONE:
 * @output: NONE
 */
void takeOffLocationHandler();

/**
 * @brief Initialize TakeoffLocationHanlder
 */
void takeOffLocationHandlerInit();

/*
 * These are assumptions we make in the flight code about the order of settings and their consistency between
 * objects.  Please keep this synchronized to the UAVObjects
 */
#define assumptions1 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions2 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions3 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions4 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions5 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions6 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_MANUAL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_RATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_ATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_AXISLOCK == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_AXISLOCK) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_WEAKLEVELING == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_WEAKLEVELING) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_VIRTUALBAR == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_VIRTUALBAR) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_RATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_RELAYRATE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYRATE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_RELAYATTITUDE == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_RELAYATTITUDE) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_ALTITUDEHOLD == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEHOLD) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_ALTITUDEVARIO == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_ALTITUDEVARIO) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_CRUISECONTROL == (int)STABILIZATIONDESIRED_STABILIZATIONMODE_CRUISECONTROL) && \
        1 \
    )

#define assumptions7 \
    ( \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION2SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION3SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION4SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION5SETTINGS_NUMELEM) && \
        ((int)FLIGHTMODESETTINGS_STABILIZATION1SETTINGS_NUMELEM == (int)FLIGHTMODESETTINGS_STABILIZATION6SETTINGS_NUMELEM) && \
        1 \
    )

#define assumptions_flightmode \
    ( \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_MANUAL == (int)FLIGHTSTATUS_FLIGHTMODE_MANUAL) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED1 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED1) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED2 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED2) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED3 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED3) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED4 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED4) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED5 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED5) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_STABILIZED6 == (int)FLIGHTSTATUS_FLIGHTMODE_STABILIZED6) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_AUTOTUNE == (int)FLIGHTSTATUS_FLIGHTMODE_AUTOTUNE) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POSITIONHOLD == (int)FLIGHTSTATUS_FLIGHTMODE_POSITIONHOLD) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_RETURNTOBASE == (int)FLIGHTSTATUS_FLIGHTMODE_RETURNTOBASE) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_LAND == (int)FLIGHTSTATUS_FLIGHTMODE_LAND) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_PATHPLANNER == (int)FLIGHTSTATUS_FLIGHTMODE_PATHPLANNER) && \
        ((int)FLIGHTMODESETTINGS_FLIGHTMODEPOSITION_POI == (int)FLIGHTSTATUS_FLIGHTMODE_POI) && \
        1 \
    )

#endif // MANUALCONTROL_H
