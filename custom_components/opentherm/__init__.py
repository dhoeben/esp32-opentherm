import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins
from esphome.components import sensor

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor"]

opentherm_ns = cg.esphome_ns.namespace("opentherm")
OpenThermComponent = opentherm_ns.class_("OpenThermComponent", cg.Component)

CONF_IN_PIN = "in_pin"
CONF_OUT_PIN = "out_pin"
CONF_POLL_INTERVAL = "poll_interval"
CONF_RX_TIMEOUT = "rx_timeout"
CONF_DEBUG = "debug"

CONF_BOILER_TEMP = "boiler_temp"
CONF_RETURN_TEMP = "return_temp"
CONF_MODULATION = "modulation"
CONF_SETPOINT = "setpoint"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(OpenThermComponent),
    cv.Required(CONF_IN_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_OUT_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_POLL_INTERVAL, default="10s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_RX_TIMEOUT, default="40ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_DEBUG, default=False): cv.boolean,
    cv.Optional(CONF_BOILER_TEMP): sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1),
    cv.Optional(CONF_RETURN_TEMP): sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1),
    cv.Optional(CONF_MODULATION): sensor.sensor_schema(unit_of_measurement="%", accuracy_decimals=0),
    cv.Optional(CONF_SETPOINT): sensor.sensor_schema(unit_of_measurement="°C", accuracy_decimals=1),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    in_pin = await cg.gpio_pin_expression(config[CONF_IN_PIN])
    out_pin = await cg.gpio_pin_expression(config[CONF_OUT_PIN])
    cg.add(var.set_pins(in_pin, out_pin))
    cg.add(var.set_poll_interval(config[CONF_POLL_INTERVAL]))
    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    cg.add(var.set_debug(config[CONF_DEBUG]))

    
    # Bind sensors from YAML to internal sensors
    if CONF_BOILER_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_BOILER_TEMP])
        cg.add(var.set_boiler_temp_sensor(sens))
    if CONF_RETURN_TEMP in config:
        sens = await sensor.new_sensor(config[CONF_RETURN_TEMP])
        cg.add(var.set_return_temp_sensor(sens))
    if CONF_MODULATION in config:
        sens = await sensor.new_sensor(config[CONF_MODULATION])
        cg.add(var.set_modulation_sensor(sens))
    if CONF_SETPOINT in config:
        sens = await sensor.new_sensor(config[CONF_SETPOINT])
        cg.add(var.set_setpoint_sensor(sens))
