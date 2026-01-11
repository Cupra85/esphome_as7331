import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor

DEPENDENCIES = ["i2c"]

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component",
    cg.PollingComponent,
    i2c.I2CDevice,
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),

            cv.Optional("gain", default=128): cv.int_range(min=1, max=2048),
            cv.Optional("integration_time", default=64): cv.int_range(min=1, max=512),

            cv.Optional("uva"): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
            ),
            cv.Optional("uvb"): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
            ),
            cv.Optional("uvc"): sensor.sensor_schema(
                unit_of_measurement="W/m2",
                accuracy_decimals=3,
            ),
            cv.Optional("uv_index"): sensor.sensor_schema(
                accuracy_decimals=2,
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

    cg.add(var.set_gain(config["gain"]))
    cg.add(var.set_integration_time(config["integration_time"]))

    if "uva" in config:
        cg.add(var.set_uva_sensor(await sensor.new_sensor(config["uva"])))
    if "uvb" in config:
        cg.add(var.set_uvb_sensor(await sensor.new_sensor(config["uvb"])))
    if "uvc" in config:
        cg.add(var.set_uvc_sensor(await sensor.new_sensor(config["uvc"])))
    if "uv_index" in config:
        cg.add(var.set_uvi_sensor(await sensor.new_sensor(config["uv_index"])))
