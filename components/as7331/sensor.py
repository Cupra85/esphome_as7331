import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor

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

            cv.Optional("uva_raw"): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),
            cv.Optional("uvb_raw"): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),
            cv.Optional("uvc_raw"): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
            ),

            cv.Optional("uva"): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
            ),
            cv.Optional("uvb"): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
            ),
            cv.Optional("uvc"): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=4,
            ),

            cv.Optional("uv_index"): sensor.sensor_schema(
                unit_of_measurement="UV Index",
                accuracy_decimals=2,
            ),
        }
    )
    .extend(i2c.i2c_device_schema(0x77))
    .extend(cv.polling_component_schema("2s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if "uva_raw" in config:
        sens = await sensor.new_sensor(config["uva_raw"])
        cg.add(var.uva_raw.set(sens))

    if "uvb_raw" in config:
        sens = await sensor.new_sensor(config["uvb_raw"])
        cg.add(var.uvb_raw.set(sens))

    if "uvc_raw" in config:
        sens = await sensor.new_sensor(config["uvc_raw"])
        cg.add(var.uvc_raw.set(sens))

    if "uva" in config:
        sens = await sensor.new_sensor(config["uva"])
        cg.add(var.uva_irr.set(sens))

    if "uvb" in config:
        sens = await sensor.new_sensor(config["uvb"])
        cg.add(var.uvb_irr.set(sens))

    if "uvc" in config:
        sens = await sensor.new_sensor(config["uvc"])
        cg.add(var.uvc_irr.set(sens))

    if "uv_index" in config:
        sens = await sensor.new_sensor(config["uv_index"])
        cg.add(var.uv_index.set(sens))
