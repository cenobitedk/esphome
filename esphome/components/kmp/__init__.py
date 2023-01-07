import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_ADDRESS,
    CONF_ID,
    CONF_RECEIVE_TIMEOUT,
    CONF_UART_ID,
)

CODEOWNERS = ["@cenobitedk"]

DEPENDENCIES = ["uart"]

kmp_ns = cg.esphome_ns.namespace("kmp")
Kmp = kmp_ns.class_("Kmp", cg.Component, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Kmp),
            cv.Required(CONF_ADDRESS): cv.string,
            cv.Required(CONF_UART_ID): cv.entity_id,
            cv.Optional(
                CONF_RECEIVE_TIMEOUT, default="200ms"
            ): cv.positive_time_period_milliseconds,
        }
    ).extend(cv.polling_component_schema("60s"))
)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)

    cg.add(var.set_destination_address(config[CONF_ADDRESS]))
    cg.add(var.set_receive_timeout(config[CONF_RECEIVE_TIMEOUT].total_milliseconds))

    await cg.register_component(var, config)
