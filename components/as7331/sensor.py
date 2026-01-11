import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor

CONF_UVA = "uva"
CONF_UVB = "uvb"
CONF_UVC = "uvc"
CONF_UV_INDEX = "uv_index"

DEPENDENCIES = ["i2c"]

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_("AS7331Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),

            cv.Optional(CONF_UVA): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
                icon="mdi:white-balance-sunny",
            ),
            cv.Optional(CONF_UVB): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
                icon="mdi:white-balance-sunny",
            ),
            cv.Optional(CONF_UVC): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
                icon="mdi:white-balance-sunny",
            ),
            cv.Optional(CONF_UV_INDEX): sensor.sensor_schema(
                unit_of_measurement="",
                accuracy_decimals=2,
                icon="mdi:weather-sunny-alert",
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x74))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_UVA in config:
        s = await sensor.new_sensor(config[CONF_UVA])
        cg.add(var.set_uva_sensor(s))

    if CONF_UVB in config:
        s = await sensor.new_sensor(config[CONF_UVB])
        cg.add(var.set_uvb_sensor(s))

    if CONF_UVC in config:
        s = await sensor.new_sensor(config[CONF_UVC])
        cg.add(var.set_uvc_sensor(s))

    if CONF_UV_INDEX in config:
        s = await sensor.new_sensor(config[CONF_UV_INDEX])
        cg.add(var.set_uvi_sensor(s))
