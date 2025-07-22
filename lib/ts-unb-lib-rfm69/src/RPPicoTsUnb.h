/* -----------------------------------------------------------------------------

Software License for the Fraunhofer TS-UNB-Lib

(c) Copyright  2019 - 2023 Fraunhofer-Gesellschaft zur Förderung der angewandten
Forschung e.V. All rights reserved.


1. INTRODUCTION

The Fraunhofer Telegram Splitting - Ultra Narrowband Library ("TS-UNB-Lib") is software
that implements only the uplink of the ETSI TS 103 357 TS-UNB standard ("MIOTY") for wireless 
data transmission in the field of IoT. Patent licenses for any patent claim regarding the 
ETSI TS 103 357 TS-UNB standard implementation (including those of Fraunhofer) may be 
obtained through Sisvel International S.A. 
(https://www.sisvel.com/licensing-programs/wireless-communications/mioty/license-terms)
or through the respective patent owners individually. The purpose of this TS-UNB-Lib is 
academic and non-commercial use. Therefore, Fraunhofer does not offer any support for the 
TS-UNB-Lib. Furthermore, the TS-UNB-Lib is NOT identical and on the same quality level as 
the commercially-licensed MIOTY software also available from Fraunhofer. Users are encouraged
to check the Fraunhofer website for additional applications information and documentation.


2. COPYRIGHT LICENSE

Redistribution and use in source and binary forms, with or without modification, are 
permitted without payment of copyright license fees provided that you satisfy the following 
conditions: You must retain the complete text of this software license in redistributions
of the TS-UNB-Lib software or your modifications thereto in source code form. You must retain 
the complete text of this software license in the documentation and/or other materials provided
with redistributions of the TS-UNB-Lib software or your modifications thereto in binary form.
You must make available free of charge copies of the complete source code of the TS-UNB-Lib 
software and your modifications thereto to recipients of copies in binary form. The name of 
Fraunhofer may not be used to endorse or promote products derived from this software without
prior written permission. You may not charge copyright license fees for anyone to use, copy or
distribute the TS-UNB-Lib software or your modifications thereto. Your modified versions of the
TS-UNB-Lib software must carry prominent notices stating that you changed the software and the
date of any change. For modified versions of the TS-UNB-Lib software, the term 
"Fraunhofer TS-UNB-Lib" must be replaced by the term
"Third-Party Modified Version of the Fraunhofer TS-UNB-Lib."


3. NO PATENT LICENSE

NO EXPRESS OR IMPLIED LICENSES TO ANY PATENT CLAIMS, including without limitation the patents 
of Fraunhofer, ARE GRANTED BY THIS SOFTWARE LICENSE. Fraunhofer provides no warranty of patent 
non-infringement with respect to this software. You may use this TS-UNB-Lib software or modifications
thereto only for purposes that are authorized by appropriate patent licenses.


4. DISCLAIMER

This TS-UNB-Lib software is provided by Fraunhofer on behalf of the copyright holders and contributors
"AS IS" and WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, including but not limited to the implied warranties
of merchantability and fitness for a particular purpose. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
CONTRIBUTORS BE LIABLE for any direct, indirect, incidental, special, exemplary, or consequential damages,
including but not limited to procurement of substitute goods or services; loss of use, data, or profits,
or business interruption, however caused and on any theory of liability, whether in contract, strict
liability, or tort (including negligence), arising in any way out of the use of this software, even if
advised of the possibility of such damage.


5. CONTACT INFORMATION

Fraunhofer Institute for Integrated Circuits IIS
Attention: Division Communication Systems
Am Wolfsmantel 33
91058 Erlangen, Germany
ks-contracts@iis.fraunhofer.de

This file is part of a Third-Party Modified Version of the Fraunhofer TS-UNB-Lib.
Modifications by mioty Alliance e.V. (2025)

----------------------------------------------------------------------------- */

/**
 * @brief	TS-UNB abstractions for Raspberry Pi Pico
 *
 * @authors	Joerg Robert, Augusto Kloth
 * @file	RPPicoTsUnb.h
 *
 */



#ifndef RPPICO_TSUNB_H_
#define RPPICO_TSUNB_H_

#include <inttypes.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <pico/binary_info.h>
#include <hardware/spi.h>
#include <hardware/irq.h>
#include <hardware/regs/intctrl.h>

#include "../../src/config/board_config.hpp"

#include "../TsUnb/RadioBurst.h"
#include "../TsUnb/FixedMac.h"
#include "../TsUnb/Phy.h"
#include "../TsUnb/SimpleNode.h"

// Include board configuration for GPIO pin definitions
#include "../../../src/config/board_config.hpp"

/**
 * @brief SPI interface used for communication with the transceiver (from board config)
 */
#define SPI_INTERFACE	MIOTY_SPI_INTERFACE

/**
 * @brief SPI baud rate for communication (from board config)
 */
#define SPI_BAUDRATE	Board::Comm::MIOTY_SPI_BAUDRATE

