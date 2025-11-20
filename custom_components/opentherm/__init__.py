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

    # Link YAML sensors
    cv.Optional(CONF_BOILER_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_RETURN_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_MODULATION):  cv.use_id(sensor.Sensor),
    cv.Optional(CONF_SETPOINT):    cv.use_id(sensor.Sensor),

    # External diagnostic sensors
    cv.Optional(CONF_HA_WEATHER_TEMP): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_HA_INDOOR_TEMP):  cv.use_id(sensor.Sensor),
    cv.Optional(CONF_ADAPTIVE_INDOOR): cv.use_id(sensor.Sensor),
    cv.Optional(CONF_DHW_FLOW_RATE):   cv.use_id(sensor.Sensor),

    # Numbers (new style 2025.x)
    cv.Optional("max_boiler_temp_heating"): number.number_schema(number.Number),
    cv.Optional("max_boiler_temp_water"):   number.number_schema(number.Number),

    # Equitherm tuning numbers
    cv.Optional("eq_fb_gain"): number.number_schema(number.Number),
    cv.Optional("eq_k"):       number.number_schema(number.Number),
    cv.Optional("eq_n"):       number.number_schema(number.Number),
    cv.Optional("eq_t"):       number.number_schema(number.Number),

    # Optional climate & switches
    cv.Optional("ch_climate"):   climate.climate_schema(climate.Climate),
    cv.Optional("emergency_mode"): switch.switch_schema(switch.Switch),
    cv.Optional("force_heat"):     switch.switch_schema(switch.Switch),
    cv.Optional("force_dhw"):      switch.switch_schema(switch.Switch),
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

    # Linked sensors
    for key, setter in [
        (CONF_BOILER_TEMP, "set_boiler_temp_sensor"),
        (CONF_RETURN_TEMP, "set_return_temp_sensor"),
        (CONF_MODULATION, "set_modulation_sensor"),
        (CONF_SETPOINT,   "set_setpoint_sensor"),
        (CONF_HA_WEATHER_TEMP, "set_ha_weather_sensor"),
        (CONF_HA_INDOOR_TEMP,  "set_ha_indoor_sensor"),
        (CONF_ADAPTIVE_INDOOR, "set_adaptive_indoor_sensor"),
        (CONF_DHW_FLOW_RATE,   "set_dhw_flow_rate_sensor"),
    ]:
        if key in config:
            s = await cg.get_variable(config[key])
            cg.add(getattr(var, setter)(s))

    # Numbers
    if "max_boiler_temp_heating" in config:
        n = await number.new_number(config["max_boiler_temp_heating"])
        cg.add(var.set_boiler_limit_number(n))

    if "max_boiler_temp_water" in config:
        n = await number.new_number(config["max_boiler_temp_water"])
        cg.add(var.set_dhw_limit_number(n))

    # Equitherm values
    for key in ["eq_fb_gain", "eq_k", "eq_n", "eq_t"]:
        if key in config:
            n = await number.new_number(config[key])
            cg.add(getattr(var, f"set_{key}_number")(n))

    # Optional climate
    if "ch_climate" in config:
        c = await climate.new_climate(config["ch_climate"])
        cg.add(var.set_climate_entity(c))

    # Optional switches
    for key, setter in [
        ("emergency_mode", "set_emergency_switch"),
        ("force_heat",     "set_force_heat_switch"),
        ("force_dhw",      "set_force_dhw_switch"),
    ]:
        if key in config:
            sw = await switch.new_switch(config[key])
            cg.add(getattr(var, setter)(sw))
