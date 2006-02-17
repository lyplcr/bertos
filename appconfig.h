/*!
 * \file
 * <!--
 * Copyright 2003, 2004 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief DevLib configuration options
 *
 * You should copy this header in your project and rename it to
 * "config.h" and delete the CONFIG_ macros for the modules
 * you're not using.
 *
 * <h2>Working with multiple applications</h2>
 *
 * If your project is made of multiple DevLib-based applications,
 * create a custom "config.h" file in each application subdirectory
 * and play with the compiler include path to get the desired result.
 * You can share common options by creationg a "config_common.h" header
 * and including it from all your "config.h" copies.
 *
 * <h2>Configuration style</h2>
 *
 * For improved compile-time checking of configuration options,
 * the preferred way to use a \c CONFIG_ symbol is keeping it
 * always defined with a value of either 0 or 1.  This lets
 * you write tests like this:
 *
 * \code
 *  #if CONFIG_FOO
 *  void foo(void)
 *  {
 *      if (CONFIG_BAR)
 *          bar();
 *  }
 *  #endif // CONFIG_FOO
 * \endcode
 *
 * In most cases, we rely on the optimizer to discard checks
 * on constant values and performing dead-code elimination.
 *
 * \version $Id$
 * \author Bernardo Innocenti <bernie@develer.com>
 * \author Stefano Fedrigo <aleph@develer.com>
 */

/*#*
 *#* $Log$
 *#* Revision 1.7  2006/02/17 22:28:19  bernie
 *#* Add missing UART definitions.
 *#*
 *#* Revision 1.6  2006/02/15 09:12:56  bernie
 *#* Switch to BITMAP_FMT_PLANAR_V_LSB.
 *#*
 *#* Revision 1.5  2006/02/10 12:34:33  bernie
 *#* Add missing config options for gfx and kbd.
 *#*
 *#* Revision 1.4  2006/01/23 23:12:27  bernie
 *#* Enable CONFIG_GFX_VCOORDS.
 *#*
 *#* Revision 1.3  2006/01/17 02:30:06  bernie
 *#* Add new config vars.
 *#*
 *#* Revision 1.2  2005/11/27 03:04:57  bernie
 *#* CONFIG_WATCHDOG: New config option.
 *#*
 *#* Revision 1.1  2005/11/04 17:42:12  bernie
 *#* Move cfg/config.h to appconfig.h.
 *#*
 *#* Revision 1.1  2005/04/11 19:04:13  bernie
 *#* Move top-level headers to cfg/ subdir.
 *#*
 *#* Revision 1.5  2004/12/08 08:04:28  bernie
 *#* Add missing config options.
 *#*
 *#* Revision 1.4  2004/08/25 14:12:08  rasky
 *#* Aggiornato il comment block dei log RCS
 *#*
 *#* Revision 1.3  2004/08/24 14:30:11  bernie
 *#* Use new-style config macros for drv/timer.c
 *#*
 *#* Revision 1.2  2004/08/05 18:46:52  bernie
 *#* Documentation improvements.
 *#*
 *#* Revision 1.1  2004/07/29 23:34:32  bernie
 *#* Add template configuration file.
 *#*
 *#*/

#ifndef CONFIG_COMMON_H
#define CONFIG_COMMON_H

/*! Baud-rate for the kdebug console */
#define CONFIG_KDEBUG_BAUDRATE  19200

/*!
 * printf()-style formatter configuration.
 *
 * \sa PRINTF_DISABLED
 * \sa PRINTF_NOMODIFIERS
 * \sa PRINTF_REDUCED
 * \sa PRINTF_NOFLOAT
 * \sa PRINTF_FULL
 */
#define CONFIG_PRINTF PRINTF_FULL

/*!
 * Multithreading kernel
 *
 * /sa config_kernel.h
 */
#define CONFIG_KERNEL 0

/*!
 * \name Serial driver parameters
 * \{
 */
	/*! [bytes] Size of the outbound FIFO buffer for port 0. */
	#define CONFIG_UART0_TXBUFSIZE  32

	/*! [bytes] Size of the inbound FIFO buffer for port 0. */
	#define CONFIG_UART0_RXBUFSIZE  64

	/*! [bytes] Size of the outbound FIFO buffer for port 1. */
	#define CONFIG_UART1_TXBUFSIZE  32

	/*! [bytes] Size of the inbound FIFO buffer for port 1. */
	#define CONFIG_UART1_RXBUFSIZE  64

	/*! Default transmit timeout (ms). Set to -1 to disable timeout support */
	#define CONFIG_SER_TXTIMEOUT    -1

	/*! Default receive timeout (ms). Set to -1 to disable timeout support */
	#define CONFIG_SER_RXTIMEOUT    -1

	/*! Use RTS/CTS handshake */
	#define CONFIG_SER_HWHANDSHAKE   0

	/*! Default baud rate (set to 0 to disable) */
	#define CONFIG_SER_DEFBAUDRATE   0

	/*! Enable ser_gets() and ser_gets_echo() */
	#define CONFIG_SER_GETS          0

	/** Enable second serial port in emulator. */
	#define CONFIG_EMUL_UART1        0

	/*!
	 * Transmit always something on serial port 0 TX
	 * to avoid interference when sending burst of data,
	 * using AVR multiprocessor serial mode
	 */
	#define CONFIG_SER_TXFILL        0

	#define CONFIG_SER_STROBE        0
/*\}*/

/*!
 * \name KBus configuration
 * \{
 */
	/*! Board address for KBus */
	#define CONFIG_KBUS_ADDR KBUS_ADDR_FOOBAR

	/*! Disable KBUS escaping support */
	#define CONFIG_KBUS_ESCAPE  0

	/*! Serial port for internal KBUS communication */
	#define CONFIG_KBUS_PORT  0

	/*! Serial port speed for KBus communication */
	#define CONFIG_KBUS_BAUDRATE  19200
/*\}*/

//! Hardware timer selection for drv/timer.c
#define CONFIG_TIMER  TIMER_ON_OUTPUT_COMPARE2

//! Debug timer interrupt using a strobe pin.
#define CONFIG_TIMER_STROBE  0

//! Enable watchdog timer.
#define CONFIG_WATCHDOG  1

//! EEPROM type for drv/eeprom.c
#define CONFIG_EEPROM_TYPE EEPROM_24XX256

/// Select bitmap pixel format.
#define CONFIG_BITMAP_FMT  BITMAP_FMT_PLANAR_V_LSB

/// Enable line clipping algorithm.
#define CONFIG_GFX_CLIPPING 1

/// Enable text rendering in bitmaps.
#define CONFIG_GFX_TEXT 1

/// Enable virtual coordinate system.
#define CONFIG_GFX_VCOORDS 1

/// Keyboard polling method
#define CONFIG_KBD_POLL  KBD_POLL_SOFTINT

/// Enable button bar behind menus
#define CONFIG_MENU_MENUBAR  0

#endif /* CONFIG_COMMON_H */
