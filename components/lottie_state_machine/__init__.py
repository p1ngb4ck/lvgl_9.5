"""
Lottie State Machine for ESPHome LVGL.

Exposes Lottie animation controls in Home Assistant.
"""

import esphome.automation as automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME, CONF_STATE

CODEOWNERS = ["@youkorr"]
DEPENDENCIES = ["lvgl"]

CONF_LOTTIE_ID = "lottie_id"
CONF_STATES = "states"
CONF_SEGMENT = "segment"
CONF_LOOP = "loop"
CONF_INITIAL_STATE = "initial_state"

lottie_sm_ns = cg.esphome_ns.namespace("lottie_state_machine")
LottieStateMachineComponent = lottie_sm_ns.class_(
    "LottieStateMachineComponent", cg.Component
)
LottieSetStateAction = lottie_sm_ns.class_("LottieSetStateAction", automation.Action)

STATE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_SEGMENT): cv.All(
            cv.ensure_list(cv.int_), cv.Length(min=2, max=2)
        ),
        cv.Optional(CONF_LOOP, default=True): cv.boolean,
    }
)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(LottieStateMachineComponent),
        cv.Required(CONF_LOTTIE_ID): cv.string,
        cv.Optional(CONF_NAME, default="Lottie"): cv.string,
        cv.Optional(CONF_STATES, default={}): cv.Schema(
            {cv.string: STATE_SCHEMA}
        ),
        cv.Optional(CONF_INITIAL_STATE): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)

SET_STATE_SCHEMA = cv.Schema(
    {
        cv.Required(CONF_ID): cv.use_id(LottieStateMachineComponent),
        cv.Required(CONF_STATE): cv.templatable(cv.string),
    }
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_lottie_id(config[CONF_LOTTIE_ID]))
    cg.add(var.set_name(config[CONF_NAME]))

    # Register states
    states = config.get(CONF_STATES, {})
    state_names = []
    for state_name, state_config in states.items():
        seg = state_config[CONF_SEGMENT]
        do_loop = state_config[CONF_LOOP]
        cg.add(var.add_state(state_name, seg[0], seg[1], do_loop))
        state_names.append(state_name)

    if initial := config.get(CONF_INITIAL_STATE):
        cg.add(var.set_initial_state(initial))
    elif state_names:
        cg.add(var.set_initial_state(state_names[0]))

    # Pass state names for the select entity
    for sn in state_names:
        cg.add(var.add_state_name(sn))


@automation.register_action(
    "lottie_state_machine.set_state",
    LottieSetStateAction,
    SET_STATE_SCHEMA,
    synchronous=True,
)
async def lottie_set_state_to_code(config, action_id, template_arg, args):
    parent = await cg.get_variable(config[CONF_ID])
    var = cg.new_Pvariable(action_id, template_arg, parent)
    templ = await cg.templatable(config[CONF_STATE], args, cg.std_string)
    cg.add(var.set_state(templ))
    return var
