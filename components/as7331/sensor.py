import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor

DEPENDENCIES = ["i2c"]

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),
            cv.Required("uva"): sensor.sensor_schema(),
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x77))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    sens = await sensor.new_sensor(config["uva"])
    cg.add(var.set_uva_sensor(sens))
