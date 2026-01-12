import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

CONF_UVA = "uva"
CONF_UVB = "uvb"
CONF_UVC = "uvc"
CONF_UVA_WM2 = "uva_wm2"
CONF_UVB_WM2 = "uvb_wm2"
CONF_UVC_WM2 = "uvc_wm2"
CONF_GAIN = "gain"
CONF_INT_TIME = "int_time"

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(AS7331Component),

            cv.Optional(CONF_GAIN, default=10): cv.int_range(min=0, max=15),
            cv.Optional(CONF_INT_TIME, default=6): cv.int_range(min=0, max=15),

            cv.Optional(CONF_UVA): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVB): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVC): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),

            cv.Optional(CONF_UVA_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVB_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVC_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
                icon="mdi:weather-sunny",
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x77))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_int_time(config[CONF_INT_TIME]))

    if CONF_UVA in config:
        s = await sensor.new_sensor(config[CONF_UVA])
        cg.add(var.set_uva_sensor(s))

    if CONF_UVB in config:
        s = await sensor.new_sensor(config[CONF_UVB])
        cg.add(var.set_uvb_sensor(s))

    if CONF_UVC in config:
        s = await sensor.new_sensor(config[CONF_UVC])
        cg.add(var.set_uvc_sensor(s))

    if CONF_UVA_WM2 in config:
        s = await sensor.new_sensor(config[CONF_UVA_WM2])
        cg.add(var.set_uva_wm2_sensor(s))

    if CONF_UVB_WM2 in config:
        s = await sensor.new_sensor(config[CONF_UVB_WM2])
        cg.add(var.set_uvb_wm2_sensor(s))

    if CONF_UVC_WM2 in config:
        s = await sensor.new_sensor(config[CONF_UVC_WM2])
        cg.add(var.set_uvc_wm2_sensor(s))
