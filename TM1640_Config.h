/*
 * TM1640_Config.h - Project-specific configuration for TM1640
 *
 * Edit this file to match your hardware setup
 *
 * Created on: Dec 23, 2025
 * Author: e.zehni
 */

#ifndef TM1640_CONFIG_H_
#define TM1640_CONFIG_H_

/* ============================================
 * Hardware Configuration
 * ============================================ */

/* Buffer/Inverter Support
 * Set to 1 if signals pass through ULN2003 or other inverter
 * Set to 0 for direct connection
 */
#define TM1640_USE_BUFFER    1

/* Display Type Configuration
 * Set to 1 for Common Anode displays
 * Set to 0 for Common Cathode displays
 */
#define TM1640_COMMON_ANODE  1

#endif /* TM1640_CONFIG_H_ */

