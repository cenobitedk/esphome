import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

CODEOWNERS = ["@cenobitedk"]

DEPENDENCIES = ["uart"]

CONF_DECRYPTION_KEY = "decryption_key"

em1023_ns = cg.esphome_ns.namespace("em1023")
EM1023 = em1023_ns.class_("EM1023Component", cg.Component)
# MULTI_CONF = True

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(EM1023),
            cv.Optional(CONF_DECRYPTION_KEY, default=""): cv.string,
        }
    ).extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    # uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_DECRYPTION_KEY])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)
