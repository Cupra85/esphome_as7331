import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch

from .sensor import AS7331Component

DEPENDENCIES = ["as7331"]

as7331_ns = cg.esphome_ns.namespace("as7331")

AS7331Switch = as7331_ns.class_(
    "AS7331Switch",
    switch.Switch,
)

CONFIG_SCHEMA = switch.switch_schema(AS7331Switch).extend(
    {
        cv.GenerateID("as7331_id"): cv.use_id(AS7331Component),
    }
)

async def to_code(config):
    parent = await cg.get_variable(config["as7331_id"])
    var = cg.new_Pvariable(config[cv.GenerateID()])

    cg.add(var.set_parent(parent))
    await switch.register_switch(var, config)
