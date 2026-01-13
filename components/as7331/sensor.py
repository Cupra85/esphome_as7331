import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

CONF_GAIN = "gain"
CONF_INT_TIME = "int_time"

CONF_UVA_RAW = "uva"
CONF_UVB_RAW = "uvb"
CONF_UVC_RAW = "uvc"

CONF_UVA_WM2 = "uva_irradiance"
CONF_UVB_WM2 = "uvb_irradiance"
CONF_UVC_WM2 = "uvc_irradiance"

CONF_UV_INDEX = "uv_index"

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(AS7331Component),

            cv.Optional(CONF_GAIN, default=3): cv.int_range(min=0, max=11),
            cv.Optional(CONF_INT_TIME, default=4): cv.int_range(min=0, max=7),

            cv.Optional(CONF_UVA_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_UVB_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),
            cv.Optional(CONF_UVC_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),

            cv.Optional(CONF_UVA_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
            ),
            cv.Optional(CONF_UVB_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
            ),
            cv.Optional(CONF_UVC_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
            ),

            cv.Optional(CONF_UV_INDEX): sensor.sensor_schema(
                unit_of_measurement="UV Index",
                accuracy_decimals=2,
            ),
        }
    )
    .extend(cv.polling_component_schema("5s"))
    .extend(i2c.i2c_device_schema(0x77))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_gain(config[CONF_GAIN]))
    cg.add(var.set_int_time(config[CONF_INT_TIME]))

    if CONF_UVA_RAW in config:
        cg.add(var.set_uva_raw_sensor(await sensor.new_sensor(config[CONF_UVA_RAW])))
    if CONF_UVB_RAW in config:
        cg.add(var.set_uvb_raw_sensor(await sensor.new_sensor(config[CONF_UVB_RAW])))
    if CONF_UVC_RAW in config:
        cg.add(var.set_uvc_raw_sensor(await sensor.new_sensor(config[CONF_UVC_RAW])))

    if CONF_UVA_WM2 in config:
        cg.add(var.set_uva_wm2_sensor(await sensor.new_sensor(config[CONF_UVA_WM2])))
    if CONF_UVB_WM2 in config:
        cg.add(var.set_uvb_wm2_sensor(await sensor.new_sensor(config[CONF_UVB_WM2])))
    if CONF_UVC_WM2 in config:
        cg.add(var.set_uvc_wm2_sensor(await sensor.new_sensor(config[CONF_UVC_WM2])))

    if CONF_UV_INDEX in config:
        cg.add(var.set_uv_index_sensor(await sensor.new_sensor(config[CONF_UV_INDEX])))
