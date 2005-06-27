/*!
 * \file
 * <!--
 * Copyright 2003, 2004, 2005 Develer S.r.l. (http://www.develer.com/)
 * Copyright 1999, 2003 Bernardo Innocenti
 * This file is part of DevLib - See README.devlib for information.
 * -->
 *
 * \version $Id$
 *
 * \author Bernardo Innocenti <bernie@develer.com>
 * \author Stefano Fedrigo <aleph@develer.com>
 * \author Francesco Sacchi <batt@develer.com>
 *
 * \brief Keyboard driver (implementation)
 */

/*#*
 *#* $Log$
 *#* Revision 1.1  2005/06/27 21:28:45  bernie
 *#* Import generic keyboard driver.
 *#*
 *#*/

#include <hw_kbd.h>

#include <drv/timer.h>
#include <drv/kbd.h>
#include <drv/buzzer.h>

#include <cfg/debug.h>




#define KBD_CHECK_INTERVAL  10  /*!< (ms) Timing for kbd softint */
#define KBD_DEBOUNCE_TIME   30  /*!< (ms) Debounce time */
#define KBD_BEEP_TIME        5  /*!< (ms) Duration of keybeep */

#define KBD_REPEAT_DELAY   400  /*!< (ms) Keyboard repeat delay for first character */
#define KBD_REPEAT_RATE    100  /*!< (ms) Initial interchar delay for keyboard repeat */
#define KBD_REPEAT_MAXRATE  20  /*!< (ms) Minimum delay for keyboard repeat */
#define KBD_REPEAT_ACCEL     5  /*!< (ms) Keyboard repeat speed increase */

#define KBD_LNG_DELAY     1000  /*!< (ms) Keyboard long pression keys delay */


/*! Status for keyboard repeat state machine */
static enum { KS_IDLE, KS_REPDELAY, KS_REPEAT } kbd_rptStatus;


static volatile keymask_t kbd_buf; /*!< Single entry keyboard buffer */
static volatile keymask_t kbd_cnt; /*!< Number of keypress events in \c kbd_buf */

static Timer kbd_timer;            /*!< KeyboardKBD_BEEP_TIME softtimer */

static List kbd_rawHandlers;       /*!< Raw keyboard handlers */
static List kbd_handlers;          /*!< Cooked keyboard handlers */

static KbdHandler kbd_defHandler;  /*!< The default keyboard handler */
static KbdHandler kbd_debHandler;  /*!< The debounce keyboard handler */
static KbdHandler kbd_rptHandler;  /*!< Auto-repeat keyboard handler */
#ifdef  K_LNG_MASK
static KbdHandler kbd_lngHandler;  /*!< Long pression keys handler */
#endif



/*!
 * Keyboard soft-irq handler.
 */
static void kbd_softint(UNUSED_ARG(iptr_t, arg))
{
	/*! Currently depressed key */
	static keymask_t current_key;

	struct KbdHandler *handler;
	keymask_t key = kbd_readkeys();

	/* Call raw input handlers */
	FOREACHNODE(handler, &kbd_rawHandlers)
		key = handler->hook(key);

	/* If this key was not previously pressed */
	if (key != current_key)
	{
		/* Remember last key */
		current_key = key;

		/* Call cooked input handlers */
		FOREACHNODE(handler, &kbd_handlers)
			key = handler->hook(key);
	}

	timer_add(&kbd_timer);
}


/*!
 * \brief Read a key from the keyboard buffer.
 *
 * When a key is kept depressed between calls of this function a value
 * is returned only after the time specified with KBD_REPAT_DELAY to
 * avoid too fast keyboard repeat.
 *
 * \note This function is \b not interrupt safe!
 *
 * \return The mask of depressed keys or 0 if no keys are depressed.
 *
 */
keymask_t kbd_peek(void)
{
	keymask_t key = 0;

	/* Extract an event from the keyboard buffer */
	IRQ_DISABLE;
	if (kbd_cnt)
	{
		--kbd_cnt;
		key = kbd_buf;
	}
	IRQ_ENABLE;

	return key;
}


/*!
 * Wait for a keypress and return the mask of depressed keys.
 *
 * \note This function is \b not interrupt safe!
 */
keymask_t kbd_get(void)
{
	keymask_t key;

	while (!(key = kbd_peek())) {}
	return key;
}


/*!
 * Wait up to \c timeout ms for a keypress
 * and return the mask of depressed keys, or K_TIMEOUT
 * if the timeout was reacked.
 */
keymask_t kbd_get_timeout(mtime_t timeout)
{
	keymask_t key;

	ticks_t start = timer_clock();
	ticks_t stop  = ms_to_ticks(timeout);
	do
	{
		if ((key = kbd_peek()))
			return key;
	}
	while (timer_clock() - start < stop);

	return K_TIMEOUT;
}


void kbd_addHandler(struct KbdHandler *handler)
{
	KbdHandler *node;
	List *list;

	cpuflags_t flags;
	IRQ_SAVE_DISABLE(flags);

	/* Choose between raw and coocked handlers list */
	list = (handler->flags & KHF_RAWKEYS) ?
		&kbd_rawHandlers : &kbd_handlers;

	/*
	 * Search for the first node whose priority
	 * is lower than the timer we want to add.
	 */
	FOREACHNODE(node,list)
		if (node->pri < handler->pri)
			break;

	/* Enqueue handler in the handlers chain */
	INSERTBEFORE(&handler->link, &node->link);

	IRQ_RESTORE(flags);
}


