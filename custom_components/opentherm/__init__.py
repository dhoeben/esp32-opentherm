import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins
from esphome.components import sensor, number, switch, climate

DEPENDENCIES = ["wifi"]
AUTO_LOAD = ["sensor", "number", "switch", "climate"]

opentherm_ns = cg.esphome_ns.namespace("opentherm")
OpenThermComponent = opentherm_ns.class_("OpenThermComponent", cg.Component)

# Config keys
CONF_IN_PIN = "in_pin"
CONF_OUT_PIN = "out_pin"
CONF_POLL_INTERVAL = "poll_interval"
CONF_RX_TIMEOUT = "rx_timeout"
CONF_DEBUG = "debug"

CONF_BOILER_TEMP = "boiler_temp"
CONF_RETURN_TEMP = "return_temp"
CONF_MODULATION = "modulation"
CONF_SETPOINT = "setpoint"

CONF_HA_WEATHER_TEMP = "ha_weather_temp"
CONF_HA_INDOOR_TEMP  = "ha_indoor_temp"
CONF_ADAPTIVE_INDOOR = "adaptive_indoor_temp"
CONF_DHW_FLOW_RATE   = "dhw_flow_rate"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(OpenThermComponent),

    cv.Required(CONF_IN_PIN): pins.gpio_input_pin_schema,
    cv.Required(CONF_OUT_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_POLL_INTERVAL, default="10s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_RX_TIMEOUT,   default="40ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_DEBUG,        default=False): cv.boolean,

    # Link existing sensors by ID (declared in YAML)
    cv.Optional(CONF_BOILER_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_RETURN_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_MODULATION):  cv.use_id(sensor.Sensor),
    cv.Optional(CONF_SETPOINT):    cv.use_id(sensor.Sensor),

    # HA generated sensors
    cv.Optional(CONF_HA_WEATHER_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_HA_INDOOR_TEMP):  cv.use_id(sensor.Sensor),
    cv.Optional(CONF_ADAPTIVE_INDOOR): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_DHW_FLOW_RATE):   cv.use_id(sensor.Sensor),

    # Numbers (created here)
    cv.Optional("max_boiler_temp_heating"): number.NUMBER_SCHEMA,
    cv.Optional("max_boiler_temp_water"):   number.NUMBER_SCHEMA,

    # Equitherm tuning numbers (created here)
    cv.Optional("eq_fb_gain"): number.NUMBER_SCHEMA,
    cv.Optional("eq_k"):       number.NUMBER_SCHEMA,
    cv.Optional("eq_n"):       number.NUMBER_SCHEMA,
    cv.Optional("eq_t"):       number.NUMBER_SCHEMA,

    # Optional linked entities
    cv.Optional("ch_climate"):   climate.CLIMATE_SCHEMA,
    cv.Optional("emergency_mode"): switch.SWITCH_SCHEMA,
    cv.Optional("force_heat"):     switch.SWITCH_SCHEMA,
    cv.Optional("force_dhw"):      switch.SWITCH_SCHEMA,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Pins
    in_pin  = await cg.gpio_pin_expression(config[CONF_IN_PIN])
    out_pin = await cg.gpio_pin_expression(config[CONF_OUT_PIN])
    cg.add(var.set_pins(in_pin, out_pin))

    # Basic config
    cg.add(var.set_poll_interval(config[CONF_POLL_INTERVAL]))
    cg.add(var.set_rx_timeout(config[CONF_RX_TIMEOUT]))
    cg.add(var.set_debug(config[CONF_DEBUG]))

    # Link external YAML sensors
    if CONF_BOILER_TEMP in config:
        s = await cg.get_variable(config[CONF_BOILER_TEMP])
        cg.add(var.set_boiler_temp_sensor(s))
    if CONF_RETURN_TEMP in config:
        s = await cg.get_variable(config[CONF_RETURN_TEMP])
        cg.add(var.set_return_temp_sensor(s))
    if CONF_MODULATION in config:
        s = await cg.get_variable(config[CONF_MODULATION])
        cg.add(var.set_modulation_sensor(s))
    if CONF_SETPOINT in config:
        s = await cg.get_variable(config[CONF_SETPOINT])
        cg.add(var.set_setpoint_sensor(s))

    # External HA / diagnostic sensors
    if CONF_HA_WEATHER_TEMP in config:
        s = await cg.get_variable(config[CONF_HA_WEATHER_TEMP])
        cg.add(var.set_ha_weather_sensor(s))

    if CONF_HA_INDOOR_TEMP in config:
        s = await cg.get_variable(config[CONF_HA_INDOOR_TEMP])
        cg.add(var.set_ha_indoor_sensor(s))

    if CONF_ADAPTIVE_INDOOR in config:
        s = await cg.get_variable(config[CONF_ADAPTIVE_INDOOR])
        cg.add(var.set_adaptive_indoor_sensor(s))

    if CONF_DHW_FLOW_RATE in config:
        s = await cg.get_variable(config[CONF_DHW_FLOW_RATE])
        cg.add(var.set_dhw_flow_rate_sensor(s))

    # Numbers
    if "max_boiler_temp_heating" in config:
        n = await number.new_number(config["max_boiler_temp_heating"])
        cg.add(var.set_boiler_limit_number(n))
    if "max_boiler_temp_water" in config:
        n = await number.new_number(config["max_boiler_temp_water"])
        cg.add(var.set_dhw_limit_number(n))

    # Equitherm numbers
    for key in ["eq_fb_gain", "eq_k", "eq_n", "eq_t"]:
        if key in config:
            n = await number.new_number(config[key])
            cg.add(getattr(var, f"set_{key}_number")(n))

    # Optional climate and switches
    if "ch_climate" in config:
        c = await climate.new_climate(config["ch_climate"])
        cg.add(var.set_climate_entity(c))
    if "emergency_mode" in config:
        sw = await switch.new_switch(config["emergency_mode"])
        cg.add(var.set_emergency_switch(sw))
    if "force_heat" in config:
        sw = await switch.new_switch(config["force_heat"])
        cg.add(var.set_force_heat_switch(sw))
    if "force_dhw" in config:
        sw = await switch.new_switch(config["force_dhw"])
        cg.add(var.set_force_dhw_switch(sw))
