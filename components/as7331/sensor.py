import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c, sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["i2c"]

CONF_GAIN = "gain"
CONF_INT_TIME = "int_time"
CONF_PROFILE = "profile"

CONF_UVA_RAW = "uva"
CONF_UVB_RAW = "uvb"
CONF_UVC_RAW = "uvc"

CONF_UVA_WM2 = "uva_irradiance"
CONF_UVB_WM2 = "uvb_irradiance"
CONF_UVC_WM2 = "uvc_irradiance"

CONF_UV_INDEX = "uv_index"

as7331_ns = cg.esphome_ns.namespace("as7331")
AS7331Component = as7331_ns.class_(
    "AS7331Component",
    cg.PollingComponent,
    i2c.I2CDevice,
)

# Enum aus as7331.h
PROFILE_INDOOR = as7331_ns.PROFILE_INDOOR
PROFILE_OUTDOOR = as7331_ns.PROFILE_OUTDOOR
PROFILE_UV_LAMP = as7331_ns.PROFILE_UV_LAMP

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(CONF_ID): cv.declare_id(AS7331Component),

            # Profil-Auswahl
            cv.Optional(CONF_PROFILE, default="outdoor"): cv.one_of(
                "indoor", "outdoor", "uv_lamp", lower=True
            ),

            # Optional: manuelles Start-Setup (wird vom Profil überschrieben)
            cv.Optional(CONF_GAIN): cv.int_range(min=0, max=11),
            cv.Optional(CONF_INT_TIME): cv.int_range(min=0, max=7),

            # Raw Counts
            cv.Optional(CONF_UVA_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVB_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVC_RAW): sensor.sensor_schema(
                unit_of_measurement="counts",
                accuracy_decimals=0,
                icon="mdi:weather-sunny",
            ),

            # Irradiance
            cv.Optional(CONF_UVA_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVB_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
                icon="mdi:weather-sunny",
            ),
            cv.Optional(CONF_UVC_WM2): sensor.sensor_schema(
                unit_of_measurement="W/m²",
                accuracy_decimals=6,
                icon="mdi:weather-sunny",
            ),

            # UV Index
            cv.Optional(CONF_UV_INDEX): sensor.sensor_schema(
                unit_of_measurement="UV Index",
                accuracy_decimals=2,
                icon="mdi:weather-sunny-alert",
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

    # Profil setzen
    profile_map = {
        "indoor": PROFILE_INDOOR,
        "outdoor": PROFILE_OUTDOOR,
        "uv_lamp": PROFILE_UV_LAMP,
    }
    cg.add(var.set_profile(profile_map[config[CONF_PROFILE]]))

    # Optional: manuelle Startwerte (nur initial)
    if CONF_GAIN in config:
        cg.add(var.set_gain(config[CONF_GAIN]))
    if CONF_INT_TIME in config:
        cg.add(var.set_int_time(config[CONF_INT_TIME]))

    # Raw Sensoren
    if CONF_UVA_RAW in config:
        sens = await sensor.new_sensor(config[CONF_UVA_RAW])
        cg.add(var.set_uva_raw_sensor(sens))

    if CONF_UVB_RAW in config:
        sens = await sensor.new_sensor(config[CONF_UVB_RAW])
        cg.add(var.set_uvb_raw_sensor(sens))

    if CONF_UVC_RAW in config:
        sens = await sensor.new_sensor(config[CONF_UVC_RAW])
        cg.add(var.set_uvc_raw_sensor(sens))

    # Irradiance Sensoren
    if CONF_UVA_WM2 in config:
        sens = await sensor.new_sensor(config[CONF_UVA_WM2])
        cg.add(var.set_uva_wm2_sensor(sens))

    if CONF_UVB_WM2 in config:
        sens = await sensor.new_sensor(config[CONF_UVB_WM2])
        cg.add(var.set_uvb_wm2_sensor(sens))

    if CONF_UVC_WM2 in config:
        sens = await sensor.new_sensor(config[CONF_UVC_WM2])
        cg.add(var.set_uvc_wm2_sensor(sens))

    # UV Index
    if CONF_UV_INDEX in config:
        sens = await sensor.new_sensor(config[CONF_UV_INDEX])
        cg.add(var.set_uv_index_sensor(sens))
