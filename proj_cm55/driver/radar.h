/*
 * radar.h
 *
 *  Created on: Nov 24, 2025
 *      Author: ROJ030
 */

#ifndef DRIVER_RADAR_H_
#define DRIVER_RADAR_H_

#include <stdint.h>

int radar_init();

int radar_is_data_available();

int radar_get_num_samples_per_frame();

int radar_read_data(uint16_t* data, uint16_t num_samples);


#endif /* DRIVER_RADAR_H_ */
