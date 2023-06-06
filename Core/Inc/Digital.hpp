/*
 * Digital.hpp
 *
 *  Created on: Jun 25, 2022
 *      Author: ahuijsen
 */

#ifndef DIGITAL_HPP_
#define DIGITAL_HPP_

#include "stm32f3xx_hal.h"

/**
 * Pin states
 */
typedef enum {
   LOW,
   HIGH,
} PinState_t;

/**
 * Output pin
 */
class IOutputPin
{
public:
   /**
    * Set output pin HIGH
    */
   virtual void set_high() = 0;

   /**
    * Set output pin LOW
    */
   virtual void set_low() = 0;

   /**
    * Set output pin to the desired state (HIGH or LOW)
    */
   virtual void set_state(const PinState_t state) = 0;
};

/**
 * Input pin
 */
class IInputPin
{
public:
   /**
    * Determine whether the input pin is HIGH
    */
   virtual bool is_high() = 0;

   /**
    * Determine whether the input pin is LOW
    */
   virtual bool is_low() = 0;

   /**
    * Get the state of the input pin (HIGH or LOW)
    */
   virtual PinState_t get_state() = 0;
};

/**
 * GPIO based output pin
 *
 * Example:
 *
 *    GPIO_OutputPin LedRed(GPIOC, GPIO_PIN_3);
 *    LedRed.set_low();
 */
class GPIO_OutputPin : public IOutputPin
{
public:
   GPIO_OutputPin(GPIO_TypeDef* port, const uint16_t pin) : _Port(port), _Pin(pin)
   {
      GPIO_InitTypeDef init_s = { 0 };
      init_s.Pin = _Pin;
      init_s.Mode = GPIO_MODE_OUTPUT_PP;
      init_s.Pull = GPIO_NOPULL;
      init_s.Speed = GPIO_SPEED_FREQ_LOW;
      HAL_GPIO_Init(_Port, &init_s);
   };

   void set_high()
   {
	  _Port->BSRR = (uint32_t) _Pin;
   }

   void set_low()
   {
	  _Port->BRR = (uint32_t) _Pin;
   }

   void set_state(const PinState_t state)
   {
	  if (state != LOW)
	  {
		  _Port->BSRR = (uint32_t) _Pin;
	  }
	  else
	  {
		  _Port->BRR = (uint32_t) _Pin;
	  }
   }

private:
   GPIO_TypeDef* _Port;
   const uint16_t _Pin;
};

/**
 * GPIO based input pin
 */
class GPIO_InputPin : IInputPin
{
public:
   GPIO_InputPin(GPIO_TypeDef* port, const uint32_t pin) : _Port(port), _Pin(pin)
   {
      GPIO_InitTypeDef init_s = { 0 };
      init_s.Pin = _Pin;
      init_s.Mode = GPIO_MODE_INPUT;
      init_s.Pull = GPIO_NOPULL;
      HAL_GPIO_Init(_Port, &init_s);
   };

   bool is_high()
   {
      return (_Port->IDR & _Pin) != (uint32_t)GPIO_PIN_RESET;
   }

   bool is_low()
   {
      return (_Port->IDR & _Pin) == (uint32_t)GPIO_PIN_RESET;
   }

   PinState_t get_state()
   {
      return ((_Port->IDR & _Pin) != (uint32_t) GPIO_PIN_RESET) ? HIGH : LOW;
   }

private:
   GPIO_TypeDef* _Port;
   const uint16_t _Pin;
};

#endif /* DIGITAL_HPP_ */
