
#include "clock.h"
#include "i2c.h"

typedef void @far (*interrupt_handler_t)(void);

struct interrupt_vector {
	unsigned char interrupt_instruction;
	interrupt_handler_t interrupt_handler;
};

@far @interrupt void NonHandledInterrupt (void) {
	// in order to detect unexpected events during development, 
	// it is recommended to set a breakpoint on the following instruction
	return;
}

extern void _stext();     /* startup routine */

struct interrupt_vector const _vectab[] = {
	{0x82, (interrupt_handler_t)_stext}, /* reset */
	{0x82, NonHandledInterrupt},         /* trap  */

  // ITC->ISPR1
	{0x82, NonHandledInterrupt}, /* irq0  */
	{0x82, NonHandledInterrupt}, /* irq1  */
	{0x82, NonHandledInterrupt}, /* irq2  */
	{0x82, NonHandledInterrupt}, /* irq3  */
	
  // ITC->ISPR2
  {0x82, NonHandledInterrupt}, /* irq4  */
	{0x82, NonHandledInterrupt}, /* irq5  */
	{0x82, NonHandledInterrupt}, /* irq6  */
	{0x82, NonHandledInterrupt}, /* irq7  */

  // ITC->ISPR3
	{0x82, NonHandledInterrupt}, /* irq8  */
	{0x82, NonHandledInterrupt}, /* irq9  */
	{0x82, NonHandledInterrupt}, /* irq10 */
	{0x82, NonHandledInterrupt}, /* irq11 */

  // ITC->ISPR4
	{0x82, NonHandledInterrupt}, /* irq12 */
	{0x82, NonHandledInterrupt}, /* irq13 */
	{0x82, NonHandledInterrupt}, /* irq14 */
	{0x82, NonHandledInterrupt}, /* irq15 */

  // ITC->ISPR5
	{0x82, NonHandledInterrupt}, /* irq16 */
	{0x82, NonHandledInterrupt}, /* irq17 */
	{0x82, NonHandledInterrupt}, /* irq18 */

  // I2C interrupt
  // runs at priority 2 (ITC_SPR5VECT19SPR, bits 0xc0)
	{0x82, i2cIntHandler},       /* irq19 I2C*/
  
  // ITC->ISPR6
  {0x82, NonHandledInterrupt}, /* irq20 */
	{0x82, NonHandledInterrupt}, /* irq21 */	
  {0x82, NonHandledInterrupt}, /* irq22 */

  // clock timer (int every 128 usecs)
  // runs at priority 1 (ITC_SPR6VECT23SPR, bits 0xc0)
	{0x82, tim4IntHandler},      /* irq23 TIM4 */

  // ITC->ISPR7
	{0x82, NonHandledInterrupt}, /* irq24 */
};
