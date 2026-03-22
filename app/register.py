from uagents_core.utils.registration import (
    register_chat_agent,
    RegistrationRequestCredentials,
)

register_chat_agent(
    "FlashLLM",
    "https://truegradeofficial.com/submit",
    active=True,
    credentials=RegistrationRequestCredentials(
        agentverse_api_key="your_agentverse_api_key_here",
        agent_seed_phrase="ProduHacks2026",        
    ),
)