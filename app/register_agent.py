import os
from dotenv import load_dotenv
from uagents_core.utils.registration import (
    register_in_agentverse,
    AgentverseConnectRequest,
    AgentUpdates,
)
from uagents_core.crypto import Identity

load_dotenv()

identity = Identity.from_seed(os.environ["AGENT_SEED_PHRASE"], 0)

request = AgentverseConnectRequest(
    endpoint=os.environ["AGENT_EXTERNAL_ENDPOINT"],
    user_token=os.environ["AGENTVERSE_KEY"],
    agent_type="custom",
)

agent_details = AgentUpdates(
    name="FlashAI",
    readme="# FlashAI\nAn AI agent for caching and oracle services.",
)

register_in_agentverse(request, identity, agent_details)