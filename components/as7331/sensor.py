import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import STATE_CLASS_MEASUREMENT, CONF_ID

CONF_UVA_RAW = "uva_raw"
CONF_UVB_RAW = "uvb_raw"
CONF_UVC_RAW = "uvc_raw"

CONF_UVA = "uva"
CONF_UVB = "uvb"
CONF_UVC = "uvc"

CONF_UV_INDEX = "uv_index"

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component", cg.PollingComponent, i2c.I2CDevice
)

def _schema(unit=None):
    return sensor.sensor_schema(
        unit_of_measurement=unit,
        state_class=STATE_CLASS_MEASUREMENT,
    )

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(AS7331Component),

        # RAW COUNTS
        cv.Optional(CONF_UVA_RAW): _schema(),
        cv.Optional(CONF_UVB_RAW): _schema(),
        cv.Optional(CONF_UVC_RAW): _schema(),

        # CALCULATED POWER
        cv.Optional(CONF_UVA): _schema("W/m²"),
        cv.Optional(CONF_UVB): _schema("W/m²"),
        cv.Optional(CONF_UVC): _schema("W/m²"),

        # UV INDEX
        cv.Optional(CONF_UV_INDEX): _schema(),
    }
).extend(
    i2c.i2c_device_schema(0x77)
).extend(
    cv.polling_component_schema("5s")
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    for key in (
        CONF_UVA_RAW, CONF_UVB_RAW, CONF_UVC_RAW,
        CONF_UVA, CONF_UVB, CONF_UVC,
        CONF_UV_INDEX,
    ):
        if key in config:
            sens = await sensor.new_sensor(config[key])
            cg.add(getattr(var, f"set_{key}_sensor")(sens))
