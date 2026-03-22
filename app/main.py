import os
from typing import cast
from fastapi import FastAPI
from uagents_core.contrib.protocols.chat import (
    ChatMessage,
    TextContent,
)
from uagents_core.envelope import Envelope
from uagents_core.identity import Identity
from uagents_core.utils.messages import parse_envelope, send_message_to_agent

name = "FlashAI"
identity = Identity.from_seed(os.environ["AGENT_SEED_PHRASE"], 0)
readme = "Make agent faster"
endpoint = "https://cast-stamps-grad-stress.trycloudflare.com/chat"

app = FastAPI()

@app.get("/status")
async def healthcheck():
    return {"status": "OK - Agent is running"}

@app.post("/chat")
async def handle_message(env: Envelope):
    msg = cast(ChatMessage, parse_envelope(env, ChatMessage))
    print(f"Received message from {env.sender}: {msg.text()}")
    send_message_to_agent(
        destination=env.sender,
        msg=ChatMessage([TextContent("Thanks for the message!")]),
        sender=identity,
    )