namespace TsUnbLib {
namespace RPPico {


//! Flag to indicate end of timer
extern volatile absolute_time_t TimeAddedDelay;
extern volatile bool TsUnbTimerFlag;
extern volatile bool ExtraDelaySet;
extern volatile float preciseTsUnbTimer_us;
extern volatile float TsUnbBitDuration_us;
extern volatile int64_t TsUnbTimeNextCycle_us;


/**
 * @brief Interrupt function for compare match of timer to set TimerFlag
 */
extern int64_t timer_callback(alarm_id_t id, void *user_data);

/**
 * @brief Platform dependent TS-UNB implementation for ATMega328p based Arduino systems.
 *
 * This class implents all plaform dependent methods for TS-UNB.
 * It mainly offer SPI communication and a timer to generate the TS-UNB symbol clock.
 *
 * @param CS_PIN              SPI chip select pin (default is 17)
 * @param SYMBOL_RATE_MULT    TS-UNB, symbol rate in multiples of 49.591064453125, set for 48 for 2380.371sym/s and 8 for 396.729sym/s. For higher rates the clock divider of the timer may have to be adjusted.
 * @param SPI_INIT            Init SPI before use and de-init after use (default is true).
 */
template <uint16_t SYMBOL_RATE_MULT = 48>
class RPPicoTsUnb {
public:
	RPPicoTsUnb() {
	}

	~RPPicoTsUnb() {
	}

	/**
	 * @brief Bit duration in microseconds
	 *
	 * This value is the duration of a single TS-UNB symbol, e.g. 1/2380.372s for the normal mode.
	 *
	 */
	static constexpr float TS_UNB_BIT_DURATION_US = (double) 1000000 / (49.591064453125 * (double)SYMBOL_RATE_MULT);


	/**
	 * @brief Init the timer
	 */
	void initTimer() {
		preciseTsUnbTimer_us = 0;
		TsUnbBitDuration_us = TS_UNB_BIT_DURATION_US;
	}

	/**
	 * @brief Start the timer
	 */
	void startTimer() {	
		if(preciseTsUnbTimer_us == 0){
			preciseTsUnbTimer_us += TS_UNB_BIT_DURATION_US;
		}

		ExtraDelaySet = false;
		TsUnbTimerFlag = false;	

		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;	

		alarm_id = add_alarm_in_us((uint64_t)TsUnbTimeNextCycle_us, timer_callback, NULL, true);
	}



	/**
	 * @brief Stop the timer
	 */
	void stopTimer() {
		cancel_alarm(alarm_id);
	}

	/**
	 * @brief Add the counter compare value for the next interrupt
	 *
	 * @param count Delay in TX symbols
	 */
	void addTimerDelay(const int32_t count) {
		TimeAddedDelay = get_absolute_time();
		preciseTsUnbTimer_us += TS_UNB_BIT_DURATION_US * (count-1);
		ExtraDelaySet = true;		
	}

	/**
	 * @brief Wait until the timer values expires
	 */
	void waitTimer() const {
		//TODO check if timer is really running
		sleep_us(TsUnbTimeNextCycle_us-10);			
		while (true){
			if (TsUnbTimerFlag){
				break;
			}
		}
		TsUnbTimerFlag = false;
	}

	/**
	 * @brief Initialization of the SPI interface
	 */
	void spiInit(void) {
		gpio_init(Board::GPIO::MIOTY_SPI_CS);
		gpio_set_dir(Board::GPIO::MIOTY_SPI_CS, GPIO_OUT);
		gpio_put(Board::GPIO::MIOTY_SPI_CS, 1);

		gpio_set_function(Board::GPIO::MIOTY_SPI_MISO, GPIO_FUNC_SPI);
		gpio_set_function(Board::GPIO::MIOTY_SPI_SCK, GPIO_FUNC_SPI);
		gpio_set_function(Board::GPIO::MIOTY_SPI_MOSI, GPIO_FUNC_SPI);

		spi_init(SPI_INTERFACE, SPI_BAUDRATE);
	}
	
	/**
	 * @brief Deinitialization of the SPI interface
	 */
	void spiDeinit(void) {
		spi_deinit(SPI_INTERFACE);
		gpio_init(Board::GPIO::MIOTY_SPI_MISO);
		gpio_init(Board::GPIO::MIOTY_SPI_SCK);
		gpio_init(Board::GPIO::MIOTY_SPI_MOSI);
		gpio_init(Board::GPIO::MIOTY_SPI_CS);
	}


	/**
	 * @brief Sends multiple bytes using SPI and sets the slave select pin accordingly
	 * 
	 * @param dataOut Bytes to be transmitted
	 * @param numBytes Number of bytes to be transmitted
	 */
	void spiSend(const uint8_t* const dataOut, const uint8_t numBytes) {
		gpio_put(Board::GPIO::MIOTY_SPI_CS, 0);
		spi_write_blocking(SPI_INTERFACE, dataOut, numBytes);
		gpio_put(Board::GPIO::MIOTY_SPI_CS, 1);
	}

	/**
	 * @brief Sends multiple and receives bytes using SPI and sets the slave select pin accordingly
	 *
	 * This method write and reads the SPI data. Please not that the read data has a delay of one byte.
	 * Hence, the first returned byte normally has no meaning.
	 * 
	 * @param dataInOut Bytes to be transmitted and buffer containing the read data
	 * @param numBytes  Number of bytes to be transmitted
	 */
	void spiSendReceive(uint8_t* const dataInOut, const uint8_t numBytes) {
		// Allocate memory as we cannot read write on the same memory
		uint8_t readData[numBytes];

		gpio_put(Board::GPIO::MIOTY_SPI_CS, 0);
		spi_write_read_blocking(SPI_INTERFACE, dataInOut, readData, numBytes);
		gpio_put(Board::GPIO::MIOTY_SPI_CS, 1);

		for(int i=0;i<numBytes;i++) {
			dataInOut[i] = readData[i];
		}

	}

	alarm_id_t alarm_id;	


	/**
	 * @brief Reset watchdog (just stub, not implemented)
	 * 
	 */
	void resetWatchdog() {};
};


};	// namespace RPPico
};	// namespace TsUnbLib

#include "RPPicoTsUnbTemplates.h"

#endif 	// RPPICO_TSUNB_H_


