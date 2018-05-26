#pragma once

static __inline__ void gpio_write(uint32_t port, uint32_t pin, bool value)
{
	GPIO_TypeDef* GPIOx = (GPIO_TypeDef*) port;

	assert_param(IS_GPIO_PIN(pin));

	if(value)
		GPIOx->BSRR = pin;
	else
		GPIOx->BRR = pin;
}