void kbd_remHandler(struct KbdHandler *handler)
{
	/* Remove the handler */
	ATOMIC(REMOVE(&handler->link));
}


/*!
 * This is the default key handler, called after
 * all other handlers have had their chance to
 * do their special processing. This handler
 * pushes all input in the keyboard FIFO buffer.
 */
static keymask_t kbd_defHandlerFunc(keymask_t key)
{
	if (key)
	{
		/* Force a single event in kbd buffer */
		kbd_buf = key;
		kbd_cnt = 1;

		if (!(key & K_REPEAT))
			buz_beep(KBD_BEEP_TIME);
	}

	/* Eat all input */
	return 0;
}

/*!
 * Handle keyboard debounce
 */
static keymask_t kbd_debHandlerFunc(keymask_t key)
{
	/*! Buffer for debounce */
	static keymask_t debounce_key;

	/*! Timer for keyboard debounce */
	static ticks_t debounce_time;

	/*! Key aquired after debounce */
	static keymask_t new_key;


	ticks_t now = timer_clock();

	if (key != debounce_key)
	{
		/* Reset debounce timer */
		debounce_key = key;
		debounce_time = now;
	}
	else if ((new_key != debounce_key)
		&& (now - debounce_time > ms_to_ticks(KBD_DEBOUNCE_TIME)))
	{
		new_key = debounce_key;
		debounce_time = now;
	}

	return new_key;
}

#ifdef  K_LNG_MASK
/*!
 * Handle long pression keys
 */
static keymask_t kbd_lngHandlerFunc(keymask_t key)
{
	static ticks_t stop;
	ticks_t now = timer_clock();

	if (key & K_LNG_MASK)
	{
		if (now - stop > 0)
			key &= K_LNG_MASK;
		else
			key &= ~K_LNG_MASK;
	}
	else
		stop = now + ms_to_ticks(KBD_LNG_DELAY);
	return key;
}
#endif

/*!
 * Handle keyboard repeat
 */
static keymask_t kbd_rptHandlerFunc(keymask_t key)
{
	/*! Timer for keyboard repeat events */
	static ticks_t repeat_time;

	static ticks_t repeat_rate; /*! Current repeat rate (for acceleration) */

	ticks_t now = timer_clock();

	switch (kbd_rptStatus)
	{
		case KS_IDLE:
			if (key & K_RPT_MASK)
			{
				repeat_time = now;
				kbd_rptStatus = KS_REPDELAY;
			}
			break;

		case KS_REPDELAY:
			if (key & K_RPT_MASK)
			{
				if (now - repeat_time > ms_to_ticks(KBD_REPEAT_DELAY))
				{
					key = (key & K_RPT_MASK) | K_REPEAT;
					repeat_time = now;
					repeat_rate = ms_to_ticks(KBD_REPEAT_RATE);
					kbd_rptStatus = KS_REPEAT;
				}
				else
					key = 0;
			}
			else
				kbd_rptStatus = KS_IDLE;
			break;

		case KS_REPEAT:
			if (key & K_RPT_MASK)
			{
				if (now - repeat_time > repeat_rate)
				{
					/* Enqueue a new event in the buffer */
					key = (key & K_RPT_MASK) | K_REPEAT;
					repeat_time = now;

					/* Repeat rate acceleration */
					if (repeat_rate > ms_to_ticks(KBD_REPEAT_MAXRATE))
						repeat_rate -= ms_to_ticks(KBD_REPEAT_ACCEL);
				}
				else
					key = 0;
			}
			else
				kbd_rptStatus = KS_IDLE;

			break;
	}

	return key;
}



/*!
 * Initialize keyboard ports and softtimer
 */
void kbd_init(void)
{

	cpuflags_t flags;
	IRQ_SAVE_DISABLE(flags);

	KBD_HW_INIT;

	IRQ_RESTORE(flags);

	/* Init handlers lists */
	LIST_INIT(&kbd_handlers);
	LIST_INIT(&kbd_rawHandlers);

	/* Add debounce keyboard handler */
	kbd_debHandler.hook = kbd_debHandlerFunc;
	kbd_debHandler.pri = 100; /* high priority */
	kbd_debHandler.flags = KHF_RAWKEYS;
	kbd_addHandler(&kbd_debHandler);

	#ifdef  K_LNG_MASK
	/* Add long pression keyboard handler */
	kbd_lngHandler.hook = kbd_lngHandlerFunc;
	kbd_lngHandler.pri = 90; /* high priority */
	kbd_lngHandler.flags = KHF_RAWKEYS;
	kbd_addHandler(&kbd_lngHandler);
	#endif


	/* Add repeat keyboard handler */
	kbd_rptHandler.hook = kbd_rptHandlerFunc;
	kbd_rptHandler.pri = 80; /* high priority */
	kbd_rptHandler.flags = KHF_RAWKEYS;
	kbd_addHandler(&kbd_rptHandler);


	/* Add default keyboard handler */
	kbd_defHandler.hook = kbd_defHandlerFunc;
	kbd_defHandler.pri = -128; /* lowest priority */
	kbd_addHandler(&kbd_defHandler);

	/* Add kbd handler to soft timers list */
	event_initSoftInt(&kbd_timer.expire, kbd_softint, 0);
	timer_setDelay(&kbd_timer, ms_to_ticks(KBD_CHECK_INTERVAL));
	timer_add(&kbd_timer);
}
