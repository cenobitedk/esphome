import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_GAS,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_CUBIC_METER,
    UNIT_KILOWATT,
    UNIT_KILOWATT_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE,
    UNIT_VOLT,
)

CODEOWNERS = ["@cenobitedk"]

DEPENDENCIES = ["uart"]

CONF_TIMEOUT = "timeout"


kmp_ns = cg.esphome_ns.namespace("kmp")
Kmp = kmp_ns.class_("KMP", cg.Component, uart.UARTDevice)


def validate_register_id(value):
    return value


CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(Kmp),
            cv.Optional(CONF_TIMEOUT, default=200): cv.int_,
            cv.Optional(CONF_PM_2_5): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_CHEMICAL_WEAPON,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_PM25,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA),
)
