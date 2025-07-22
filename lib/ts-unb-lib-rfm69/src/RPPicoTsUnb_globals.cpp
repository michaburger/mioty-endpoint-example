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
 * @file RPPicoTsUnb_globals.cpp
 * @brief Global variable definitions for TS-UNB library
 * 
 * This file contains the definitions of global variables that were originally
 * defined in the header file, causing multiple definition errors.
 */

#include "RPPicoTsUnb.h"

namespace TsUnbLib {
namespace RPPico {

//! Flag to indicate end of timer
volatile absolute_time_t TimeAddedDelay;
volatile bool TsUnbTimerFlag;
volatile bool ExtraDelaySet;
volatile float preciseTsUnbTimer_us;
volatile float TsUnbBitDuration_us;
volatile int64_t TsUnbTimeNextCycle_us;

/**
 * @brief Interrupt function for compare match of timer to set TimerFlag
 */
int64_t timer_callback(alarm_id_t id, void *user_data) {

	if(!ExtraDelaySet) { //fire and then reload timer for next cycle
		TsUnbTimerFlag = true;
		preciseTsUnbTimer_us += TsUnbBitDuration_us;
		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;		
	}
	else { //AddTimerDelay has already calculated, just adjusting the time
		TsUnbTimeNextCycle_us = (int64_t)(preciseTsUnbTimer_us + 0.5f);
		preciseTsUnbTimer_us -= (float)TsUnbTimeNextCycle_us;	
		ExtraDelaySet = false;
	}

	return -TsUnbTimeNextCycle_us; 	// Negative menas that delay between calls will be kept regardless of callback time 
}

} // namespace RPPico
} // namespace TsUnbLib
