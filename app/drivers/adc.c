#include <stdbool.h>

#include "drivers/gpio.h"
#include "drivers/adc.h"

ADC_HandleTypeDef adc_handle;
static bool m_initialized;
static uint16_t m_adc_values[NUM_OF_ADC_CHANNELS];
static uint32_t m_adc_current_index;
static uint32_t m_errors_count;
static adc_conversion_ready m_callback;

/* This is the actual IRQ handler. Need to forward it to the HAL. */
void ADC1_IRQHandler(void)
{
	HAL_ADC_IRQHandler(&adc_handle);
}

err_t adc_init(void)
{
	HAL_StatusTypeDef status;

	adc_handle.Instance = ADC1;
	adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	adc_handle.Init.Resolution = ADC_RESOLUTION_12B;
	adc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	adc_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;
	adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	adc_handle.Init.LowPowerAutoWait = DISABLE;
	adc_handle.Init.ContinuousConvMode = ENABLE;
	adc_handle.Init.NbrOfConversion = NUM_OF_ADC_CHANNELS;
	adc_handle.Init.DiscontinuousConvMode = DISABLE;
	adc_handle.Init.NbrOfDiscConversion = 1;
	adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	adc_handle.Init.DMAContinuousRequests = DISABLE;
	adc_handle.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	adc_handle.Init.OversamplingMode = ENABLE;
	adc_handle.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_16;
	adc_handle.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_NONE;
	adc_handle.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
	adc_handle.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;

	status = HAL_ADC_Init(&adc_handle);
	HAL_ERR_CHECK(status, EADC_HAL_INIT);

	ADC_ChannelConfTypeDef adc_config = {
		.Channel = ADC_CHANNEL_5,
		.Rank = ADC_REGULAR_RANK_1,
		.SamplingTime = ADC_SAMPLETIME_640CYCLES_5,
		.SingleDiff = ADC_SINGLE_ENDED,
		.OffsetNumber = ADC_OFFSET_NONE,
		.Offset = 0,
	};

	status = HAL_ADC_ConfigChannel(&adc_handle, &adc_config);
	HAL_ERR_CHECK(status, EADC_HAL_CONFIG_CHANNEL);

	adc_config.Channel = ADC_CHANNEL_6;
	adc_config.Rank = ADC_REGULAR_RANK_2;
	status = HAL_ADC_ConfigChannel(&adc_handle, &adc_config);
	HAL_ERR_CHECK(status, EADC_HAL_CONFIG_CHANNEL);

	adc_config.Channel = ADC_CHANNEL_VREFINT;
	adc_config.Rank = ADC_REGULAR_RANK_3;
	status = HAL_ADC_ConfigChannel(&adc_handle, &adc_config);
	HAL_ERR_CHECK(status, EADC_HAL_CONFIG_CHANNEL);

	m_initialized = true;

	return ERR_OK;
}

err_t adc_start(void)
{
	if (!m_initialized)
		return EADC_NO_INIT;

	HAL_NVIC_EnableIRQ(ADC1_2_IRQn);
	HAL_ADC_Start_IT(&adc_handle);

	return ERR_OK;
}

err_t adc_stop(void)
{
	if (!m_initialized)
		return EADC_NO_INIT;

	HAL_ADC_Stop_IT(&adc_handle);
	HAL_NVIC_DisableIRQ(ADC1_2_IRQn);

	return ERR_OK;
}

/* Note that callback is called from interrupt context */
void adc_set_callback(adc_conversion_ready callback)
{
	m_callback = callback;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
	const bool is_end_of_sequence = hadc->Instance->ISR & ADC_ISR_EOS;

	m_adc_values[m_adc_current_index++] = HAL_ADC_GetValue(&adc_handle);

	/* If End Of Sequence is set, the last sample in our sequence is done. */
	if (is_end_of_sequence && m_adc_current_index >= NUM_OF_ADC_CHANNELS) {
		m_adc_current_index = 0;
		if (m_callback)
			m_callback(m_adc_values);
	} else if (m_adc_current_index >= NUM_OF_ADC_CHANNELS) {
		/* We must've missed at least one interrupt.
		 * Throw away old samples as they might be out of order and start over.
		 */
		m_adc_current_index = 0;
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
	m_errors_count++;
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
	GPIO_InitTypeDef gpio_config;

	if(adcHandle->Instance == ADC1) {
		__HAL_RCC_ADC_CLK_ENABLE();

		gpio_config.Pin = DUT_VDD_IN_Pin | DUT_VDD_OUT_Pin;
		gpio_config.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
		gpio_config.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &gpio_config);
	}
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{
	if(adcHandle->Instance == ADC1) {
		__HAL_RCC_ADC_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOA, DUT_VDD_IN_Pin | DUT_VDD_OUT_Pin);
	}
}
