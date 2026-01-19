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
                accuracy_decimals=3,
            ),
            cv.Optional("uvb"): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=3,
            ),
            cv.Optional("uvc"): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=3,
            ),

            cv.Optional("uv_index"): sensor.sensor_schema(
                unit_of_measurement="UV Index",
                accuracy_decimals=2,
            ),

            cv.Optional("uva_calibration", default=1.0): cv.float_,
            cv.Optional("uvb_calibration", default=1.0): cv.float_,
            cv.Optional("uvc_calibration", default=1.0): cv.float_,
        }
    )
    .extend(i2c.i2c_device_schema(0x77))
    .extend(cv.polling_component_schema("2s"))
)

async def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_uva_calibration(config["uva_calibration"]))
    cg.add(var.set_uvb_calibration(config["uvb_calibration"]))
    cg.add(var.set_uvc_calibration(config["uvc_calibration"]))

    if "uva_raw" in config:
        cg.add(var.set_uva_raw(await sensor.new_sensor(config["uva_raw"])))
    if "uvb_raw" in config:
        cg.add(var.set_uvb_raw(await sensor.new_sensor(config["uvb_raw"])))
    if "uvc_raw" in config:
        cg.add(var.set_uvc_raw(await sensor.new_sensor(config["uvc_raw"])))

    if "uva" in config:
        cg.add(var.set_uva(await sensor.new_sensor(config["uva"])))
    if "uvb" in config:
        cg.add(var.set_uvb(await sensor.new_sensor(config["uvb"])))
    if "uvc" in config:
        cg.add(var.set_uvc(await sensor.new_sensor(config["uvc"])))

    if "uv_index" in config:
        cg.add(var.set_uv_index(await sensor.new_sensor(config["uv_index"])))
