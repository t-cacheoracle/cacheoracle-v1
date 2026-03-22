import os
from uagents_core.crypto import Identity
from fetchai.registration import register_with_agentverse

# Store your Agentverse API Key in the environment variables. 

# Your Agent's unique key for generating an address on Agentverse
ai_identity = Identity.from_seed(("Produhacks2026"), 0)

# Give your Agent a name. This allows you to easily identify one
# of your Agents from other agents on Agentverse.
name = "FlashAI"

# The webhook that your AI receives messages on.
ai_webhook = "https://truegradeofficial.com/submit"

register_with_agentverse(
    ai_identity,
    ai_webhook,
    AGENTVERSE_KEY,
    name,
    readme="HI",
)