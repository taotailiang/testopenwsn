/**
* @file    SDK_Configuration_Common.h
* @author  LowPower RF BU - AMG
* @version 1.3.0
* @date    10-July-2017
* @brief   Common configuration header file.
* @details
*
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
* TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
* DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
* FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
* CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOURCE CODE IS PROTECTED BY A LICENSE.
* FOR MORE INFORMATION PLEASE CAREFULLY READ THE LICENSE AGREEMENT FILE LOCATED
* IN THE ROOT DIRECTORY OF THIS FIRMWARE PACKAGE.
*
* <h2><center>&copy; COPYRIGHT 2017 STMicroelectronics</center></h2>
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __S2LP_PKT_TEST_COMMON_H
#define __S2LP_PKT_TEST_COMMON_H

#define USE_HIGH_BAND
//#define USE_MIDDLE_BAND


#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @addtogroup SDK_Examples
 * @{
 */

/**
 * @defgroup SDK_Configuration_Common
 * @{
 */

/**
 * @defgroup SDK_Configuration_Common                                    Configuration Common
 * @brief Radio and packet parameters definitions.
 * @details These parameters are in common between the device A and B.
 *
 * The user can change the configuration parameters editing these defines.
 * @{
 */


/**
 * @defgroup Configuration_Common_Exported_Constants                     Configuration Common Exported Constants
 * @{
 */


/*  Radio configuration parameters  */

#ifdef USE_MIDDLE_BAND
#define BASE_FREQUENCY              433000000
#endif

#ifdef USE_HIGH_BAND
#define BASE_FREQUENCY              868000000
#endif

#define MODULATION_SELECT           MOD_2FSK
#define DATARATE                    38400
#define FREQ_DEVIATION              20000
#define BANDWIDTH                   100000

#define POWER_DBM                   12/*.0*/


  
/*  Packet configuration parameters  */
#define PREAMBLE_LENGTH             PREAMBLE_BYTE(4)
#define SYNC_LENGTH                 SYNC_BYTE(4)
#define SYNC_WORD                   0x88888888
#define VARIABLE_LENGTH             S_ENABLE
#define EXTENDED_LENGTH_FIELD       S_DISABLE
#define CRC_MODE                    PKT_CRC_MODE_32BITS//PKT_CRC_MODE_8BITS//zyx20181120
#define EN_ADDRESS                  S_DISABLE
#define EN_FEC                      S_DISABLE
#define EN_WHITENING                S_ENABLE


/* Wake Up timer in ms for LDC mode */
#define WAKEUP_TIMER                100/*.0*/

   
#define PREAMBLE_BYTE(v)        (4*v)
#define SYNC_BYTE(v)            (8*v)
   
/**
 *@}
 */

/**
 *@}
 */

/**
 *@}
 */

/**
 *@}
 */

#ifdef __cplusplus
}
#endif

#endif

/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/
