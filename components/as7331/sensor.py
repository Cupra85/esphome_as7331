import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor

DEPENDENCIES = ["i2c"]

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_("AS7331Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = cv.Schema({...}).extend(cv.polling_component_schema("5s")).extend(i2c.i2c_device_schema(0x74))

async def to_code(config):
    ...
