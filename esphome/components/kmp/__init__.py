import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@cenobitedk"]

DEPENDENCIES = ["uart"]

kmp_ns = cg.esphome_ns.namespace("kmp")
Kmp = kmp_ns.class_("KMP", cg.Component, uart.UARTDevice)

CONF_UART_ID = "uart_id"
CONF_TIMEOUT = "timeout"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Kmp),
        cv.Required(CONF_UART_ID): cv.string,
        cv.Optional(CONF_TIMEOUT, default=200): cv.int_,
    }
).extend(cv.COMPONENT_SCHEMA)
