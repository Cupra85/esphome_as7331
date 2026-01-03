import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

# Eigene Keys (NICHT aus esphome.const importieren!)
CONF_UVA = "uva"
CONF_UVB = "uvb"
CONF_UVC = "uvc"
CONF_OUTCONV = "outconv"

CONF_GAIN = "gain"
CONF_CONVERSION_TIME = "conversion_time"
CONF_CCLK = "cclk"
CONF_MEAS_MODE = "measurement_mode"
CONF_DIVIDER = "divider"
CONF_ENABLE_DIVIDER = "enable_divider"
CONF_TEMP_CONV_ENABLED = "temp_conversion_enabled"

DEPENDENCIES = ["i2c"]

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

# Mappings (Enums)
GAIN_MAP = {
    2048: 0, 1024: 1, 512: 2, 256: 3, 128: 4, 64: 5,
    32: 6, 16: 7, 8: 8, 4: 9, 2: 10, 1: 11,
}

TIME_MAP_MS = {
    1: 0, 2: 1, 4: 2, 8: 3, 16: 4, 32: 5,
    64: 6, 128: 7, 256: 8, 512: 9,
    1024: 10, 2048: 11, 4096: 12,
    8192: 13, 16384: 14,
}

CCLK_MAP_MHZ = {
    1.024: 0,
    2.048: 1,
    4.096: 2,
    8.192: 3,
}

MEAS_MODE_MAP = {
    "cont": 0,
    "cmd": 1,
    "syns": 2,
    "synd": 3,
}

DIVIDER_MAP = {
    2: 0, 4: 1, 8: 2, 16: 3,
    32: 4, 64: 5, 128: 6, 256: 7,
}

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),

            cv.Optional(CONF_UVA): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_UVB): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_UVC): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_OUTCONV): sensor.sensor_schema(
                unit_of_measurement="ticks",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),

            cv.Optional(CONF_GAIN, default=2): cv.one_of(*GAIN_MAP.keys(), int=True),
            cv.Optional(CONF_CONVERSION_TIME, default=64): cv.one_of(*TIME_MAP_MS.keys(), int=True),
            cv.Optional(CONF_CCLK, default=1.024): cv.one_of(*CCLK_MAP_MHZ.keys(), float=True),
            cv.Optional(CONF_MEAS_MODE, default="cmd"): cv.one_of(*MEAS_MODE_MAP.keys(), lower=True),

            cv.Optional(CONF_ENABLE_DIVIDER, default=False): cv.boolean,
            cv.Optional(CONF_DIVIDER, default=2): cv.one_of(*DIVIDER_MAP.keys(), int=True),

            cv.Optional(CONF_TEMP_CONV_ENABLED, default=True): cv.boolean,
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x74)),
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_UVA in config:
        sens = await sensor.new_sensor(config[CONF_UVA])
        cg.add(var.set_uva_sensor(sens))

    if CONF_UVB in config:
        sens = await sensor.new_sensor(config[CONF_UVB])
        cg.add(var.set_uvb_sensor(sens))

    if CONF_UVC in config:
        sens = await sensor.new_sensor(config[CONF_UVC])
        cg.add(var.set_uvc_sensor(sens))

    if CONF_TEMPERATURE in config:
        sens = await sensor.new_sensor(config[CONF_TEMPERATURE])
        cg.add(var.set_temperature_sensor(sens))

    if CONF_OUTCONV in config:
        sens = await sensor.new_sensor(config[CONF_OUTCONV])
        cg.add(var.set_outconv_sensor(sens))

    cg.add(var.set_gain(GAIN_MAP[config[CONF_GAIN]]))
    cg.add(var.set_conversion_time(TIME_MAP_MS[config[CONF_CONVERSION_TIME]]))
    cg.add(var.set_cclk(CCLK_MAP_MHZ[config[CONF_CCLK]]))
    cg.add(var.set_measurement_mode(MEAS_MODE_MAP[config[CONF_MEAS_MODE]]))
    cg.add(var.set_enable_divider(config[CONF_ENABLE_DIVIDER]))
    cg.add(var.set_divider(DIVIDER_MAP[config[CONF_DIVIDER]]))
    cg.add(var.set_temp_conversion_enabled(config[CONF_TEMP_CONV_ENABLED]))

# Sensor-Plattform registrieren
PLATFORM_SCHEMA = CONFIG_SCHEMA
