import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID, STATE_CLASS_MEASUREMENT

DEPENDENCIES = ["i2c"]

CONF_UVA = "uva"
CONF_UVB = "uvb"
CONF_UVC = "uvc"

CONF_UVA_IRR = "uva_irradiance"
CONF_UVB_IRR = "uvb_irradiance"
CONF_UVC_IRR = "uvc_irradiance"

CONF_UVA_MULT = "uva_multiplier"
CONF_UVB_MULT = "uvb_multiplier"
CONF_UVC_MULT = "uvc_multiplier"

CONF_GAIN = "gain"
CONF_CONVERSION_TIME = "conversion_time"
CONF_CCLK = "cclk"
CONF_MEAS_MODE = "measurement_mode"
CONF_ENABLE_DIVIDER = "enable_divider"
CONF_DIVIDER = "divider"

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

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

CCLK_MAP_MHZ = {1.024: 0, 2.048: 1, 4.096: 2, 8.192: 3}
MEAS_MODE_MAP = {"cont": 0, "cmd": 1}
DIVIDER_MAP = {2: 0, 4: 1, 8: 2, 16: 3, 32: 4, 64: 5, 128: 6, 256: 7}

CONFIG_SCHEMA = cv.Schema(
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

        cv.Optional(CONF_UVA_IRR): sensor.sensor_schema(
            unit_of_measurement="µW/cm²",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_UVB_IRR): sensor.sensor_schema(
            unit_of_measurement="µW/cm²",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),
        cv.Optional(CONF_UVC_IRR): sensor.sensor_schema(
            unit_of_measurement="µW/cm²",
            accuracy_decimals=3,
            state_class=STATE_CLASS_MEASUREMENT,
        ),

        cv.Optional(CONF_UVA_MULT, default=0.0): cv.float_,
        cv.Optional(CONF_UVB_MULT, default=0.0): cv.float_,
        cv.Optional(CONF_UVC_MULT, default=0.0): cv.float_,

        cv.Optional(CONF_GAIN, default=16): cv.one_of(*GAIN_MAP.keys(), int=True),
        cv.Optional(CONF_CONVERSION_TIME, default=256): cv.one_of(*TIME_MAP_MS.keys(), int=True),
        cv.Optional(CONF_CCLK, default=1.024): cv.one_of(*CCLK_MAP_MHZ.keys(), float=True),
        cv.Optional(CONF_MEAS_MODE, default="cont"): cv.one_of("cont", "cmd"),

        cv.Optional(CONF_ENABLE_DIVIDER, default=False): cv.boolean,
        cv.Optional(CONF_DIVIDER, default=2): cv.one_of(*DIVIDER_MAP.keys(), int=True),
    }
).extend(cv.polling_component_schema("5s")).extend(i2c.i2c_device_schema(0x77))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_UVA in config:
        cg.add(var.set_uva_sensor(await sensor.new_sensor(config[CONF_UVA])))
    if CONF_UVB in config:
        cg.add(var.set_uvb_sensor(await sensor.new_sensor(config[CONF_UVB])))
    if CONF_UVC in config:
        cg.add(var.set_uvc_sensor(await sensor.new_sensor(config[CONF_UVC])))

    if CONF_UVA_IRR in config:
        cg.add(var.set_uva_irr_sensor(await sensor.new_sensor(config[CONF_UVA_IRR])))
    if CONF_UVB_IRR in config:
        cg.add(var.set_uvb_irr_sensor(await sensor.new_sensor(config[CONF_UVB_IRR])))
    if CONF_UVC_IRR in config:
        cg.add(var.set_uvc_irr_sensor(await sensor.new_sensor(config[CONF_UVC_IRR])))

    cg.add(var.set_uva_multiplier(config[CONF_UVA_MULT]))
    cg.add(var.set_uvb_multiplier(config[CONF_UVB_MULT]))
    cg.add(var.set_uvc_multiplier(config[CONF_UVC_MULT]))

    cg.add(var.set_gain(GAIN_MAP[config[CONF_GAIN]]))
    cg.add(var.set_conversion_time(TIME_MAP_MS[config[CONF_CONVERSION_TIME]]))
    cg.add(var.set_cclk(CCLK_MAP_MHZ[config[CONF_CCLK]]))
    cg.add(var.set_measurement_mode(MEAS_MODE_MAP[config[CONF_MEAS_MODE]]))

    cg.add(var.set_enable_divider(config[CONF_ENABLE_DIVIDER]))
    cg.add(var.set_divider(DIVIDER_MAP[config[CONF_DIVIDER]]))

PLATFORM_SCHEMA = CONFIG_SCHEMA
