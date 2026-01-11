import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import (
    UNIT_WATT_PER_SQUARE_METER,
    ICON_WEATHER_SUNNY,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor"]

as7331_ns = cg.esphome_ns.namespace("as7331")

AS7331Component = as7331_ns.class_(
    "AS7331Component",
    cg.Component,
    i2c.I2CDevice,
)

# ❗❗ DAS IST DER ENTSCHEIDENDE TEIL ❗❗
cg.add_global(as7331_ns.using)
cg.add_library("as7331", None)
cg.add_define("USE_AS7331")

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),
            cv.Optional("uva"): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_PER_SQUARE_METER,
                icon=ICON_WEATHER_SUNNY,
                accuracy_decimals=3,
            ),
            cv.Optional("uvb"): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_PER_SQUARE_METER,
                icon=ICON_WEATHER_SUNNY,
                accuracy_decimals=3,
            ),
            cv.Optional("uvc"): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_PER_SQUARE_METER,
                icon=ICON_WEATHER_SUNNY,
                accuracy_decimals=3,
            ),
            cv.Optional("uv_index"): sensor.sensor_schema(
                icon=ICON_WEATHER_SUNNY,
                accuracy_decimals=2,
            ),
        }
    )
    .extend(i2c.i2c_device_schema(0x74))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if "uva" in config:
        sens = await sensor.new_sensor(config["uva"])
        cg.add(var.set_uva_sensor(sens))

    if "uvb" in config:
        sens = await sensor.new_sensor(config["uvb"])
        cg.add(var.set_uvb_sensor(sens))

    if "uvc" in config:
        sens = await sensor.new_sensor(config["uvc"])
        cg.add(var.set_uvc_sensor(sens))

    if "uv_index" in config:
        sens = await sensor.new_sensor(config["uv_index"])
        cg.add(var.set_uvi_sensor(sens))
