import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import UNIT_WATT_PER_SQUARE_METER, UNIT_EMPTY

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(AS7331Component),
            cv.Optional("uva_raw"): sensor.sensor_schema(unit_of_measurement="counts"),
            cv.Optional("uvb_raw"): sensor.sensor_schema(unit_of_measurement="counts"),
            cv.Optional("uvc_raw"): sensor.sensor_schema(unit_of_measurement="counts"),
            cv.Optional("uva"): sensor.sensor_schema(unit_of_measurement=UNIT_WATT_PER_SQUARE_METER),
            cv.Optional("uvb"): sensor.sensor_schema(unit_of_measurement=UNIT_WATT_PER_SQUARE_METER),
            cv.Optional("uvc"): sensor.sensor_schema(unit_of_measurement=UNIT_WATT_PER_SQUARE_METER),
            cv.Optional("uv_index"): sensor.sensor_schema(unit_of_measurement=UNIT_EMPTY),
        }
    )
    .extend(i2c.i2c_device_schema(0x77))
    .extend(cv.polling_component_schema("2s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    for key, attr in [
        ("uva_raw", "uva_raw"),
        ("uvb_raw", "uvb_raw"),
        ("uvc_raw", "uvc_raw"),
        ("uva", "uva_irr"),
        ("uvb", "uvb_irr"),
        ("uvc", "uvc_irr"),
        ("uv_index", "uv_index"),
    ]:
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, attr).set(sens))
